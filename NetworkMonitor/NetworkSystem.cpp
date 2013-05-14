#include "StdAfx.h"
#include "networksystem.h"
#include "NameServerInterface.h"
#include "Network.h"
#include "Rack.h"
#include <mmsystem.h>
#include "ProcessObject.h"
#include "DLLObject.h"
#include "..\tkcom\tinysql.h"
#include "..\..\RackRecoverySystem\RackRecoverInterface.h"

NetworkSystem::NetworkSystem(void)
{
	float m_percent_alive=0;
	float m_percent_not_crashed=0;

	int m_alive=0;
	int m_dead=0;
	int m_crashed=0;

	mb_processes_changed=false;
	mb_dlls_changed=false;
}

NetworkSystem::~NetworkSystem(void)
{
}

void NetworkSystem::Shutdown(void)
{
	m_com_system.Shutdown();
	this->StopThread();
}

UINT NetworkSystem::Run()
{
	//do processing
	TRACE("NetworkSystem::Run().\n");
	CTime next_thin_time=CTime::GetCurrentTime()+CTimeSpan(0,0,2,0);
	CTime next_rack_update=CTime::GetCurrentTime()+CTimeSpan(0,0,1,0);
	CTime next_process_update=CTime::GetCurrentTime()+CTimeSpan(0,0,1,0);

	CTime next_comment_update=CTime::GetCurrentTime()+CTimeSpan(0,0,0,4);
	CTime next_stat_update=CTime::GetCurrentTime()+CTimeSpan(0,0,0,4);
	CTime next_save_update=CTime::GetCurrentTime()+CTimeSpan(0,0,0,10);

	CTime next_dead_export=CTime::GetCurrentTime()+CTimeSpan(0,0,4,0);

	int rack_broad_cast_index=0;
	CTime m_next_send_rotation=CTime::GetCurrentTime()+CTimeSpan(0,0,0,30);

	srand(timeGetTime());
	while(!this->b_killThread){

		//update the comments for each rack
		if(CTime::GetCurrentTime()>next_comment_update){
			TinySQL sql1;
			sql1.Init("38.118.160.161","onsystems","ebertsux37","network_management",3306);
		
			TinySQL sql2;
			sql2.Init("38.118.160.161","onsystems","ebertsux37","network_management",3306);
		

			if(sql1.Query("select ip,comment from comments_by_rack",true) && sql2.Query("select network,comment from comments_by_network",true)){  
				//if we successfully queried, then lets clear all the old rack comments
				for(int i=0;i<(int)mv_racks.Size();i++){
					Rack* rack=(Rack*)mv_racks.Get(i);
					rack->m_comment="";
				}
				
				for(int i=0;i<(int)sql2.m_num_rows;i++){
					string network=sql2.mpp_results[i][0];
					string comment=sql2.mpp_results[i][1];

					for(int j=0;j<(int)mv_networks.Size();j++){
						Network* n=(Network*)mv_networks.Get(j);
						if(stricmp(n->m_name.c_str(),network.c_str())==0){
							for(int k=0;k<(int)n->mv_racks.Size();k++){
								Rack* rack=(Rack*)n->mv_racks.Get(k);
								rack->m_comment+=comment;
							}
							break;
						}
					}
				}

				for(int i=0;i<(int)sql1.m_num_rows;i++){
					string ip=sql1.mpp_results[i][0];
					string comment=sql1.mpp_results[i][1];

					for(int i=0;i<(int)mv_racks.Size();i++){
						Rack* rack=(Rack*)mv_racks.Get(i);
						if(stricmp(rack->m_ip.c_str(),ip.c_str())==0){
							if(rack->m_comment.size()>0)
								rack->m_comment+=" - ";  //if it has network comments already, seperate the two with a delimiter
							rack->m_comment+=comment;
							break;
						}
					}
				}
			}
			next_comment_update=CTime::GetCurrentTime()+CTimeSpan(0,0,15,0);  //update the racks every several minutes
		}
		
		//update the statistics
		if(CTime::GetCurrentTime()>next_stat_update){
			int alive=0;
			int dead=0;
			int crashed=0;
				
			for(int i=0;i<(int)mv_racks.Size();i++){
				Rack* rack=(Rack*)mv_racks.Get(i);
				if(rack->IsAlive()){
					alive++;
					if(!rack->IsTKOn()){
						crashed++;
					}
				}
				else{
					dead++;
				}	
			}

			float percent_alive=(float)alive/(float)max(1,(alive+dead));
			percent_alive*=100.0f;

			float percent_not_crashed=(float)crashed/(float)max(1,(alive));
			percent_not_crashed=1.0f-percent_not_crashed;
			percent_not_crashed*=100.0f;

			m_alive=alive;
			m_dead=dead;
			m_crashed=crashed;

			m_percent_alive=percent_alive;
			m_percent_not_crashed=percent_not_crashed;
			next_stat_update=CTime::GetCurrentTime()+CTimeSpan(0,0,0,10);  //update the racks every several minutes
		}


		//update the dll objects
		if(CTime::GetCurrentTime()>next_rack_update){
			for(int i=0;i<(int)mv_racks.Size();i++){
				Rack* rack=(Rack*)mv_racks.Get(i);
				rack->Update();
			}
			next_rack_update=CTime::GetCurrentTime()+CTimeSpan(0,0,3,0);  //update the racks every several minutes
		}

		//export dead racks
		if(CTime::GetCurrentTime()>next_dead_export){
			ExportDeadRacks();
			ExportCrashedRacks();
			next_dead_export=CTime::GetCurrentTime()+CTimeSpan(0,0,10,0);  //update the racks every several minutes
		}


		//save our rack history
		if(CTime::GetCurrentTime()>next_save_update){
			SaveHistory();
			next_save_update=CTime::GetCurrentTime()+CTimeSpan(0,0,10,0);  //save the history every 10 minutes
		}


		//update the process and dll objects
		if(CTime::GetCurrentTime()>next_process_update){

			//delete any processes that don't have any racks that are running them
			for(int i=0;i<(int)mv_processes.Size();i++){
				ProcessObject* po=(ProcessObject*)mv_processes.Get(i);
				if(!po->Update()){
					mv_processes.Remove(i);
					i--;
				}
			}

			//delete any old dlls
			for(int i=0;i<(int)mv_dlls.Size();i++){
				DLLObject* dll_object=(DLLObject*)mv_dlls.Get(i);
				if(dll_object->IsExpired()){
					mv_dlls.Remove(i);
					i--;
				}
			}

			next_process_update=CTime::GetCurrentTime()+CTimeSpan(0,0,3,0);  //update the racks every several minutes
		}

		//mv_on_time thinning algorithm
		if(CTime::GetCurrentTime()>next_thin_time){
			for(int i=2;i<(int)mv_on_time.size() && !this->b_killThread;i++){
				//PingResponse *pr=(PingResponse*)mv_ping_responses.Get(i);

				if(mv_on_time[i]<CTime::GetCurrentTime()-CTimeSpan(20,0,0,0)){
					CSingleLock lock(&m_on_time_vector_lock,TRUE);
					mv_on_time.erase(mv_on_time.begin()+i);
					i--;
					continue;  //discard a ping response if it is older than 30 days.  We don't need to remember this far back in time
				}

				//PingResponse *mid_pr=(PingResponse*)mv_ping_responses.Get(i-1);
				//PingResponse *last_pr=(PingResponse*)mv_ping_responses.Get(i-2);
				
				CTimeSpan dif_time=mv_on_time[i]-mv_on_time[i-2];
				if(dif_time<CTimeSpan(0,0,40,0) ){ //if first, and last are less than 40 minutes apart, we don't need the one in the middle
					CSingleLock lock(&m_on_time_vector_lock,TRUE);
					mv_on_time.erase(mv_on_time.begin()+(i-1));
					//mv_ping_responses.Remove(i-1);  //remove the middle one if it is unnecessary
					i--;
					continue;
				}
			}

			//schedule another thinning at some time in the future, so we don't run through this algorithm unnecessarily
			next_thin_time=CTime::GetCurrentTime()+CTimeSpan(0,0,20,0);
		}

		
		//handle any data we have received
		int receive_count=0;
		while(receive_count++<20 && m_com_system.HasReceived()){
			OnReceive();
		}

		//broadcast our pings and stuff
		int send_count=0;
		while(send_count++<15){
			//	int rack_broad_cast_index=0;
			//	CTime m_next_send_rotation==CTime::GetCurrentTime()+CTimeSpan(0,0,0,60);
			if(rack_broad_cast_index>=(int)mv_racks.Size()){
				if(CTime::GetCurrentTime()>m_next_send_rotation){
					rack_broad_cast_index=0;  //set it back to zero
					m_next_send_rotation=CTime::GetCurrentTime()+CTimeSpan(0,0,0,30);
				}
			}
			else{
				Rack *rack=(Rack*)mv_racks.Get(rack_broad_cast_index);
				
				/*
				Network *nw=GetNetwork("Marina22");
				if(!nw){
					rack_broad_cast_index++;
					continue;
				}

				//test if its one of our test racks
				if(!nw->HasRack(rack->m_ip.c_str()) && stricmp(rack->m_ip.c_str(),"38.119.66.24")!=0){
					rack_broad_cast_index++;
					continue;
				}
				*/

				//TRACE("NetworkMonitor::NetworkSystem::Run() Broadcasting Ping to %s.\n",rack->m_ip.c_str());
				//is it time to send an ip request?
				m_com_system.Send(rack->m_ip.c_str(),RACKPING);

				//is it time to send a process request?
				if(rack->m_next_process_request<CTime::GetCurrentTime()){
					//TRACE("NetworkMonitor::NetworkSystem::Run() Broadcasting Process Request to %s.\n",rack->m_ip.c_str());	
					m_com_system.Send(rack->m_ip.c_str(),RACKPROCESSES);
					rack->m_next_process_request=CTime::GetCurrentTime()+CTimeSpan(0,0,10+rand()%3,rand()%60);
				}
				//is it time to send a dll request?
				if(rack->m_next_dll_request<CTime::GetCurrentTime()){
					//TRACE("NetworkMonitor::NetworkSystem::Run() Broadcasting Dll Request to %s.\n",rack->m_ip.c_str());	
					m_com_system.Send(rack->m_ip.c_str(),RACKDLLS);
					rack->m_next_dll_request=CTime::GetCurrentTime()+CTimeSpan(0,0,10+rand()%3,rand()%60);
				}

				if(rack->mb_restart){
					m_com_system.Send(rack->m_ip.c_str(),RACKRESTARTCOMMAND);
					rack->mb_restart=false;
				}
			}
			rack_broad_cast_index++;
		}


		Sleep(50);
	}
	return 0;
}
void NetworkSystem::ScanInNetworks(void)
{
	TRACE("NetworkSystem::ScanInNetworks().\n");
	vector <string> v_networks;
	//mv_extended_file_names.clear();
	HANDLE file_handle = CreateFile((LPCSTR)"network.ini", // open file at local_path 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	DWORD hsize=0;
	DWORD data_length=0;
	data_length=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL || data_length<1){ //evil file check
		CloseHandle(file_handle);
		return;
	}
	byte *buffer=new byte[data_length+1];
	BOOL stat=ReadFile(file_handle,buffer,data_length,(DWORD*)&data_length,NULL);
	CloseHandle(file_handle);	

	buffer[data_length]=(char)0; //make sure its terminated with a null char so we don't crash the computer

	int last_index=0;
	for(int i=0;i<(int)data_length;i++){
		if(buffer[i]=='\n'){
			buffer[i]=(char)0;
			CString big_daddy=(char*)(buffer+last_index);
			last_index=i+1;
			big_daddy.Replace("\n","");
			big_daddy.Replace("\r","");
			big_daddy.Replace("\t"," ");
			big_daddy=big_daddy.Trim();
			CString tmp=big_daddy;
			tmp=tmp.MakeLower();
			int nw_index=-1;
			if(big_daddy.GetLength()>0 && (nw_index=tmp.Find("<network>"))!=-1){
				v_networks.push_back(string((LPCSTR)big_daddy.Mid(nw_index+(int)strlen("<network>"))));
			}
		}
	}

	delete []buffer;	
	
	//add any new ones
	for(int i=0;i<(int)v_networks.size();i++){
		CString network=v_networks[i].c_str();
		
		bool b_found=false;
		for(int j=0;j<(int)mv_networks.Size();j++){
			Network *nw=(Network*)mv_networks.Get(j);
			if(stricmp(nw->m_name.c_str(),network)==0){
				b_found=true;
				break;
			}
		}
		if(!b_found){
			TRACE("NetworkSystem::ScanInNetworks(). ADDED NETWORK %s\n",network);
			mv_networks.Add(new Network(network));
		}
	}
	
	//delete any that should no longer be there
	for(int j=0;j<(int)mv_networks.Size();j++){
		Network *nw=(Network*)mv_networks.Get(j);
		bool b_found=false;
		for(int i=0;i<(int)v_networks.size();i++){
			CString network=v_networks[i].c_str();
			if(stricmp(nw->m_name.c_str(),network)==0){
				b_found=true;
				break;
			}
		}
		if(!b_found){
			TRACE("NetworkSystem::ScanInNetworks(). DELETED NETWORK %s\n",nw->m_name.c_str());
			mv_networks.Remove(j);
			j--;
		}
	}
	TRACE("NetworkSystem::ScanInNetworks() Finished.\n");
}

