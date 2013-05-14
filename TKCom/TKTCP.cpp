#include "StdAfx.h"
#include "tktcp.h"
#include <mmsystem.h>
#include <Ws2spi.h>


//a few globals that have scope only within this file.

UINT socket_id_counter=1;
UINT sockets_connecting=0;
UINT tcp_instances=0;
CCriticalSection id_lock;  //for synchronization so multiple connections and servers don't get the same handle.
string TKTCP::m_local_ip;

TKTCP::Server::Server(unsigned short port){
	m_server_socket=NULL;
	m_port=port;

	CSingleLock lock(&id_lock,TRUE);
	m_id=socket_id_counter++;
}

TKTCP::Connection::Connection(SOCKET handle,const char* peer,unsigned short peer_port)
:m_connection_info(peer,peer_port)
{
	m_peer=peer;
	m_peer_port=peer_port;
	
	if(socket_id_counter==0)
		socket_id_counter=1;

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
	m_wsa_recv_buffer.buf=(char*)new byte[(1<<12)];
	m_wsa_recv_buffer.len=(1<<12);
	mb_valid=true;
	m_socket_handle=handle;
	mb_awaiting_send_notification=false;
	m_error=TKTCP_UNKNOWN;
	mb_connected=false;

	mb_connecting=false;
	if(handle==NULL)
		mb_connecting=true;

	CSingleLock lock(&id_lock,TRUE);
	mp_owner_module=NULL;
	m_connection_info.SetHandle(socket_id_counter++);

}

void TKTCP::Connection::Clean(){
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
}

