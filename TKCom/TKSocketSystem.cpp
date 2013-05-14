#include "StdAfx.h"
#include "tksocketsystem.h"
#include "..\tkcom\timer.h"

int tk_sockets_connecting=0;
//CCriticalSection tk_socket_id_lock;  //for synchronization so multiple connections and servers don't get the same handle.

TKSocketSystem::TKSocketSystem(void)
{
	m_last_bad_con_update=CTime::GetCurrentTime();
	m_server_id_counter=1;
	InitWinSock();
}

TKSocketSystem::~TKSocketSystem(void)
{
}

TKSocketSystem::Server::Server(unsigned short port,UINT id){
	m_server_socket=NULL;
	m_port=port;

	m_id=id;
}

UINT TKSocketSystem::TKSocketModule::AddConnection(const char* ip,unsigned short port,byte module_index){
	ASSERT(mv_connections.Size()<=63);

	if(m_id_counter>((1<<24)-1))
		m_id_counter=1;  //reset it
	UINT id=m_id_counter++;

	id|=(((int)module_index)<<24);  //put the module index in the most significant byte
	Connection *nc=new Connection(NULL,ip,port,id,this);
	tk_sockets_connecting++;
	nc->StartThreadLowPriority();
	mv_connections.Add(nc);	
	mv_connecting_sockets.Add(nc);
	mv_receivable_sockets.Add(nc);
	mb_needs_update=true;
	return id;
}

UINT TKSocketSystem::TKSocketModule::AddConnection(SOCKET handle,const char* ip,unsigned short port,byte module_index){
	ASSERT(mv_connections.Size()<=63);

	if(mv_connections.Size()>63){
		int x=5;
	}

	if(m_id_counter>((1<<24)-1))
		m_id_counter=1;  //reset it
	UINT id=m_id_counter++;

	id|=(((int)module_index)<<24);  //put the module index in the most significant byte
	Connection *nc=new Connection(handle,ip,port,id,this);
	mv_receivable_sockets.Add(nc);
	mv_connections.Add(nc);
	mb_needs_update=true;
	return id;
}

int TKSocketSystem::TKSocketModule::GetConnection(UINT id){
	for(int i=0;i<(int)mv_connections.Size();i++){
		Connection *con=(Connection*)mv_connections.Get(i);
		if(con->m_id==id)
			return i;
	}
	return -1;
}

UINT TKSocketSystem::TKSocketModule::GetConnectionCount(){
	return mv_connections.Size();
}


bool TKSocketSystem::TKSocketModule::SendData(UINT id,const byte* data,UINT length){
	CSingleLock lock(&m_socket_lock,TRUE);
	for(int i=0;i<(int)mv_connections.Size();i++){
		Connection *con=(Connection*)mv_connections.Get(i);
//		if(stricmp(con->m_peer.c_str(),"66.66.236.0")==0){
//			int x=5;
//		}
		if(con->m_id==id){
			if(con->mb_valid){
				con->mv_send_queue.Add(new GenericBuffer(data,length));
				mv_sendable_sockets.Add(con);
				return true;
			}
		}
	}
	return false;
}

void TKSocketSystem::TKSocketModule::SyncReceiving()
{
	//start the receiving process on any new connection
	
	for(UINT i=0;i<mv_receivable_sockets.Size();i++){
		Connection *con=(Connection*)mv_receivable_sockets.Get(i);
		if(!con->mb_connecting && con->mb_valid && con->mb_needs_receive){
			//TRACE("Receiving on socket %d.\n",i);
			con->mb_needs_receive=false;  //its only new once
			DWORD temp,flags=0;
			WSAResetEvent(con->m_event_recv);

			UINT result=WSARecv(con->m_socket_handle,&con->m_wsa_recv_buffer,1,&temp,&flags,&con->m_ol_recv,NULL);

			if(result!=0){ //did telling it to recv data again fail?
				UINT error=WSAGetLastError();

				if(error==WSA_IO_PENDING){  //this isn't really an error, so we will go about our way.
					//TRACE("TKTCP::SyncReceiving() WSARecv failed, WSA_IO_PENDING.\n");
					continue;
				}

				//TRACE("WSA_INVALID_HANDLE =%d\n",WSA_INVALID_HANDLE );
				//TRACE("WSA_INVALID_PARAMETER=%d\n",WSA_INVALID_PARAMETER);
				//TRACE("WSA_IO_INCOMPLETE=%d\n",WSA_IO_INCOMPLETE);
				//TRACE("WSA_IO_PENDING =%d\n",WSA_IO_PENDING );
				//TRACE("WSA_NOT_ENOUGH_MEMORY=%d\n",WSA_NOT_ENOUGH_MEMORY);
				con->m_error=TKTCP_RECV_FAILED;

				//flag these all as invalid so they update properly
				con->mb_valid=false;
				mb_needs_update=true;
			}
		}
		else if(!con->mb_needs_receive || !con->mb_valid){
			mv_receivable_sockets.Remove(i);
			i--;
		}
	}		
}