void NetworkSystem::Init(void)
{
	ScanInNetworks();
	ScanInRacks();
	LoadHistory();
	m_com_system.StartThread();
	this->StartThread();
}

void NetworkSystem::ScanInRacks(void)
{
	TRACE("NetworkSystem::ScanInRacks().\n");
	NameServerInterface name_server;

	for(int i=0;i<(int)mv_racks.Size();i++){
		Rack* r=(Rack*)mv_racks.Get(i);
		r->mb_exist_check=false;
		r->mp_owner_network=NULL;
	}

	for(int i=0;i<(int)mv_networks.Size();i++){
		Network *nw=(Network*)mv_networks.Get(i);
		nw->mv_racks.Clear();  //clear this vector in this network, we'll fill it back up as we go
		vector <string> v_ips;
		CString tag=nw->m_name.c_str();
		name_server.RequestIP(tag,v_ips);
		
		//add any new ones
		for(int k=0;k<(int)v_ips.size();k++){
			bool b_found=false;
			for(int j=0;j<(int)mv_racks.Size();j++){
				Rack* r=(Rack*)mv_racks.Get(j);
				if(stricmp(r->m_ip.c_str(),v_ips[k].c_str())==0){
					b_found=true;
					nw->mv_racks.Add(r);
					r->mp_owner_network=nw;
					r->mb_exist_check=true;
					break;
				}
			}

			if(!b_found){  //we need to add this rack
				Rack *nr=new Rack(v_ips[k].c_str());
				nr->mp_owner_network=nw;
				mv_racks.Add(nr);
				nw->mv_racks.Add(nr);
			}
		}
	}

	for(int i=0;i<(int)mv_racks.Size();i++){
		Rack* r=(Rack*)mv_racks.Get(i);
		if(!r->mb_exist_check){
			mv_racks.Remove(i);  //the name server didn't report this ip, lets wipe it
			i--;
		}
	}

	mv_racks.Sort(1);
	TRACE("NetworkSystem::ScanInRacks() Finished.\n");
}