//returns true on success, false on failure
bool TKTCP::Connection::SendMore(){
	if(!mb_valid && mv_send_queue.Size()>0)
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

UINT TKTCP::Connection::Run(){
	//create the socket
	//SOCKET new_socket;
	while(sockets_connecting>100 && !this->b_killThread){
		Sleep(5);
	}
	if(this->b_killThread)
		return 0;
	sockets_connecting++;
	if( (m_socket_handle = WSASocket( AF_INET , SOCK_STREAM , IPPROTO_IP , NULL , 0 , WSA_FLAG_OVERLAPPED )) == INVALID_SOCKET)  {
		m_error=TKTCP_UNKNOWN;
		mb_valid=false;
		sockets_connecting--;
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
			sockets_connecting--;
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
		closesocket(m_socket_handle);
		m_socket_handle=NULL;
		m_error=TKTCP_CONNECT_FAILED;
		mb_valid=false;
		sockets_connecting--;
		return 0;
	}

	mb_connected=true;
	sockets_connecting--;
	return 0;
}

TKTCP::TKTCP(void)
{
	TRACE("TKTCP::TKTCP()  BEGIN\n");
	mb_update_mode=false;
//	mb_valid=false;
	//m_event_index=0;
	if(tcp_instances==0){
		  //if this is the first then we need to start up winsock
		InitWinSock();
	}
	tcp_instances++;
	//m_queued_data_size=0;

	for(int i=0;i<128;i++){
		m_module_bins[i].ref();  //add a reference to this object, so if it ends up in a vector, the vector won't try to delete it thinking noone is referencing it anymore
	}
	TRACE("TKTCP::TKTCP()  END\n");
}

TKTCP::~TKTCP(void)
{
	tcp_instances--;
	if(tcp_instances==0){ //is this the last?
		//CleanUpWinSock();  //then we need to shutdown winsock.
	}

	if(mv_connecting_sockets.Size()>0)
		ASSERT(0);
	if(mv_receivable_sockets.Size()>0)
		ASSERT(0);
	if(mv_sendable_sockets.Size()>0)
		ASSERT(0);
	if(mv_invalid_module_bins.Size()>0)
		ASSERT(0);
	if(mv_job_queue.Size()>0)
		ASSERT(0);
}

UINT TKTCP::OpenConnection(TKTCPConnection& con_ref)
{
	//TRACE("TKTCP::OpenConnection().\n");
	
	Connection *nc=new Connection(NULL,con_ref.GetPeer(),con_ref.GetPort());
	con_ref=nc->m_connection_info;
	
	mv_job_queue.Add(new TKTCPNewConnectionJob(nc));

	//TODO:  RETURN FAILURE IF THERE ARE TOO MANY MODULES IN EXISTANCE
	return TKTCP_OK;
}


void TKTCP::CloseConnection(TKTCPConnection &close_con)
{
	//TRACE("TKTCP::CloseConnection().\n");

	TKTCPCloseConnectionJob *dc=new TKTCPCloseConnectionJob(close_con);
	mv_job_queue.Add(dc);

	/*
	CSingleLock lock(&m_data_lock,TRUE);
	ModuleBin *bin=GetModuleBin(close_con.GetHandle());
	
	if(bin->CloseConnection(close_con.GetHandle())){
		mv_invalid_module_bins.Add(bin);
//		mb_valid=false;
	}
	*/
}

//This is a nonblocking call.
UINT TKTCP::SendData(TKTCPConnection &con_info,const byte* data, UINT length)
{
	//TRACE("TKTCP::SendData().\n");
	
	
	//m_queued_data_size+=length;
	TKTCPDataSendJob *ndsj=new TKTCPDataSendJob(data,length,con_info.GetHandle());
	mv_job_queue.Add(ndsj);

	return TKTCP_OK;
	
	
	/*
	//CSingleLock lock2(&m_data_lock,TRUE);
	Connection* con= GetModuleBin(con_info.GetHandle())->SendData(con_info.GetHandle(),new GenericBuffer(data,length));
	if(con){
		//CSingleLock lock2(&m_sendable_vector_lock,TRUE);
		mv_sendable_sockets.Add(con);
		return TKTCP_OK;
	}
	else return TKTCP_INVALID_HANDLE;
	*/
	

	//for(int i=0;i<(int)mv_modules.Size();i++){
	//	Module* module=(Module*)mv_modules.Get(i);
	//	CSingleLock lock(&module->m_socket_vector_lock,TRUE);
	//	Connection * con=module->GetConnection(handle);
	//Connection *con = GetConnection(handle);
	//if(con){
	//	CSingleLock lock2(&module->m_sendable_vector_lock,TRUE);
	//	con->mv_send_queue.Add(new GenericBuffer(data,length));
	//	module->mv_sendable_sockets.Add(con);
	//	return TKTCP_OK;
	//}
	//}

	//return TKTCP_INVALID_HANDLE;
	/*
	Connection * con=GetConnection(handle);

	if(con && con->mb_valid){
		CSingleLock lock2(&m_sendable_vector_lock,TRUE);
		con->mv_send_queue.Add(new GenericBuffer(data,length));
		mv_sendable_sockets.Add(con);
		return TKTCP_OK;
	}
	else 
		return TKTCP_INVALID_HANDLE;
		*/
}

void TKTCP::InitWinSock(void)
{
	TRACE("TKTCP::InitWinSock() BEGIN\n");
	
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
	
	TRACE("TKTCP::InitWinSock() END\n");
}

void TKTCP::CleanUpWinSock(void)
{
	WSACleanup();
}

void TKTCP::SyncReceiving()
{
	//start the receiving process on any new connection
	
//	CSingleLock lock(&m_data_lock,TRUE);

	//Vector v_tmp;
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
				Module *mod=(Module*)con->mp_owner_module;
				mod->mb_valid=false;
				mv_invalid_module_bins.Add(mod->mp_owner_bin);
			}
		}
		else if(!con->mb_needs_receive || !con->mb_valid){
			mv_receivable_sockets.Remove(i);
			i--;
		}
	}		

	//mv_receivable_sockets.Clear();
	//mv_receivable_sockets.Copy(&v_tmp);
	
	

	
	/*
	for(UINT i=0;i<mv_sockets.Size();i++){
		Connection *con=(Connection*)mv_sockets.Get(i);
		if(!con->mb_connecting && con->mb_valid && con->mb_needs_receive){
			//TRACE("Receiving on socket %d.\n",i);
			con->mb_needs_receive=false;  //its only new once
			DWORD temp,flags=0;
			WSAResetEvent(con->m_event_recv);

			UINT result=WSARecv(con->m_handle,&con->m_wsa_recv_buffer,1,&temp,&flags,&con->m_ol_recv,NULL);

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
				con->mb_valid=false;
				mb_valid=false;
			}
		}
	}		
	*/
	
}