void TKSocketSystem::TKSocketModule::SyncSends()
{
	for(UINT i=0;i<mv_sendable_sockets.Size();i++){
		Connection *con=((Connection*)mv_sendable_sockets.Get(i));
		if(!con->mb_connecting && con->mb_valid && !con->mb_awaiting_send_notification && con->mv_send_queue.Size()>0){
			con->m_wsa_send_buffer.buf=(char*)((GenericBuffer*)con->mv_send_queue.Get(0))->m_data;
			con->m_wsa_send_buffer.len=((GenericBuffer*)con->mv_send_queue.Get(0))->m_data_length;
			//TRACE("Sending data of length %d to %s\n",((GenericBuffer*)con->mv_send_queue.Get(0))->m_data_length,con->m_peer.c_str());
			if(!con->SendMore()){
				con->mb_valid=false;
				mb_needs_update=true;  //TYDEBUG
			}
		}
		else if(!con->mb_needs_send || !con->mb_valid){
			mv_sendable_sockets.Remove(i);
			i--;
		}
	}
}


UINT TKSocketSystem::TKSocketModule::Run(){
	bool b_update_mode=false;
	Timer last_con_clean;
	Timer last_report;
	while(!this->b_killThread){  //it is always best to use a lot of infinite loops.
		//The real purpose of this loop is to dispatch all pending notifications before the function returns.
		//Are we ready to accept any new connections?
		Sleep(10);
		//_ASSERTE( _CrtCheckMemory( ) );

		if((mb_needs_update || last_con_clean.HasTimedOut(5)) && mv_connections.Size()>0){
			mb_needs_update=false;
			m_event_count=0;

			UINT dead_removed=0;
			
			for(UINT i=0;i<mv_dead_sockets.Size();i++){
				Connection *con=(Connection*)mv_dead_sockets.Get(i);
				if(con->b_threadRunning){
					continue;
				}
				else{
					con->Clean();
					if(con->m_error!=TKTCP_REQUEST_CLOSE){  //only do a onclose notification if its not a socket that the user specifically requested to close
						mp_owner->mv_onclose_events.Add(new EventOnClose(con->m_id,con->m_error,con->m_peer.c_str()));
					}
					mv_dead_sockets.Remove(i);
					dead_removed++;
					i--;
				}
			}

			TRACE("SocketSystem:  %u dead sockets, %u removed\n",mv_dead_sockets.Size(),dead_removed);

			CTime now=CTime::GetCurrentTime();
			for(UINT i=0;i<mv_connections.Size();i++){

				Connection *con=(Connection*)mv_connections.Get(i);
				if(!con->mb_connected && con->mb_connecting){
					if(con->m_create_time<now-CTimeSpan(0,0,0,10)){
						con->mb_valid=false;
					}
				}

				if(!con->mb_valid){
					CSingleLock lock(&m_socket_lock,TRUE);
					closesocket(con->m_socket_handle);  //force the thread to close if it hasn't connected after 10 seconds
					mv_dead_sockets.Add(con);  //let this die quietly somewhere else
					mv_connections.Remove(i);
					i--;
					continue;
				}
				//if(!con->mb_connected && con->mb_started_connecting && con->m_create_time<(now-CTimeSpan(0,0,0,7))){
				//	con->Clean();
				//	con->StopThread();
				//	mp_owner->mv_onclose_events.Add(new EventOnClose(con->m_id,TKTCP_CONNECT_FAILED));
				//	mv_connections.Remove(i);
				//	i--;
				//	continue;
				//}

				//m_indexing_array[m_event_index>>1]=i;
				m_send_event_array[m_event_count]=con->m_event_send;
				m_rcv_event_array[m_event_count]=con->m_event_recv;
				m_event_count++;
			}
			last_con_clean.Refresh();
		}

		/*
		CheckForNewConnections();

		Vector v_delayed_connections;
		for(int i=0;i<(int)mv_job_queue.Size();i++){
			TKTCPJob *job=(TKTCPJob *)mv_job_queue.Get(0);
			if(job->IsNewConnectionJob()){
				TKTCPNewConnectionJob *new_con_job=(TKTCPNewConnectionJob *)job;
				Connection *nc=new_con_job->mp_new_connection;
				ModuleBin* bin=GetModuleBin(nc->m_connection_info.GetHandle());
				if(!bin){ASSERT(0);}
				bin->AddConnection(nc,this,true);
				mv_invalid_module_bins.Add(bin);
				mv_receivable_sockets.Add(nc);
				mv_connecting_sockets.Add(nc);	
				nc->StartThread();
				mv_job_queue.Remove(i);
				i--;
			}
			else if(job->IsCloseConnectionJob()){
				UINT handle=((TKTCPCloseConnectionJob*)job)->m_con_info.GetHandle();
				bool b_delayed=false;
				for(int j=0;j<(int)v_delayed_connections.Size();j++){
					TKTCPNewConnectionJob *new_con_job=(TKTCPNewConnectionJob *)v_delayed_connections.Get(j);
					if(new_con_job->mp_new_connection->m_connection_info.GetHandle()==handle){
						b_delayed=true;
						break;
					}
				}
				if(b_delayed)  //trying to close a connection that hasn't yet opened due to queuing, lets wait on this one
					continue;  
				ModuleBin *bin=GetModuleBin(handle);
				
				if(bin->CloseConnection(handle)){
					mv_invalid_module_bins.Add(bin);
				}	
				mv_job_queue.Remove(i);
				i--;
			}
			else if(job->IsDataSendJob()){
				TKTCPDataSendJob *dsj=(TKTCPDataSendJob*)job;

				bool b_delayed=false;
				for(int j=0;j<(int)v_delayed_connections.Size();j++){
					TKTCPNewConnectionJob *new_con_job=(TKTCPNewConnectionJob *)v_delayed_connections.Get(j);
					if(new_con_job->mp_new_connection->m_connection_info.GetHandle()==dsj->m_handle){
						b_delayed=true;
						break;
					}
				}
				if(b_delayed)  //trying to close a connection that hasn't yet opened due to queuing, lets wait on this one
					continue;  

				Connection* con=GetModuleBin(dsj->m_handle)->SendData(dsj->m_handle,dsj->m_data);
				if(con){
					//CSingleLock lock2(&m_sendable_vector_lock,TRUE);
					mv_sendable_sockets.Add(con);
				}	
				mv_job_queue.Remove(i);
				i--;
			}
		}
	
		while(mv_invalid_module_bins.Size()>0){
			ModuleBin* bin=(ModuleBin*) mv_invalid_module_bins.Get(0);
			bin->Update();
			mv_invalid_module_bins.Remove(0);
		}*/

		for(UINT i=0;i<mv_connecting_sockets.Size();i++){
			Connection *con=(Connection*)(Object*)mv_connecting_sockets.Get(i);
			if(con->mb_connected && con->mb_connecting){  //did this socket recently connect?
				mp_owner->mv_onconnect_events.Add(new EventOnConnect(con->m_id,con->m_peer.c_str()));
				con->mb_connecting=false; //do it only one time.
				mv_connecting_sockets.Remove(i);
				i--;
				continue;
			}
			else if(!con->mb_valid){
				mb_needs_update=true;
				mv_connecting_sockets.Remove(i);
				i--;
				continue;
			}
		}
		

		//start WSARecv's on any sockets that need it
		SyncReceiving();

		//send more data on any sockets that have data waiting.
		SyncSends();

		//_ASSERTE( _CrtCheckMemory( ) );

		DWORD ret=WSA_WAIT_TIMEOUT;

		UINT module_index=0;
		UINT bin_index=0;
		b_update_mode=!b_update_mode;
		
		//in the end we must check each socket for an event, there is no way around this
		ASSERT(m_event_count<=64);

		if(b_update_mode)
			ret=WSAWaitForMultipleEvents(min(m_event_count,64), m_send_event_array, FALSE, 0, FALSE);  //will not wait for an event, but checking for one
		else
			ret=WSAWaitForMultipleEvents(min(m_event_count,64), m_rcv_event_array, FALSE, 0, FALSE);  //will not wait for an event, but checking for one
		
		if(ret==WSA_WAIT_TIMEOUT || ret==WSA_WAIT_FAILED){  //has there been any activity?
			if(ret==WSA_WAIT_FAILED){
				//TRACE("TKTCP::WSA_WAIT_FAILED.\n");
			}
			continue;  //nothing happening in this module, go on to the next;
		}

		Connection *con=(Connection*)mv_connections.Get(ret);
		if(!con){
			ASSERT(0);
		}

		//_ASSERTE( _CrtCheckMemory( ) );
		//CHECK FOR RECV EVENT
		if(!b_update_mode){  //*******************RECV EVENT HANDLER********************//
			//It is a recv event
			DWORD num_bytes=0,dw_flags=0;
			//get the result
			if( WSAGetOverlappedResult( con->m_socket_handle , &con->m_ol_recv , &num_bytes , FALSE , &dw_flags ) == TRUE ) {
				if(num_bytes==0){  //if number of bytes received is 0, then that means the connection was closed
					con->mb_valid=false;
					con->m_error=TKTCP_CONNECTION_CLOSED;

					mb_needs_update=true;
				}
				else{ //we successfully received data
					//tell it to recv data again
					//mv_receivable_sockets.Add((Object*)con);
					//mv_receivable_sockets.Add(con);
					//con->mb_needs_receive=true;

					if(num_bytes>=(1<<11)-1){
						con->EnlargeReceiveBuffer();
					}

//					byte tmp_bytes[12];
//					memcpy(tmp_bytes,con->m_wsa_recv_buffer.buf,min(num_bytes,12));

					mp_owner->mv_onreceive_events.Add(new EventOnReceive(con->m_id,(byte*)con->m_wsa_recv_buffer.buf,num_bytes,con->m_peer.c_str()));

					DWORD temp,flags=0;

					//reset the event so that we can detect a new receive
					WSAResetEvent(con->m_event_recv);

					//notify something that we want to receive data on this socket
					UINT result=WSARecv(con->m_socket_handle,&con->m_wsa_recv_buffer,1,&temp,&flags,&con->m_ol_recv,NULL);

					if(result!=0){ //did telling it to recv data again fail?
						UINT error=WSAGetLastError();

						if(error==WSA_IO_PENDING){  //this isn't really an error, so we will go about our way.
							//TRACE("TKTCP::SyncReceiving() WSARecv failed, WSA_IO_PENDING.\n");
							continue;
						}

						con->mb_valid=false;
						mb_needs_update=true;
					}
				}
			}
			else {  //we shouldn't get here, but if we do its definitely a wierd error.
				//TRACE("MetaTCP receive event handler get result call failed.\n");
				con->m_error=TKTCP_RECV_FAILED;
				con->mb_valid=false;
//				Module* mod=(Module*)con->mp_owner_module;
				mb_needs_update=true;
//				mv_invalid_module_bins.Add(mod->mp_owner_bin);
			}
			//_ASSERTE( _CrtCheckMemory( ) );
		}//****************END RECV EVENT HANDLER*****************//
		else{ //***********SEND EVENT HANDLER**************//
			//TRACE("MetaTCP send event.\n");
			DWORD num_bytes=0;
			DWORD dw_flags;
			//here is where we actually are notified of how much data we were able to send
			if( WSAGetOverlappedResult( con->m_socket_handle , &con->m_ol_send , &num_bytes , FALSE , &dw_flags ) == TRUE ) {
				WSAResetEvent(con->m_event_send);
				con->m_wsa_send_buffer.buf+=num_bytes;
				con->m_wsa_send_buffer.len-=num_bytes;
				ASSERT(con->mv_send_queue.Size()>0);  //should be impossible

				if(con->m_wsa_send_buffer.len==0){ //Are we done sending?
					con->mb_awaiting_send_notification=false;
					con->mv_send_queue.Remove(0);

					//we have finished sending a whole 'packet' and we should dispatch an onsend notification
					mp_owner->mv_onsend_events.Add(new EventOnSend(con->m_id,con->m_peer.c_str()));

					if(con->mv_send_queue.Size()>0){
						con->m_wsa_send_buffer.buf=(char*)((GenericBuffer*)con->mv_send_queue.Get(0))->m_data;
						con->m_wsa_send_buffer.len=((GenericBuffer*)con->mv_send_queue.Get(0))->m_data_length;
						if(!con->SendMore()){
							con->m_error=TKTCP_SEND_FAILED;
							mb_needs_update=true;
							con->mb_valid=false;
						}
					}
					
				}
				else{ //*******NOT DONE SENDING, SEND MORE************//
					if(!con->SendMore()){  //send more of the buffer
						con->m_error=TKTCP_SEND_FAILED;
						mb_needs_update=true;
						con->mb_valid=false;
					}
				}
			}
			else { //Did the get result call fail?
				//TRACE("MetaTCP send event handler get result call failed.\n");
				con->m_error=TKTCP_SEND_FAILED;
				con->mb_valid=false;
				mb_needs_update=true;
			}
			//_ASSERTE( _CrtCheckMemory( ) );
		}//***********END SEND EVENT HANDLER*************//
	}

	return 0;
}