void NetworkSystem::EnumerateOnTime(int range,vector <bool> &vb_on)
{
	CSingleLock lock(&m_on_time_vector_lock,TRUE);
	CTime ref_time=CTime::GetCurrentTime();
	CTimeSpan one_hour(0,1,0,0);
	int response_offset=(int)mv_on_time.size()-1;
	//for each historical hour through range hours, determine if the trapper keeper was on or off
	for(int i=0;i<range;i++){
		bool b_on=false;
		for(int j=response_offset;j>=0;j--){
			//PingResponse *pong=(PingResponse*)mv_ping_responses.Get(j);
			if(mv_on_time[j]<=ref_time && mv_on_time[j]>(ref_time-one_hour)){
				b_on=true;
				break;
			}
			else if(mv_on_time[j]<(ref_time-one_hour)){
				break;  //aren't going to find one in the range we are looking for, since the response times are in chronological order
			}
			else if(mv_on_time[j]>ref_time){
				response_offset=j;  //decrease our response_offset, so we don't waste work in the future looking at parts of the vector we know are too high, since ref_time is decreasing
			}
		}
		vb_on.push_back(b_on);
		ref_time-=one_hour;
	}
}

void NetworkSystem::MarkOnTime(void)
{
	CSingleLock lock(&m_on_time_vector_lock,TRUE);
	mv_on_time.push_back(CTime::GetCurrentTime());
}