void TKTCP::SyncSends()
{
	
	
	//CSingleLock lock(&m_sendable_vector_lock,TRUE);
	//send some more data if possible
	//Vector v_tmp;
	for(UINT i=0;i<mv_sendable_sockets.Size();i++){
		Connection *con=((Connection*)mv_sendable_sockets.Get(i));
		if(!con->mb_connecting && con->mb_valid && !con->mb_awaiting_send_notification && con->mv_send_queue.Size()>0){
			con->m_wsa_send_buffer.buf=(char*)((GenericBuffer*)con->mv_send_queue.Get(0))->m_data;
			con->m_wsa_send_buffer.len=((GenericBuffer*)con->mv_send_queue.Get(0))->m_data_length;
			//TRACE("Sending data of length %d to %s\n",((GenericBuffer*)con->mv_send_queue.Get(0))->m_data_length,con->m_peer.c_str());
			if(!con->SendMore()){
				//TRACE("TKTCP::SyncSends, couldn't send more.\n");
				con->mb_valid=false;
				Module *mod=(Module*)con->mp_owner_module;
				mod->mb_valid=false;
				mv_invalid_module_bins.Add(mod->mp_owner_bin);
			}
		}
		else if(!con->mb_needs_send || !con->mb_valid){
			mv_sendable_sockets.Remove(i);
			i--;
		}
	}
	//mv_sendable_sockets.Clear();
	//mv_sendable_sockets.Copy(&v_tmp);
	

	/*
	for(UINT i=0;i<mv_sockets.Size();i++){
		Connection *con=(Connection*)mv_sockets.Get(i);
		if(!con->mb_connecting && con->mb_valid && !con->mb_awaiting_send_notification && con->mv_send_queue.Size()>0){
			con->m_wsa_send_buffer.buf=(char*)((GenericBuffer*)con->mv_send_queue.Get(0))->m_data;
			con->m_wsa_send_buffer.len=((GenericBuffer*)con->mv_send_queue.Get(0))->m_data_length;
			//TRACE("Sending data of length %d to %s\n",((GenericBuffer*)con->mv_send_queue.Get(0))->m_data_length,con->m_peer.c_str());
			if(!con->SendMore()){
				//TRACE("TKTCP::SyncSends, couldn't send more.\n");
				con->mb_valid=false;
				mb_valid=false;  //note that we need to remove this invalid connection later
			}
		}
	}*/
	
}

void TKTCP::CheckForNewConnections(void)
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

					Connection *con=new Connection(temp_handle,pcstr_addr,server->m_port);
					ModuleBin* bin=GetModuleBin(con->m_connection_info.GetHandle());
					bin->AddConnection(con,this,false);
					mv_invalid_module_bins.Add(bin);
					mv_receivable_sockets.Add(con);
					TKTCPConnection new_con=con->m_connection_info;

					OnAcceptedConnection(server->m_id,new_con);
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

UINT TKTCP::Listen(unsigned short port)
{
	Server *ns=new Server(port);
	mv_servers.Add(ns);
	return ns->m_id;
}

void TKTCP::OnAcceptedConnection(UINT server_handle,TKTCPConnection &new_con)
{
	/*TRACE("TKTCP::OnAcceptedConnection().\n");*/
	CloseConnection(new_con);  //if you don't override this function the incoming connection just gets closed.
}