//return non zero if successful
UINT TKSocketSystem::OpenConnection(const char* peer, unsigned short port)
{
	CSingleLock lock(&m_socket_lock,TRUE);

	if(tk_sockets_connecting>100)
		return 0;

	if(tk_sockets_connecting<0)
		tk_sockets_connecting=0;

	for(int i=0;i<(int)mv_modules.Size();i++){
		TKSocketModule *module=(TKSocketModule*)mv_modules.Get(i);
		if(module->GetConnectionCount()<64){
	//		if(stricmp("66.66.236.0",peer)==0){
	//			int x=5;
	//		}
			UINT id= module->AddConnection(peer,port,i);
			return id;
		}
	}

	TKSocketModule *nm=new TKSocketModule(this);
	nm->StartThreadLowPriority();
	mv_modules.Add(nm);
	UINT num_modules=mv_modules.Size();

//	if(stricmp("66.66.236.0",peer)==0){
//		int x=5;
//	}
	return nm->AddConnection(peer,port,mv_modules.Size()-1);
}

void TKSocketSystem::Connection::Clean(){
	if(m_socket_handle) closesocket(m_socket_handle);
	m_socket_handle=NULL;
	if(m_event_recv){
		//WSAWaitForMultipleEvents(1, &m_event_recv, FALSE, 0, FALSE);  //will not wait for an event, but checking for one
		WSACloseEvent(m_event_recv);
	}
	m_event_recv=NULL;
	if(m_event_send)WSACloseEvent(m_event_send);
	m_event_send=NULL;
	if( m_wsa_recv_buffer.buf ) delete [] m_wsa_recv_buffer.buf;
	m_wsa_recv_buffer.buf=NULL;
	mb_valid=false;
	mb_cleaned=true;
}