void NetworkSystem::SaveHistory(void)
{
	CreateDirectory("c:\\syncher",NULL);
	HANDLE file_handle = CreateFile((LPCSTR)"c:\\syncher\\status_history.dat", // open file at local_path 
                GENERIC_WRITE,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                CREATE_ALWAYS,             // create file always - CREATE_ALWAYS,OPEN_EXISTIN,..
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	DWORD hsize=0;
	DWORD data_length=0;
	//data_length=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL ){ //evil file check
		CloseHandle(file_handle);
		return;
	}

	int num_system_ons=(int)mv_on_time.size();
	DWORD tmp=0;
	WriteFile(file_handle,&num_system_ons,sizeof(int),&tmp,NULL);  //write out the length of our systeon om time vector

	for(int i=0;i<(int)mv_on_time.size();i++){
		WriteFile(file_handle,&mv_on_time[i],sizeof(CTime),&tmp,NULL);
	}

	int num_racks=(int)mv_racks.Size();
	WriteFile(file_handle,&num_racks,sizeof(int),&tmp,NULL);  //write out the length of our rack vector
	//begin writing out the history of each rack as we know it
	//for each rack...
	for(int i=0;i<(int)mv_racks.Size();i++){
		Rack* rack=(Rack*)mv_racks.Get(i);
		WriteFile(file_handle,rack->m_ip.c_str(),(DWORD)rack->m_ip.size()+1,&tmp,NULL);  //write out the ip
		Vector v_pings;
		rack->GetPingResponseVector(v_pings);
		int num_pings=(int)v_pings.Size();
		WriteFile(file_handle,&num_pings,sizeof(int),&tmp,NULL);   //write out the number of pings
		//for each ping response...
		for(int j=0;j<(int)v_pings.Size();j++){
			PingResponse* pr=(PingResponse*)v_pings.Get(j);
			WriteFile(file_handle,&pr->m_response_time,sizeof(CTime),&tmp,NULL);  //write time of ping response
			WriteFile(file_handle,&pr->mb_tk_on,1,&tmp,NULL); //write whether tk was on or off
		}
	}

	byte end_byte=254;
	WriteFile(file_handle,&end_byte,1,&tmp,NULL);

	CloseHandle(file_handle);
}