void TKTCP::StopTCP(void)
{
	TRACE("TKTCP::StopTCP(void) STARTED.\n");

	//TRACE("TKTCP::StopTCP(void) About to stop the module bins.\n");
	//********* SHUTDOWN ***********//
	for(int i=0;i<128;i++){
		m_module_bins[i].StopTCP();
		//TRACE("Stopping bin %d.\n",i);
	}
	//TRACE("TKTCP::StopTCP(void) Finshed stopping module bins.\n");

	//TRACE("TKTCP::StopTCP(void) About to stop job %d queues.\n",mv_job_queue.Size());
	while(mv_job_queue.Size()>0){
		
		TKTCPJob *job=(TKTCPJob *)mv_job_queue.Get(0);
		if(job->IsNewConnectionJob()){
			TKTCPNewConnectionJob *new_con_job=(TKTCPNewConnectionJob *)job;
			Connection *nc=new_con_job->mp_new_connection;
			nc->StopThread();
			nc->Clean();
		}

		mv_job_queue.Remove(0);
	}

	mv_job_queue.Clear();
	mv_connecting_sockets.Clear();
	mv_receivable_sockets.Clear();
	mv_sendable_sockets.Clear();
	mv_invalid_module_bins.Clear();
	

	//TRACE("TKTCP::StopTCP(void) Finshed stopping job queues.\n");

	/*
	for(int i=0;i<(int)mv_modules.Size();i++){
		Module* module=(Module*)mv_modules.Get(i);
		for(UINT j=0;j<module->mv_sockets.Size();j++){
			Connection *con=(Connection*)module->mv_sockets.Get(j);
			con->StopThread();
			con->Clean();
		}		
	}*/

	/*
	for(UINT i=0;i<mv_sockets.Size();i++){
		Connection *con=(Connection*)mv_sockets.Get(i);
		con->StopThread();
	}		

	for(UINT i=0;i<mv_sockets.Size();i++){
		Connection *con=(Connection*)mv_sockets.Get(i);
		con->Clean();
	}		*/

	for(UINT i=0;i<mv_servers.Size();i++){
		Server *server=(Server*)mv_servers.Get(i);
		if(server->m_server_socket)
			closesocket(server->m_server_socket);
	}	

	TRACE("TKTCP::StopTCP(void) FINISHED.\n");
}

UINT TKTCP::GetConnectionCount(void)
{
	UINT sum=0;
	for(UINT i=0;i<128;i++){
		sum+=m_module_bins[i].m_connection_count;
	}
	return sum;
}


/*
bool TKTCP::GetPeerInfo(UINT handle,string &peer_name,unsigned short &port)
{
	//
	//signal the socket to close.
	ModuleBin *bin=GetModuleBin(handle);
	Connection *con=bin->GetConnection(handle);
	if(con){
		peer_name=con->m_peer;
		port=con->m_peer_port;			
		return true;
	}
	else return false;
}
*/