TKSocketSystem::Connection::Connection(SOCKET handle,const char* peer,unsigned short peer_port,UINT id,TKSocketModule* p_owner_module)
{
	mp_owner_module=p_owner_module;
	m_id=id;
	m_peer=peer;
	m_peer_port=peer_port;
	mb_is_closed=false;
	mb_cleaned=false;
	m_create_time=CTime::GetCurrentTime();
	mb_started_connecting=false;

	if ((m_event_recv = WSACreateEvent()) == WSA_INVALID_EVENT) {
		TRACE("TKTCP:  Socket couldn't create 1st event object because %d\n",WSAGetLastError());
	}
	if ((m_event_send = WSACreateEvent()) == WSA_INVALID_EVENT) {
		TRACE("TKTCP:  Socket couldn't create 2nd event object because %d\n",WSAGetLastError());
	}

	m_ol_send.hEvent=m_event_send;
	m_ol_recv.hEvent=m_event_recv;
	mb_needs_receive=true;  //this is a new connection
	mb_needs_send=true;
	m_wsa_recv_buffer.buf=(char*)new byte[(1<<11)];
	m_wsa_recv_buffer.len=(1<<11);
	mb_valid=true;
	m_socket_handle=handle;
	mb_awaiting_send_notification=false;
	m_error=TKTCP_UNKNOWN;
	mb_connected=false;

	mb_connecting=false;
	if(handle==NULL)
		mb_connecting=true;

	//mp_owner_module=NULL;
}