void NetworkSystem::LoadHistory(void)
{
	TRACE("NetworkSystem::LoadHistory().\n");
	//CreateDirectory("c:\\syncher",NULL);
	HANDLE file_handle = CreateFile((LPCSTR)"c:\\syncher\\status_history.dat", // open file at local_path 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // create file always - CREATE_ALWAYS,OPEN_EXISTIN,..
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	DWORD hsize=0;
	DWORD data_length=0;
	data_length=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL || data_length<1){ //evil file check
		CloseHandle(file_handle);
		return;
	}

	//advance to the end of the file to check last byte
	LONG high_move=0;
	LONG low_move=(UINT)(data_length-1);
	DWORD dwPtr=SetFilePointer(file_handle,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading
	DWORD error;
	if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
		error=GetLastError();
		ASSERT(0);
		CloseHandle(file_handle);
		return;
	}

	//check last byte to verify the last SaveHistory finished properly
	byte end_check=0;
	DWORD tmp=0;
	ReadFile(file_handle,&end_check,1,&tmp,NULL);
	if(end_check!=254){
		ASSERT(0);
		CloseHandle(file_handle);
		return;  //we won't be loading this corrupted history in
	}

	//advance back to the beginning of the file
	high_move=0;
	low_move=(UINT)(0);
	dwPtr=SetFilePointer(file_handle,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading

	if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
		error=GetLastError();
		ASSERT(0);
		CloseHandle(file_handle);
		return;
	}

	int num_system_ons=0;//(int)mv_on_time.size();
	ReadFile(file_handle,&num_system_ons,sizeof(int),&tmp,NULL);  //read in how many system on times there are
	
	///WriteFile(file_handle,&num_system_ons,sizeof(int),&tmp,NULL);  //write out the length of our systeon om time vector
	//for each system on time in the file...
	for(int i=0;i<(int)num_system_ons;i++){
		CTime tmp_time;
		ReadFile(file_handle,&tmp_time,sizeof(CTime),&tmp,NULL);
		mv_on_time.push_back(tmp_time);
	}

	int num_racks=0;
	ReadFile(file_handle,&num_racks,sizeof(int),&tmp,NULL);  //read in how many system on times there are
	
	//begin writing out the history of each rack as we know it
	//for each rack...
	for(int i=0;i<(int)num_racks;i++){
		string ip;
		char ch=0;
		//read in the ip
		while(ReadFile(file_handle,&ch,1,&tmp,NULL) && ch!=(char)0){
			ip+=ch;
		}
		Rack* rack=GetRack(ip.c_str());

		int num_pings=0;//(int)rack->mv_ping_responses.Size();
		ReadFile(file_handle,&num_pings,sizeof(int),&tmp,NULL);   //read in the number of pings
		//for each ping response...
		for(int j=0;j<(int)num_pings;j++){
			CTime tmp_time;
			bool b_tk_on=false;
			ReadFile(file_handle,&tmp_time,sizeof(CTime),&tmp,NULL);
			ReadFile(file_handle,&b_tk_on,1,&tmp,NULL);		
			if(rack){
				PingResponse *pr=new PingResponse(b_tk_on);
				pr->m_response_time=tmp_time;
				rack->SavePingResponse(pr);
			}
		}
	}

	CloseHandle(file_handle);
}