void TKTCP::Update(void)
{

//	CSingleLock lock(&m_data_lock,TRUE);
	while(true){  //it is always best to use a lot of infinite loops.
		//The real purpose of this loop is to dispatch all pending notifications before the function returns.
		//Are we ready to accept any new connections?
		
		//_ASSERTE( _CrtCheckMemory( ) );
		CheckForNewConnections();

		Vector v_delayed_connections;
		for(int i=0;i<(int)mv_job_queue.Size();i++){
			TKTCPJob *job=(TKTCPJob *)mv_job_queue.Get(0);
			if(job->IsNewConnectionJob()){
				//if(sockets_connecting<30){
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
				//}
				//else{
				//	v_delayed_connections.Add(job);
				//	continue;
				//}
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
		/*
		while(mv_job_queue.Size()>0){
			
			TKTCPJob *job=(TKTCPJob *)mv_job_queue.Get(0);

			if(job->IsNewConnectionJob()){
				TKTCPNewConnectionJob *new_con_job=(TKTCPNewConnectionJob *)job;
				Connection *nc=new_con_job->mp_new_connection;
				ModuleBin* bin=GetModuleBin(nc->m_connection_info.GetHandle());
				if(!bin){ASSERT(0);}
				bin->AddConnection(nc,this,true);
				nc->StartThread();
				mv_invalid_module_bins.Add(bin);
				mv_receivable_sockets.Add(nc);
				mv_connecting_sockets.Add(nc);			
			}
			
			else if(job->IsCloseConnectionJob()){
				UINT handle=((TKTCPConnection*)job)->GetHandle();
				ModuleBin *bin=GetModuleBin(handle);
				
				if(bin->CloseConnection(handle)){
					mv_invalid_module_bins.Add(bin);
				}				
			}
			else if(job->IsDataSendJob()){
				TKTCPDataSendJob *dsj=(TKTCPDataSendJob*)job;
				Connection* con=GetModuleBin(dsj->m_handle)->SendData(dsj->m_handle,dsj->m_data);
				if(con){
					//CSingleLock lock2(&m_sendable_vector_lock,TRUE);
					mv_sendable_sockets.Add(con);
				}		
			}
			
			mv_job_queue.Remove(0);
		}
		*/

		//_ASSERTE( _CrtCheckMemory( ) );
		
		for(UINT i=0;i<mv_connecting_sockets.Size();i++){
			Connection *con=(Connection*)(Object*)mv_connecting_sockets.Get(i);
			if(con->mb_connected && con->mb_connecting){  //did this socket recently connect?
				TKTCPConnection notify_con(con->m_peer.c_str(),con->m_peer_port);
				notify_con.SetHandle(con->m_connection_info.GetHandle());
				OnConnect(notify_con);  //do a one time notification
				con->mb_connecting=false; //do it only one time.
				mv_connecting_sockets.Remove(i);
				i--;
				continue;
			}
			else if(!con->mb_valid){
				Module* mod=(Module*)con->mp_owner_module;
				//ASSERT(mod);
				mod->mb_valid=false;
				mv_invalid_module_bins.Add(mod->mp_owner_bin);
				mv_connecting_sockets.Remove(i);
				i--;
				continue;
			}
		}
		
		while(mv_invalid_module_bins.Size()>0){
			ModuleBin* bin=(ModuleBin*) mv_invalid_module_bins.Get(0);
			bin->Update();
			mv_invalid_module_bins.Remove(0);
		}

		//start WSARecv's on any sockets that need it
		SyncReceiving();

		//send more data on any sockets that have data waiting.
		SyncSends();

		//_ASSERTE( _CrtCheckMemory( ) );

		DWORD ret=WSA_WAIT_TIMEOUT;

		UINT module_index=0;
		UINT bin_index=0;
		mb_update_mode=!mb_update_mode;
		
		//in the end we must check each socket for an event, there is no way around this
		
		for(bin_index=0;bin_index<128;bin_index++){
			bool b_stop=false;
			for(module_index=0;module_index<m_module_bins[bin_index].mv_modules.Size();module_index++){  //check any connecting sockets in each module
				Module* module=(Module*)m_module_bins[bin_index].mv_modules.Get(module_index);
				if(module->m_event_count>64)
					ASSERT(0);
				if(mb_update_mode)
					ret=WSAWaitForMultipleEvents(module->m_event_count, module->m_send_event_array, FALSE, 0, FALSE);  //will not wait for an event, but checking for one
				else
					ret=WSAWaitForMultipleEvents(module->m_event_count, module->m_rcv_event_array, FALSE, 0, FALSE);  //will not wait for an event, but checking for one
				
				if(ret==WSA_WAIT_TIMEOUT || ret==WSA_WAIT_FAILED){  //has there been any activity?
					if(ret==WSA_WAIT_FAILED){
						//TRACE("TKTCP::WSA_WAIT_FAILED.\n");
					}
					continue;  //nothing happening in this module, go on to the next;
				}
				else{
					b_stop=true;
					break;  //something happened, break and handle this event
				}
			}
			if(b_stop)
				break;
		}
		
		//_ASSERTE( _CrtCheckMemory( ) );
		if(ret==WSA_WAIT_TIMEOUT || ret==WSA_WAIT_FAILED){  //has there been any activity?
			if(ret==WSA_WAIT_FAILED){
				//TRACE("TKTCP::WSA_WAIT_FAILED.\n");
			}
			
			return;  //Nothing happened, lets return
		}

		Connection *con=(Connection*)(Object*)((Module*)m_module_bins[bin_index].mv_modules.Get(module_index))->mv_sockets.Get(ret);
		if(!con){
			ASSERT(0);
		}

		//_ASSERTE( _CrtCheckMemory( ) );
		//CHECK FOR RECV EVENT
		if(!mb_update_mode){  //*******************RECV EVENT HANDLER********************//
			//It is a recv event
			DWORD num_bytes=0,dw_flags=0;
			//get the result
			if( WSAGetOverlappedResult( con->m_socket_handle , &con->m_ol_recv , &num_bytes , FALSE , &dw_flags ) == TRUE ) {
				if(num_bytes==0){  //if number of bytes received is 0, then that means the connection was closed
					con->mb_valid=false;
					con->m_error=TKTCP_CONNECTION_CLOSED;

					Module* mod=(Module*)con->mp_owner_module;
					mod->mb_valid=false;
					mv_invalid_module_bins.Add(mod->mp_owner_bin);

				}
				else{ //we successfully received data
					TKTCPConnection notify_con(con->m_peer.c_str(),con->m_peer_port);
					notify_con.SetHandle(con->m_connection_info.GetHandle());
					OnReceive(notify_con,(byte*)con->m_wsa_recv_buffer.buf,num_bytes); //notify clients

					//tell it to recv data again
					mv_receivable_sockets.Add((Object*)con);
					//mv_receivable_sockets.Add(con);
					con->mb_needs_receive=true;
				}
			}
			else {  //we shouldn't get here, but if we do its definitely a wierd error.
				//TRACE("MetaTCP receive event handler get result call failed.\n");
				con->m_error=TKTCP_RECV_FAILED;
				con->mb_valid=false;
				Module* mod=(Module*)con->mp_owner_module;
				mod->mb_valid=false;
				mv_invalid_module_bins.Add(mod->mp_owner_bin);
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
				ASSERT(con->mv_send_queue.Size()>0);  //supposedly impossible

				if(con->m_wsa_send_buffer.len==0){ //Are we done sending?
					con->mb_awaiting_send_notification=false;
					con->mv_send_queue.Remove(0);

					TKTCPConnection notify_con(con->m_peer.c_str(),con->m_peer_port);
					notify_con.SetHandle(con->m_connection_info.GetHandle());
					OnSend(notify_con);
					if(con->mv_send_queue.Size()>0){
						mv_sendable_sockets.Add((Object*)con);
					}
				}
				else{ //*******NOT DONE SENDING, SEND MORE************//
					//TRACE("MetaTCP send event handler not done sending.\n");

					//TEST CODE -  EFFICIENCY IMPROVEMENT
					//mv_sendable_sockets.Add(con);
					mv_sendable_sockets.Add((Object*)con);
					
					if(!con->SendMore()){  //send more of the buffer
						Module* mod=(Module*)con->mp_owner_module;
						mod->mb_valid=false;
						mv_invalid_module_bins.Add(mod->mp_owner_bin);
						con->mb_valid=false;
					}
				}
			}
			else { //Did the get result call fail?
				//TRACE("MetaTCP send event handler get result call failed.\n");
				con->m_error=TKTCP_SEND_FAILED;
				con->mb_valid=false;
				Module* mod=(Module*)con->mp_owner_module;
				mod->mb_valid=false;
				mv_invalid_module_bins.Add(mod->mp_owner_bin);
			}
			//_ASSERTE( _CrtCheckMemory( ) );
		}//***********END SEND EVENT HANDLER*************//
	}
}

//this function takes a lot of cpu time, don't call it very often
UINT TKTCP::GetQueuedDataSize(void)
{
	//return m_queued_data_size;
	
	//a brute force algorithm, don't call this a lot
	UINT total_len=0;
	for(int m=0;m<128;m++){
		for(UINT i=0;i<m_module_bins[m].mv_modules.Size();i++){
			Module* module=(Module*)m_module_bins[m].mv_modules.Get(i);
			CSingleLock lock(&module->m_socket_vector_lock,TRUE);	
			
			for(UINT k=0;k<module->mv_sockets.Size();k++){
				Connection *con=(Connection*)module->mv_sockets.Get(k);
				for(UINT j=0;j<con->mv_send_queue.Size();j++){
					GenericBuffer *gb=(GenericBuffer*)con->mv_send_queue.Get(j);
					total_len+=gb->m_data_length;
				}
			}
		}
	}

	return total_len;
	
	/*
	CSingleLock lock(&m_socket_vector_lock,TRUE);
	//signal the socket to close.
	UINT total_len=0;
	for(UINT i=0;i<mv_sockets.Size();i++){
		Connection *con=(Connection*)mv_sockets.Get(i);
		for(UINT j=0;j<con->mv_send_queue.Size();j++){
			GenericBuffer *gb=(GenericBuffer*)con->mv_send_queue.Get(j);
			total_len+=gb->m_data_length;
		}
	}

	return total_len;
	*/
}


//the connections are always sorted, thus we can use binary search
TKTCP::Connection* TKTCP::GetConnection(UINT id)
{
	ModuleBin* bin=GetModuleBin(id);
	return bin->GetConnection(id);
	/*
	int low=0;
	int high=max(0,mv_sockets.Size()-1);
	int index=0;
	//do a binary search to determine where we should insert this file into the array
	for(;low<high;){
		index=low+((high-low)>>1);
		Connection* c=(Connection*)mv_sockets.Get(index);
		int result=0;
		if(id<c->m_id)
			result=-11;
		else if(id>c->m_id)
			result=1;
		if(result<0){
			high=index-1;  //we know that our bounds are limited to the middle-1.
		}
		else if(result>0){
			low=index+1;
		}
		else{  
			break;
		}
	}

	if(mv_sockets.Size()>0){
		Connection* c=(Connection*)mv_sockets.Get(index);
		if(c->m_id==id){
			return c;
		}

		//this inefficiency is imposed on us by the premise that we can theoretically have duplicate hashes (files of different sizes actually having the same hash)
		for(int i=low;i<=high;i++){
			Connection* c=(Connection*)mv_sockets.Get(i);
			if(c->m_id==id){
				return c;
			}
		}
	}
	return NULL;
	*/
}


void TKTCP::Module::Update()
{
	mb_valid=true;
	m_event_count=0;
	for(UINT i=0;i<mv_sockets.Size();i++){
		Connection *con=(Connection*)mv_sockets.Get(i);
		if(!con->mb_valid){
			//TRACE("TKTCP::Update() Removing socket because it isn't valid.\n");
			closesocket(con->m_socket_handle);
			con->Clean();
			if(con->m_error!=TKTCP_REQUEST_CLOSE){  //only do a onclose notification if its not a socket that the user specifically requested to close
				TKTCPConnection notify_con(con->m_peer.c_str(),con->m_peer_port);
				notify_con.SetHandle(con->m_connection_info.GetHandle());
				
				mp_owner->OnClose(notify_con,con->m_error);  
			}
			con->StopThread();
//			CSingleLock lock(&m_socket_vector_lock,TRUE);
//			mv_dead_sockets.Add(con);
			mv_sockets.Remove(i);
			i--;
			continue;
		}

		//m_indexing_array[m_event_index>>1]=i;
		m_send_event_array[m_event_count]=con->m_event_send;
		m_rcv_event_array[m_event_count]=con->m_event_recv;
		m_event_count++;
	}

}


bool TKTCP::Module::IsFull(void)
{
	return mv_sockets.Size()>=64;
}

/*
UINT TKTCP::Module::AddConnection(const char* address, unsigned short port)
{
	CSingleLock lock(&m_receivable_vector_lock,TRUE);
	Connection *nc=new Connection(NULL,address,port);

	mv_receivable_sockets.Add(nc);
	mv_connecting_sockets.Add(nc);
	nc->StartThread();
	mv_sockets.Add(nc);
	//mv_new_sockets.Add(nc);
	mb_valid=false;
	return nc->m_id;
}
*/
void TKTCP::Module::AddConnection(TKTCP::Connection* con,bool b_new)
{
	
	con->mp_owner_module=this;
	mv_sockets.Add(con);
	
	if(b_new){
		//mv_connecting_sockets.Add(nc);
		//con->StartThread();
	}
	
	mb_valid=false;
}

bool TKTCP::Module::CloseConnection(UINT handle)
{
	//CSingleLock lock(&m_socket_vector_lock,TRUE);
	//signal the socket to close.
	Connection *con=GetConnection(handle);

	if(con){
		closesocket(con->m_socket_handle); //close it immediately.
		con->m_socket_handle=NULL;
		con->m_error=TKTCP_REQUEST_CLOSE;
		con->mb_valid=false;
		mb_valid=false;
		return true;
	}

	return false;
}

TKTCP::Connection* TKTCP::Module::GetConnection(UINT handle)
{
	for(int i=0;i<(int)mv_sockets.Size();i++){
		Connection *con=(Connection*)mv_sockets.Get(i);
		if(con->m_connection_info.GetHandle()==handle){
			return con;
		}
	}
	return NULL;
}

//if b_new is true, then the connection was initiated from this endpoint, as opposed to an accepted connection
void TKTCP::ModuleBin::AddConnection(TKTCP::Connection* con,TKTCP* tktcp,bool b_new)
{
	
	//find a module to place this socket into
	for(int i=0;i<(int)mv_modules.Size();i++){
		Module* module=(Module*)mv_modules.Get(i);
		if(!module->IsFull()){

			module->AddConnection(con,b_new);
			return;
		}
	}

	
	//we got here so that must mean there are no avaible modules and we must create a new one
	Module *nm=new Module(tktcp);
	nm->mp_owner_bin=this;
	mv_modules.Add(nm);
	nm->AddConnection(con,b_new);
}

TKTCP::ModuleBin* TKTCP::GetModuleBin(UINT handle)
{
	int index=handle&0x7f;
	//TRACE("TKTCP::GetModuleBin() return module with index %d.\n",index);
	return &m_module_bins[index];
}

TKTCP::Connection* TKTCP::ModuleBin::SendData(UINT handle,TKTCP::GenericBuffer *data){
	for(int i=0;i<(int)mv_modules.Size();i++){
		Module* module=(Module*)mv_modules.Get(i);
		//CSingleLock lock(&module->m_socket_vector_lock,TRUE);
		Connection * con=module->GetConnection(handle);
		if(con && con->mb_valid){
			//CSingleLock lock2(&module->m_sendable_vector_lock,TRUE);
			con->mv_send_queue.Add(data);
			//module->mv_sendable_sockets.Add(con);
			return con;
		}
	}
	//delete data;
	return NULL;
}

bool TKTCP::ModuleBin::CloseConnection(UINT handle){

	for(int i=0;i<(int)mv_modules.Size();i++){
		Module* module=(Module*)mv_modules.Get(i);
		if(module->CloseConnection(handle)){
			return true;
		}
	}
	return false;
}

TKTCP::Connection* TKTCP::ModuleBin::GetConnection(UINT handle)
{
	for(int i=0;i<(int)mv_modules.Size();i++){
		Module* module=(Module*)mv_modules.Get(i);
		//CSingleLock lock(&module->m_socket_vector_lock,TRUE);
		Connection * con=module->GetConnection(handle);
		if(con){
			return con;
		}
	}
	return NULL;
}

void TKTCP::ModuleBin::StopTCP(void)
{
	for(int i=0;i<(int)mv_modules.Size();i++){
		Module* module=(Module*)mv_modules.Get(i);
		for(UINT j=0;j<module->mv_sockets.Size();j++){
			Connection *con=(Connection*)module->mv_sockets.Get(j);
			con->StopThread();
			con->Clean();
		}		
	}
	mv_modules.Clear();
}

void TKTCP::ModuleBin::Update(void)
{
	UINT tmp_con_count=0;
	
	for(int i=0;i<(int)mv_modules.Size();i++){
		Module* module=(Module*)mv_modules.Get(i);
		if(!module->mb_valid){
			module->Update();
		}
		if(module->mv_sockets.Size()==0){
			mv_modules.Remove(i);
			i--;
		}
		else{
			tmp_con_count+=module->mv_sockets.Size();
		}
	}
	m_connection_count=tmp_con_count;
}

/*
UINT TKTCP::ModuleBin::GetQueuedDataSize(void)
{
	UINT total_len=0;
	CSingleLock lock(&m_bin_lock,TRUE);
	for(UINT i=0;i<mv_modules.Size();i++){
		Module* module=(Module*)m_module_bins[m].mv_modules.Get(i);
		CSingleLock lock(&module->m_socket_vector_lock,TRUE);	
		
		for(UINT k=0;k<module->mv_sockets.Size();k++){
			Connection *con=(Connection*)module->mv_sockets.Get(k);
			for(UINT j=0;j<con->mv_send_queue.Size();j++){
				GenericBuffer *gb=(GenericBuffer*)con->mv_send_queue.Get(j);
				total_len+=gb->m_data_length;
			}
		}
	}
	return total_len;
}*/