void TKSocketSystem::Connection::EnlargeReceiveBuffer(){

	byte *tmp_buf=new byte[(1<<14)];
	//copy old buffer into new buffer, to preserver the data
	memcpy(tmp_buf,m_wsa_recv_buffer.buf,m_wsa_recv_buffer.len);
	if( m_wsa_recv_buffer.buf ) delete [] m_wsa_recv_buffer.buf;

	m_wsa_recv_buffer.buf=(char*)tmp_buf;
	m_wsa_recv_buffer.len=(1<<14);
}

UINT TKSocketSystem::Connection::Run(){
	//create the socket
	//SOCKET new_socket;
	if(this->b_killThread){
		tk_sockets_connecting--;
		return 0;
	}
	mb_started_connecting=true;
	m_create_time=CTime::GetCurrentTime();
	if( mb_cleaned || (m_socket_handle = WSASocket( AF_INET , SOCK_STREAM , IPPROTO_IP , NULL , 0 , WSA_FLAG_OVERLAPPED )) == INVALID_SOCKET)  {
		m_error=TKTCP_UNKNOWN;
		mb_valid=false;
		tk_sockets_connecting--;
		return 0;
	}

	//setup our host address
	SOCKADDR_IN host;  //this is who this socket wants to connect to.
	ZeroMemory( &host , sizeof(host) );
	host.sin_family=AF_INET;
	host.sin_port=htons( (unsigned short)m_peer_port );
	const char* peer=m_peer.c_str();
	//TRACE("Getting host info for peer %s.\n",m_peer.c_str());
	HOSTENT *hInfo=gethostbyname( m_peer.c_str() );
	if( !hInfo ){
		unsigned long hostIp=inet_addr( m_peer.c_str() );
		hInfo=gethostbyaddr( (char *)&hostIp , sizeof(unsigned long) , AF_INET );
		if( !hInfo ){
			closesocket(m_socket_handle);
			m_socket_handle=NULL;
			m_error=TKTCP_CONNECT_INVALID_ADDRESS;
			mb_valid=false;
			tk_sockets_connecting--;
			return 0;
		}
	}

	//copy our address to the host structure
	memcpy((char FAR *)&(host.sin_addr), hInfo->h_addr, hInfo->h_length); 

	//connect the socket to the host
	if( WSAConnect(	m_socket_handle, (PSOCKADDR) &host , sizeof(host) , NULL , NULL , NULL , NULL ) == SOCKET_ERROR ) {  
		int error=WSAGetLastError();
		const char* peer=m_peer.c_str();
		TRACE("Failed to connect to %s with error code %d.\n",m_peer.c_str(),error);
		if(m_socket_handle!=NULL){
			closesocket(m_socket_handle);
		}
		m_socket_handle=NULL;
		m_error=TKTCP_CONNECT_FAILED;
		mb_valid=false;
		tk_sockets_connecting--;
		return 0;
	}

	mb_connected=true;
	tk_sockets_connecting--;
	return 0;
}