Rack* NetworkSystem::GetRack(const char* ip)
{
	int low=0;
	int high=max(0,mv_racks.Size()-1);
	int index=0;
	//do a binary search to determine where we should insert this file into the array
	for(;low<high;){
		index=low+((high-low)>>1);
		Rack* rack=(Rack*)mv_racks.Get(index);
		int result=stricmp(ip,rack->m_ip.c_str());
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

	if(mv_racks.Size()>0){
		Rack* rack=(Rack*)mv_racks.Get(index);
		if(stricmp(ip,rack->m_ip.c_str())==0){
			return rack;
		}

		//this inefficiency is imposed on us by the premise that we can theoretically have duplicate hashes (files of different sizes actually having the same hash)
		for(int i=low;i<=high;i++){
			Rack* rack=(Rack*)mv_racks.Get(i);
			if(stricmp(ip,rack->m_ip.c_str())==0){
				return rack;
			}
		}
	}

	return NULL;
}

void NetworkSystem::OnReceive(void)
{
//	#define RACKPING 5556
//	#define RACKDLLS 5558
//	#define RACKPROCESSES 5559

	ReceivedMessage *msg=m_com_system.GetNextReceivedMessage();

	UINT header=msg->GetHeader();
	Buffer2000 *data=msg->GetData();
	if(header==RACKPING){
		//TRACE("NetworkMonitor::NetworkSystem::OnReceive() Received PING RESPONSE from %s.\n",msg->GetSource());
		if(data->GetLength()<4)
			return;
		int ping_version=data->GetByte(0);
		int version=data->GetWord(1);
		DWORD status=data->GetByte(3);
		
		bool b_trapper_on=false;
		if(status&0x01)
			b_trapper_on=true;
		
		//we have a ping with trapper on/off status
		Rack *rack=GetRack(msg->GetSource());

		if(rack){
			rack->m_version=version;
			rack->GotPingResponse(b_trapper_on);
		}
	}
	else if(header==RACKDLLS){
		//TRACE("NetworkMonitor::NetworkSystem::OnReceive() Received RACK DLL RESPONSE from %s.\n",msg->GetSource());
		if(data->GetLength()<1)
			return;

		Rack *rack=GetRack(msg->GetSource());
		if(!rack)
			return;

		int num_dlls=data->GetByte(0);

		const byte* buf=data->GetBufferPtr();
		buf++;
		int offset=0;

		int data_length=(int)data->GetLength();
		vector <string> v_strings;
		string cur_dll;
		while(offset<(int)data->GetLength() && (int)v_strings.size()<num_dlls){
			if(buf[offset]==(char)0){
				v_strings.push_back(cur_dll);
				cur_dll="";
			}
			else
				cur_dll+=buf[offset]; //safely iterate through, incase of corruption or no telling what else.  As opposed to just setting a string equal to the buffer which would be the most efficient
			offset++;
		}

		Vector v_dlls;
		

		for(int i=0;i<(int)v_strings.size();i++){
			bool b_found=false;
			for(int j=0;j<(int)mv_dlls.Size();j++){
				DLLObject *dll=(DLLObject*)mv_dlls.Get(j);
				if(stricmp(dll->m_name.c_str(),v_strings[i].c_str())==0){
					b_found=true;
					dll->Renew();
					v_dlls.Add(dll);
					break;
				}
			}
			
			if(!b_found){
				DLLObject *new_dll=new DLLObject(v_strings[i].c_str());
				mv_dlls.Add(new_dll);
				v_dlls.Add(new_dll);
				mb_dlls_changed=true;
				mv_dlls.Sort(1);
			}
		}

		if(rack->SetDLLs(v_dlls)){
			mb_dlls_changed=true;
		}
	}
	else if(header==RACKPROCESSES){
		//TRACE("NetworkMonitor::NetworkSystem::OnReceive() Received RACK PROCESS RESPONSE from %s.\n",msg->GetSource());
		if(data->GetLength()<1)
			return;

		Rack *rack=GetRack(msg->GetSource());
		if(!rack)
			return;

		int num_processes=data->GetByte(0);

		const byte* buf=data->GetBufferPtr();
		buf++;
		int offset=0;

		int data_length=(int)data->GetLength();
		vector <string> v_strings;
		string cur_process;
		while(offset<(int)data->GetLength() && (int)v_strings.size()<num_processes){
			if(buf[offset]==(char)0){
				v_strings.push_back(cur_process);
				cur_process="";
			}
			else
				cur_process+=buf[offset];  //safely iterate through, incase of corruption or no telling what else.  As opposed to just setting a string equal to the buffer which would be the most efficient
			offset++;
		}

		

		for(int i=0;i<(int)v_strings.size();i++){
			bool b_found=false;
			for(int j=0;j<(int)mv_processes.Size();j++){
				ProcessObject *process=(ProcessObject*)mv_processes.Get(j);
				if(stricmp(process->m_name.c_str(),v_strings[i].c_str())==0){
					b_found=true;
					if(process->AddRack(rack->m_ip.c_str())){
						mb_processes_changed=true;
					}
					break;
				}
			}
			
			if(!b_found){
				ProcessObject *new_process=new ProcessObject(v_strings[i].c_str());
				new_process->AddRack(rack->m_ip.c_str());
				mv_processes.Add(new_process);
				mb_processes_changed=true;
				mv_processes.Sort(1);
			}
		}
	}

	m_com_system.PopMessages();
}

void NetworkSystem::FillTreeWithProcessInfo(CTreeCtrl& tree)
{
	if(!mb_processes_changed){
		return;
	}
	mb_processes_changed=false;

	TRACE("NetworkMonitor::NetworkSystem::FillTreeWithProcessInfo() Refilling process tree.\n");
	tree.DeleteAllItems();  //clear the tree

	//go through each of the processes
	for(int i=0;i<(int)mv_processes.Size();i++){
		ProcessObject *process=(ProcessObject*)mv_processes.Get(i);
		CString tmp;

		//set process name
		tmp.Format("%s   (%d racks)",process->m_name.c_str(),process->mv_racks.Size());
		HTREEITEM parent_node1=tree.InsertItem(tmp,0,0);

		//add the racks by network for each process
		vector <string> vs_networks;
		vector <HTREEITEM> vt_networks;
		for(int j=0;j<(int)process->mv_racks.Size();j++){
			ProcessObject::RackProcessHolder *rph=(ProcessObject::RackProcessHolder *)process->mv_racks.Get(j);
			Rack *rack=GetRack(rph->m_ip.c_str());

			HTREEITEM target_network;
			//figure out which network this is supposed to go on

			//see if we have already added this network to our tree under this process
			bool b_found=false;
			for(int m=0;m<(int)vs_networks.size();m++){
				if(stricmp(rack->mp_owner_network->m_name.c_str(),vs_networks[m].c_str())==0){
					b_found=true;
					target_network=vt_networks[m];
					break;
				}
			}
			if(!b_found){
				target_network=tree.InsertItem(rack->mp_owner_network->m_name.c_str(),0,0,parent_node1);
				vt_networks.push_back(target_network);
				vs_networks.push_back(rack->mp_owner_network->m_name);
			}

			//add the racks ip under its network name in the tree
			tree.InsertItem(rph->m_ip.c_str(),0,0,target_network);

		}

	}

	tree.Invalidate(TRUE);
}

void NetworkSystem::FillTreeWithDllInfo(CTreeCtrl& tree)
{
	if(!mb_dlls_changed){
		return;
	}
	mb_dlls_changed=false;

	TRACE("NetworkMonitor::NetworkSystem::FillTreeWithDllInfo() Refilling dll tree.\n");
	tree.DeleteAllItems();  //clear the tree

	//go through each of the dlls
	for(int i=0;i<(int)mv_dlls.Size();i++){
		DLLObject *dll=(DLLObject*)mv_dlls.Get(i);
		CString tmp;

		//count up the number of racks that have this dll
		Vector v_racks;
		for(int j=0;j<(int)mv_racks.Size();j++){
			Rack* rack=(Rack*)mv_racks.Get(j);
			if(rack->HasDll(dll->m_name.c_str()))
				v_racks.Add(rack);
		}

		//set process name
		tmp.Format("%s   (%d racks)",dll->m_name.c_str(),v_racks.Size());
		HTREEITEM parent_node1=tree.InsertItem(tmp,0,0);

		//add the racks by network for each process
		vector <string> vs_networks;
		vector <HTREEITEM> vt_networks;
		for(int j=0;j<(int)v_racks.Size();j++){			
			Rack *rack=(Rack*)v_racks.Get(j);

			HTREEITEM target_network;
			//figure out which network this is supposed to go on

			//see if we have already added this network to our tree under this process
			bool b_found=false;
			for(int m=0;m<(int)vs_networks.size();m++){
				if(stricmp(rack->mp_owner_network->m_name.c_str(),vs_networks[m].c_str())==0){
					b_found=true;
					target_network=vt_networks[m];
					break;
				}
			}
			if(!b_found){
				target_network=tree.InsertItem(rack->mp_owner_network->m_name.c_str(),0,0,parent_node1);
				vt_networks.push_back(target_network);
				vs_networks.push_back(rack->mp_owner_network->m_name);
			}

			//add the racks ip under its network name in the tree
			tree.InsertItem(rack->m_ip.c_str(),0,0,target_network);
		}

	}

	tree.Invalidate(TRUE);
}

Network* NetworkSystem::GetNetwork(const char* name)
{
	for(int j=0;j<(int)mv_networks.Size();j++){
		Network* n=(Network*)mv_networks.Get(j);
		if(stricmp(name,n->m_name.c_str())==0)
			return n;
	}
	return NULL;
}

void NetworkSystem::RemoteControlRack(const char* ip)
{
	char command[256];
	sprintf(command, "mstsc /v:%s /f", ip);
	PROCESS_INFORMATION pi;
	STARTUPINFOA si = {sizeof si};
	int bret = CreateProcess(NULL,command,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
	if(bret != 0)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
}

void NetworkSystem::OnRestartDeadRacks(void)
{
	for(int i=0;i<(int)mv_racks.Size();i++){
		Rack *rack=(Rack*)mv_racks.Get(i);
		if(!rack->IsAlive() || !rack->IsTKOn())
			rack->mb_restart=true;
	}
}

void NetworkSystem::OnRestartSelectedNetwork(Network* network)
{
	if(!network)
		return;

	for(int i=0;i<(int)network->mv_racks.Size();i++){
		Rack *rack=(Rack*)network->mv_racks.Get(i);
		rack->mb_restart=true;
	}
}

void NetworkSystem::OnRemoteCrashedSelectedNetwork(Network* network)
{
	if(!network)
		return;

	for(int i=0;i<(int)network->mv_racks.Size();i++){
		Rack *rack=(Rack*)network->mv_racks.Get(i);
		if(rack->IsAlive() && !rack->IsTKOn())
			RemoteControlRack(rack->m_ip.c_str());
	}
}

void NetworkSystem::OnRemoteDeadSelectedNetwork(Network* network)
{
	if(!network)
		return;

	for(int i=0;i<(int)network->mv_racks.Size();i++){
		Rack *rack=(Rack*)network->mv_racks.Get(i);
		if(!rack->IsAlive())
			RemoteControlRack(rack->m_ip.c_str());
	}
}

void NetworkSystem::OnRestartDeadSelectedNetwork(Network* network)
{
	if(!network)
		return;

	for(int i=0;i<(int)network->mv_racks.Size();i++){
		Rack *rack=(Rack*)network->mv_racks.Get(i);
		if(!rack->IsAlive())
			rack->mb_restart=true;
	}
}

void NetworkSystem::OnRestartCrashedSelectedNetwork(Network* network)
{
	if(!network)
		return;

	for(int i=0;i<(int)network->mv_racks.Size();i++){
		Rack *rack=(Rack*)network->mv_racks.Get(i);
		if(rack->IsAlive() && !rack->IsTKOn())
			rack->mb_restart=true;
	}
}

void NetworkSystem::ExportDeadRacks(void)
{
	HANDLE file_handle = CreateFile((LPCSTR)"dead_racks.txt", // open file at local_path 
                GENERIC_WRITE,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                CREATE_ALWAYS,             // create file always - CREATE_ALWAYS,OPEN_EXISTIN,..
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	DWORD hsize=0;
	DWORD data_length=0;
	//data_length=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL ){ //evil file check
		CloseHandle(file_handle);
		return;
	}
	
	DWORD tmp=0;
	WriteToFile(file_handle,"Dead Racks By Network\r\n\r\n");
	
	for(int i=0;i<(int)mv_networks.Size();i++){
		Network* nw=(Network*)mv_networks.Get(i);

		CString tmp_output;
		tmp_output+="======= ";
		tmp_output+=nw->m_name.c_str();
		tmp_output+=" =======";
		tmp_output+="\r\n";
		int dead_count=0;
		for(int j=0;j<(int)nw->mv_racks.Size();j++){
			Rack* rack=(Rack*)nw->mv_racks.Get(j);
			if(!rack->IsAlive()){
				tmp_output+="-- ";
				tmp_output+=rack->m_ip.c_str();
				tmp_output+="\r\n";
				dead_count++;
			}
			
		}
		

		if(dead_count>0){
			WriteToFile(file_handle,tmp_output);
			WriteToFile(file_handle,"\r\n\r\n");
		}
	}

	CloseHandle(file_handle);
}

void NetworkSystem::WriteToFile(HANDLE file, const char* str)
{
	DWORD tmp=0;
	WriteFile(file,str,(DWORD)strlen(str),&tmp,NULL);
}

void NetworkSystem::ExportCrashedRacks(void)
{
	HANDLE file_handle = CreateFile((LPCSTR)"crashed_racks.txt", // open file at local_path 
                GENERIC_WRITE,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                CREATE_ALWAYS,             // create file always - CREATE_ALWAYS,OPEN_EXISTIN,..
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	DWORD hsize=0;
	DWORD data_length=0;
	//data_length=GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL ){ //evil file check
		CloseHandle(file_handle);
		return;
	}
	
	DWORD tmp=0;
	WriteToFile(file_handle,"Crashed Racks By Network\r\n\r\n");
	
	for(int i=0;i<(int)mv_networks.Size();i++){
		Network* nw=(Network*)mv_networks.Get(i);

		CString tmp_output;
		tmp_output+="======= ";
		tmp_output+=nw->m_name.c_str();
		tmp_output+=" =======";
		tmp_output+="\r\n";
		int dead_count=0;
		for(int j=0;j<(int)nw->mv_racks.Size();j++){
			Rack* rack=(Rack*)nw->mv_racks.Get(j);
			if(rack->IsAlive() && !rack->IsTKOn()){
				tmp_output+="-- ";
				tmp_output+=rack->m_ip.c_str();
				tmp_output+="\r\n";
				dead_count++;
			}
			
		}
		

		if(dead_count>0){
			WriteToFile(file_handle,tmp_output);
			WriteToFile(file_handle,"\r\n\r\n");
		}
	}

	CloseHandle(file_handle);
}