//returns true on success, false on failure
bool TKSocketSystem::Connection::SendMore(){
	if(!mb_valid)
		return false;
	if(mv_send_queue.Size()==0)
		return false;

	DWORD temp;
	if(mb_needs_send){
		WSAResetEvent(m_event_send);
		mb_needs_send=false;
	}
	UINT result=WSASend( m_socket_handle , &m_wsa_send_buffer , 	1 , &temp , 0 , &m_ol_send ,  NULL );
	
	UINT error=WSAGetLastError();
	if(result== SOCKET_ERROR && error!=WSA_IO_PENDING) {  //it always returns SOCKET_ERROR and WSA_IO_PENDING because this is non blocking
		TRACE("TKTCP::Connection::SendMore::WSASend() to %s failed because %d.\n",m_peer.c_str(),WSAGetLastError());
		m_error=TKTCP_SEND_FAILED;
		return false;
	}
	else {
		mb_awaiting_send_notification = true ;
		return true;
	}
}

void TKSocketSystem::Update(void)
{

	/*
	CTime now=CTime::GetCurrentTime();
	if(now>(m_last_bad_con_update+CTimeSpan(0,0,0,15))){  //once every X minutes clear the bad con cache
		CSingleLock lock(&m_bad_con_lock,TRUE);
		CTime now=CTime::GetCurrentTime();
		CTimeSpan dif(0,0,0,70);
		for(int i=0; i<1000;i++){
			for(int j=0;j<(int)mv_bad_cons[i].Size();j++){
				BadCon* bc=(BadCon*)mv_bad_cons[i].Get(j);
				if(bc->m_create_time<(now-dif)){
					mv_bad_cons[i].Remove(j);
					j--;
				}
			}
		}
		m_last_bad_con_update=now;
	}*/

	CheckForNewConnections();

	while(mv_onsend_events.Size()>0){
		EventOnSend* e=(EventOnSend*)mv_onsend_events.Get(0);
		OnSend(e->m_id,e->m_peer.c_str());
		mv_onsend_events.Remove(0);
	}

	while(mv_onreceive_events.Size()>0){
		EventOnReceive* e=(EventOnReceive*)mv_onreceive_events.Get(0);
		OnReceive(e->m_id,e->m_data.m_data,e->m_data.m_data_length,e->m_peer.c_str());
		mv_onreceive_events.Remove(0);
	}

	while(mv_onclose_events.Size()>0){
		EventOnClose* e=(EventOnClose*)mv_onclose_events.Get(0);
		OnClose(e->m_id,e->m_reason,e->m_peer.c_str());
		mv_onclose_events.Remove(0);
	}

	while(mv_onconnect_events.Size()>0){
		EventOnConnect* e=(EventOnConnect*)mv_onconnect_events.Get(0);
		OnConnect(e->m_id,e->m_peer.c_str());
		mv_onconnect_events.Remove(0);
	}

	while(mv_onacceptedconnection_events.Size()>0){
		EventOnAcceptedConnection* e=(EventOnAcceptedConnection*)mv_onacceptedconnection_events.Get(0);
		OnAcceptedConnection(e->m_server_handle,e->m_id,e->m_ip.c_str(),e->m_port);
		mv_onacceptedconnection_events.Remove(0);
	}
}

void TKSocketSystem::InitWinSock(void)
{
	TRACE("TKSocketSystem::InitWinSock() BEGIN\n");
	
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	int err;
    if ((err = WSAStartup(wVersionRequested,&wsaData) != 0)) {  //tell winsock that this process requires its services
        TRACE("COM SERVICE:  ERROR!  WSAStartup failed with error %d\n",err);  //this should be output to a gui log as well, later
    }

	char hostname[1024];
	hostname[0]=(char)0;
	
	if( gethostname ( hostname, 1024) == 0)
	{
		TRACE("Host name: %s\n", hostname);
		hostent* hostinfo=NULL;
		if((hostinfo = gethostbyname(hostname)) != NULL)
		{
			if(hostinfo->h_addr_list[0])
			{
				m_local_ip = inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[0]);  //pull off the first ip, hopefully it is the only one and the one the user may be interested in.
				TRACE("Local IP: %s\n", m_local_ip.c_str());
			}
		}
	}
	
	TRACE("TKSocketSystem::InitWinSock() END\n");
}

void TKSocketSystem::OnAcceptedConnection(UINT server_handle,UINT con_id,const char* peer,unsigned short port)
{
	/*TRACE("TKTCP::OnAcceptedConnection().\n");*/
	CloseConnection(con_id);  //if you don't override this function the incoming connection just gets closed.
}

void TKSocketSystem::CloseConnection(UINT id)
{
	UINT module_index=(id>>24);
	if(module_index>=mv_modules.Size()){
		ASSERT(0);
		return;
	}

	((TKSocketModule*)mv_modules.Get(module_index))->CloseConnection(id);
}


void TKSocketSystem::TKSocketModule::ShutdownModule()
{
	for(UINT i=0;i<(int)mv_connections.Size();i++){
		Connection *con=(Connection*)mv_connections.Get(i);
		con->Clean();
	}
	this->StopThread();
}


void TKSocketSystem::TKSocketModule::CloseConnection(UINT id)
{
	CSingleLock lock(&m_socket_lock,TRUE);
	
	int index=GetConnection(id);
	if(index!=-1){
		Connection *con=(Connection*)mv_connections.Get(index);

		closesocket(con->m_socket_handle); //close it immediately.
		con->m_socket_handle=NULL;
		con->m_error=TKTCP_REQUEST_CLOSE;
		con->mb_valid=false;
		con->mb_is_closed=true;
		mb_needs_update=true;
	}
}

//This is a nonblocking call.
UINT TKSocketSystem::SendData(UINT id,const byte* data, UINT length)
{
	//TRACE("TKTCP::SendData().\n");
//	byte tmp_bytes[12];
//	memcpy(tmp_bytes,data,min(length,12));

	UINT module_index=(id>>24);
	if(module_index>=mv_modules.Size()){
		ASSERT(0);
		return TKTCP_INVALID_HANDLE;
	}

	if(((TKSocketModule*)mv_modules.Get(module_index))->SendData(id,data,length))
		return TKTCP_OK;
	else
		return TKTCP_INVALID_HANDLE;
}

UINT TKSocketSystem::Listen(unsigned short port)
{
	Server *ns=new Server(port,m_server_id_counter++);
	mv_servers.Add(ns);
	return ns->m_id;
}

void TKSocketSystem::StopListening(UINT con_handle)
{
	for(int i=0;i<(int)mv_servers.Size();i++){
		Server *s=(Server*)mv_servers.Get(i);//new Server(port,m_server_id_counter++);
		if(s->m_id==con_handle){
			closesocket(s->m_server_socket);
			mv_servers.Remove(i);
			return;
		}
	}
	//Server *ns=new Server(port,m_server_id_counter++);
	//mv_servers.Add(ns);
	//return ns->m_id;
}


void TKSocketSystem::CheckForNewConnections(void)
{
	//init any new servers that have been added (start listening)
	for(UINT i=0;i<mv_servers.Size();i++){
		Server *server=(Server*)mv_servers.Get(i);
		if( server->m_server_socket == NULL ){//****************** INIT SERVER ***********************//
			struct sockaddr_in local_addr;
			local_addr.sin_family		= AF_INET;
			local_addr.sin_addr.s_addr	= INADDR_ANY; 
			local_addr.sin_port			= htons((unsigned short)server->m_port);

			//create the listener socket
			server->m_server_socket = WSASocket(AF_INET, SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED); // TCP/IP socket

			if ( server->m_server_socket == INVALID_SOCKET ){  //does our m_server_socket handle suck?
				TRACE("COM SERVICE TKTCP::CheckForNewConnections() :  ERROR!  socket() failed with error %d.  The TrapperKeeper will NOT be handling incoming connections.\n",WSAGetLastError());
				OnServerError(server->m_id,TKTCP_SERVER_INIT_FAILED);
				mv_servers.Remove(i);
				i--;
				continue;
			}

			//we will have to bind the listener socket
			bool b_bind=false;
			int bind_count=0;
			while(!b_bind && bind_count++<10){  //try to bind a few times.
				if ( bind( server->m_server_socket , (struct sockaddr*)&local_addr , sizeof(local_addr) ) == SOCKET_ERROR ) {
					TRACE("COM SERVICE TKTCP::CheckForNewConnections() :  ERROR!  bind() to port %d failed with error %d.  The TrapperKeeper will NOT be handling incoming connections.\n",server->m_port,WSAGetLastError());
					Sleep(200);  //pause a moment
				}
				else{
					b_bind=true;
				}
			}

			if(!b_bind){
				closesocket(server->m_server_socket);
				OnServerError(server->m_id,TKTCP_SERVER_PORT_NOT_AVAILABLE);
				mv_servers.Remove(i);
				i--;
				continue;
			}

			//start listening
			if ( listen(  server->m_server_socket , min(SOMAXCONN,100) ) == SOCKET_ERROR ) { //a backlog of X sockets has been specified.
				TRACE("COM SERVICE TKTCP::CheckForNewConnections() :  ERROR!  listen() failed with error %d.  The TrapperKeeper will NOT be handling incoming connections.\n",WSAGetLastError());
				closesocket(server->m_server_socket);
				OnServerError(server->m_id,TKTCP_SERVER_INIT_FAILED);
				mv_servers.Remove(i);
				i--;
				continue;
			}		
		}//**************** END INIT SERVER *****************//
	}

	//**************** CHECK FOR NEW CONNECTIONS *************************//
	//we will use the select style so that we are not blocking indefinitely on an accept call.  Allowing us to properly cleanup our thread when the program wants to die.
	timeval timeout={0,0};  //give each accept call a zero timeout. (it just checks if any connections are pending, the OS has its own buffer for this socket handle)
	FD_ZERO(&m_server_set);
	for(UINT i=0;i<mv_servers.Size();i++){ //add all the servers to the select set
		Server *server=(Server*)mv_servers.Get(i);
		FD_SET(server->m_server_socket,&m_server_set);
	}
	
	int num=select(0,&m_server_set,0,0,&timeout);

	while(num>0 && num!=SOCKET_ERROR){
		for(UINT i=0;i<mv_servers.Size();i++){ //add all the servers to the select set
			Server *server=(Server*)mv_servers.Get(i);
			if(FD_ISSET(server->m_server_socket,&m_server_set)){
				struct sockaddr_in from_addr;
				int address_size=sizeof(sockaddr);
				SOCKET temp_handle=WSAAccept(server->m_server_socket,(struct sockaddr*)&from_addr,&address_size,NULL,0);
				if(temp_handle && temp_handle!=INVALID_SOCKET){
					//tell the system we have a new connection
					char *pcstr_addr=inet_ntoa(from_addr.sin_addr);
					//TRACE("COM SERVICE:  Com accepted a connection from %s.\n",pcstr_addr);
					//CSingleLock lock(&m_receivable_vector_lock,TRUE);

					//Connection *con=new Connection(temp_handle,pcstr_addr,server->m_port);
					//ModuleBin* bin=GetModuleBin(con->m_connection_info.GetHandle());
					//bin->AddConnection(con,this,false);
					//mv_invalid_module_bins.Add(bin);
					//mv_receivable_sockets.Add(con);
					//TKTCPConnection new_con=con->m_connection_info;

					UINT id=0;
					CSingleLock lock(&m_socket_lock,TRUE);
					for(int module_index=0;module_index<(int)mv_modules.Size();module_index++){
						TKSocketModule *module=(TKSocketModule*)mv_modules.Get(module_index);
						if(module->GetConnectionCount()<64){
							id=module->AddConnection(temp_handle,pcstr_addr,server->m_port,module_index);
							break;
						}
					}

					if(id==0){
						TKSocketModule *nm=new TKSocketModule(this);
						mv_modules.Add(nm);
						nm->StartThreadLowPriority();
						id=nm->AddConnection(temp_handle,pcstr_addr,server->m_port,mv_modules.Size()-1);
					}

					OnAcceptedConnection(server->m_id,id,pcstr_addr,server->m_port);
				}
				else{
					TRACE("COM SERVICE TKTCP::CheckForNewConnections() :  ERROR!  The accept() call failed in main server thread because of %d.  This is odd.\n",WSAGetLastError());
					closesocket(server->m_server_socket);
					OnServerError(server->m_id,TKTCP_UNKNOWN);	
					mv_servers.Remove(i);
					i--;
					continue;
				}
			}
		}
		FD_ZERO(&m_server_set);
		for(UINT i=0;i<mv_servers.Size();i++){ //add all the servers to the select set
			Server *server=(Server*)mv_servers.Get(i);
			FD_SET(server->m_server_socket,&m_server_set);
		}
		num=select(0,&m_server_set,0,0,&timeout);
	}
	//************* END CHECKING FOR NEW CONNECTIONS **********************//
}

void TKSocketSystem::StopSystem(void)
{
	for(int i=0;i<(int)mv_modules.Size();i++){
		TKSocketModule *m=(TKSocketModule*)mv_modules.Get(i);
		m->ShutdownModule();
	}
}

bool TKSocketSystem::IsBadCon(const char* ip)
{
	/*
	CSingleLock lock(&m_bad_con_lock,TRUE);
	UINT iip=inet_addr(ip);
	int index=iip%1000;
	for(int i=0;i<(int)mv_bad_cons[index].Size();i++){
		BadCon *bc=(BadCon*)mv_bad_cons[index].Get(i);
		if(bc->m_ip==iip)
			return true;
	}*/
	return false;
}

void TKSocketSystem::AddBadCon(const char* peer)
{
	/*
	CSingleLock lock(&m_bad_con_lock,TRUE);
	UINT iip=inet_addr(peer);
	int index=iip%1000;
	mv_bad_cons[index].Add(new BadCon(peer));
	*/
}

int TKSocketSystem::UDPSend(SOCKET hsock,const char* dest, unsigned short port,byte *p_data,UINT len){
	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = PF_INET;             
	sa.sin_port = htons(port);
	unsigned long addr=inet_addr(dest);
	sa.sin_addr = *(in_addr*)&addr;

	int sent=sendto(hsock,(const char*)p_data,len,0,(sockaddr*)&sa,sizeof(sa));

	return sent;
}

int TKSocketSystem::UDPReceive(SOCKET hsock,string &addr,unsigned short &port,byte *p_data){
	struct sockaddr_in from_addr;
	ZeroMemory(&from_addr,sizeof(from_addr));
	int address_size=sizeof(sockaddr);
	int nread=recvfrom(hsock,(char*)p_data,512,0,(sockaddr*)&from_addr,&address_size);

	addr=inet_ntoa(from_addr.sin_addr);
	port=ntohs(from_addr.sin_port);
	return nread;
}
UINT TKSocketSystem::GetNumberOfModules(void)
{
	return mv_modules.Size();
}
