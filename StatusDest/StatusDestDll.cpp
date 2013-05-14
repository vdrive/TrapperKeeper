#include "StdAfx.h"
#include "statusdestdll.h"
#include "../NameServer/NameServerInterface.h"
#include "DllInterface.h"
#include "RackRecoverInterface.h"


StatusDestDll::StatusDestDll(void)
{
}

//
//
//
StatusDestDll::~StatusDestDll(void)
{
}

UINT StatusDestThreadProc(LPVOID pParam)
{
//	UINT i;

	// Init message window handle
	//HWND hwnd=(HWND)pParam;


	// Init the message data structure and send it
//	CCriticalSection* critical_section = NULL;
	StatusDestThreadData* thread_data = (StatusDestThreadData*)pParam;
	
/*	// Create the events
	HANDLE events[StatusDestThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data->m_events[i]=events[i];
	}

	PostMessage(thread_data->m_hwnd,WM_INIT_STATUS_DEST_THREAD_DATA,0,(LPARAM)thread_data);	// the thread data is ready

	// Start the thread
	DWORD num_events=2;
	BOOL wait_all=FALSE;
	DWORD timeout=INFINITE;
	DWORD event;	// which event fired

	while(1)
	{
		event=WaitForMultipleObjects(num_events,events,wait_all,timeout);

		// Check to see if this is the kill thread events (event 0)
		if(event==0)
		{
			ResetEvent(events[event]);
			break;
		}

		// There is new data to be written to database
		if(event==1)
		{
			//CSingleLock singleLock(critical_section);
			//singleLock.Lock();
			//if(singleLock.IsLocked())
			//{
*/				unsigned int i,j;
				UINT host_index = 0;

				// See if there is already an item for this IP
				bool found=false;
				for(i=0;i<(unsigned int)thread_data->pv_hosts->size();i++)
				{
					if(strcmp((*thread_data->pv_hosts)[i].m_ip.c_str(), thread_data->m_from)==0)
					{
						found=true;
						host_index = i;
						(*thread_data->pv_hosts)[i].UpdateAvgNetworkBandwidth(thread_data->m_status.m_total_used_bandwidth);
						(*thread_data->pv_hosts)[i].UpdateAvgCpuUsage(thread_data->m_status.m_percent_processor_usage);
						(*thread_data->pv_hosts)[i].UpdateAvgUploadBandwidth(thread_data->m_status.m_sent_per_sec);
						(*thread_data->pv_hosts)[i].UpdateAvgDownloadBandwidth(thread_data->m_status.m_received_per_sec);
						break;
					}
				}
				if(!found)
				{
					delete thread_data;
					return 0; //ignore it
				}
				string network = (*thread_data->pv_hosts)[host_index].m_network;
				// Find this ip's network in the network bandwidths
				for(i=0;i<thread_data->pv_network_bandwidths->size();i++)
				{
					if(strcmp((*thread_data->pv_network_bandwidths)[i]->m_network.c_str(), network.c_str())==0)
					{
						//update individual network dialog status
						(*thread_data->pv_network_bandwidths)[i]->m_dlg.UpdateStatus(thread_data->m_from, &thread_data->m_status, 
							(*thread_data->pv_hosts)[host_index].GetAverageUsedNetworkBandwidth());
						for(j=0;j< (*thread_data->pv_network_bandwidths)[i]->v_hosts.size();j++)
						{
							if(strcmp( (*thread_data->pv_network_bandwidths)[i]->v_hosts[j].m_ip.c_str(),thread_data->m_from)==0)
							{
								//set the latest status reply time to "now"
								(*thread_data->pv_network_bandwidths)[i]->v_hosts[j].m_last_reply_time = CTime::GetCurrentTime(); 

								//get the average bandwidth
								(*thread_data->pv_network_bandwidths)[i]->v_hosts[j].m_bandwidth
									=(unsigned int)( (*thread_data->pv_hosts)[host_index].GetAverageUsedNetworkBandwidth()*1024*1024 );

								//get the average upload bandwidth
								(*thread_data->pv_network_bandwidths)[i]->v_hosts[j].m_upload_bandwidth
									=(unsigned int)( (*thread_data->pv_hosts)[host_index].GetAverageUploadBandwidth()*1024*1024 );

								//get the average download bandwidth
								(*thread_data->pv_network_bandwidths)[i]->v_hosts[j].m_download_bandwidth
									=(unsigned int)( (*thread_data->pv_hosts)[host_index].GetAverageDownloadBandwidth()*1024*1024 );

								//get the number of files shared
								(*thread_data->pv_network_bandwidths)[i]->v_hosts[j].m_files_shared = thread_data->m_status.m_shared_file_count;

								//get the average cpu usage
								(*thread_data->pv_network_bandwidths)[i]->v_hosts[j].m_cpu_usage 
									=(unsigned int)( (*thread_data->pv_hosts)[host_index].GetAverageCpuUsage());

								//Update the main Network summary dialog
								//PostMessage(thread_data->m_hwnd,WM_UPDATE_NETWORK_SUMMARY,(WPARAM)(*thread_data->pv_network_bandwidths)[i]->m_network.c_str(),0);
								break;	// out of for(j)
							}
						}
						break; //out of for(i)
					}
				}
/*
				//singleLock.Unlock();
				break;
			//}
		}
	}


	// Close the events
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		CloseHandle(events[i]);
	}
*/	
	delete thread_data;
	return 0;	// exit the thread
}

//
//
//
void StatusDestDll::DllInitialize()
{
	m_dlg.Create(IDD_STATUS_DEST_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.MoveWindow(0,0,1050,1020);
	m_dlg.InitParent(this);
	m_interval = 20; //20 sec default
	m_second_count = 0;
}

//
//
//
void StatusDestDll::DllUnInitialize()
{
	if(v_network_bandwidths.size() > 0)
	{
		vector<NetworkBandwidth*>::iterator iter = v_network_bandwidths.begin();
		while(iter != v_network_bandwidths.end())
		{
			delete (*iter);
			iter++;
		}
	}
}

//
//
//
void StatusDestDll::DllStart()
{
	//register com service
	m_com.Register(this,DllInterface::m_app_id.m_app_id);
	LoadINIFile();
	//m_dlg.SetTimer(1,m_interval*1000,NULL);	//request status every 5 sec
	m_dlg.SetTimer(2,60*1000,NULL); //rescan network.ini every min
	m_dlg.SetTimer(3,1000,NULL);	//1 sec timer
	m_dlg.SetTimer(4,5*1000,NULL); //5 sec timer
	m_dlg.SetTimer(5,5*60*1000,NULL); //send system info request every 5 min
	//m_dlg.SetTimer(6,30*1000,NULL); //update summary every 30 sec
}

//
//
//
void StatusDestDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void StatusDestDll::DataReceived(char *source_name, void *data, int data_length)
{
	byte* buf = (byte*)data;
	//received remote data from the Interface
	StatusHeader* header = (StatusHeader*)buf;
	
	switch(header->op)
	{
		case (StatusHeader::Status_Reply):
		{
			StatusData status;
			status.ReadFromBuffer((char*)&buf[sizeof(StatusHeader)]);
			/*
			vector<UINT> kazaa_mems;
			
			unsigned int *ptr=(unsigned int *)&buf[sizeof(StatusHeader)+sizeof(StatusData)];
			for(UINT i=0;i<status->m_kazaa_count;i++)
			{
				kazaa_mems.push_back(*ptr);
				ptr++;
			}
			*/
			ReceivedStatus(source_name, &status);
			break;
		}
		case (StatusHeader::System_Info_Reply):
		{
			SystemInfoData* sys_info = (SystemInfoData*)&buf[sizeof(StatusHeader)];

			if(data_length < sizeof(StatusHeader)+sizeof(SystemInfoData))
			{
				SystemInfoData* new_sys_info = new SystemInfoData();
				new_sys_info->m_total_physical_memory = sys_info->m_total_physical_memory;
				new_sys_info->m_total_harddisk_space = sys_info->m_total_harddisk_space;
				strcpy(new_sys_info->m_cpu_info,sys_info->m_cpu_info);
				new_sys_info->m_os_info = sys_info->m_os_info;
				new_sys_info->b_OsVersionInfoEx = sys_info->b_OsVersionInfoEx;
				ReceivedSystemInfoReply(source_name,new_sys_info);
				delete new_sys_info;
			}
			else
			{
				ReceivedSystemInfoReply(source_name, sys_info);
			}
			
			break;
		}
		case (StatusHeader::Num_Kazaa_Uploads):
		{
			int uploads = (*(int*)&buf[sizeof(StatusHeader)]);
			ReceivedKazaaUploads(source_name, uploads);
			break;
		}
		case (StatusHeader::Shared_Files_Reply):
		{
			FileInfos reply;
			reply.ReadFromBuffer((char*)&buf[sizeof(StatusHeader)]);
			ReceivedSharedFilesReply(source_name, reply);
			break;
		}
	}
}

//
//
//
void StatusDestDll::SendStatusRequest(const char* dest)
{
	int data_len = sizeof(StatusHeader)+sizeof(UINT);
	byte* buf = new byte[data_len];
	StatusHeader* header = (StatusHeader*)buf;
	header->op = StatusHeader::Status_Request;
	header->size = data_len - sizeof(StatusHeader);

	UINT* version = (UINT*)&buf[sizeof(StatusHeader)];
	*version = DllInterface::m_app_id.m_version;

	m_com.SendUnreliableData((char*)dest,buf,data_len);
	
	delete buf;
}

//
//
//
void StatusDestDll::OnTimer(UINT nIDEvent)
{
	//testing only
	switch(nIDEvent)
	{
		/*
		case 1:
		{
			for(UINT i=0;i<v_network_bandwidths.size();i++)
			{
				SendStatusRequest(v_network_bandwidths[i]->m_network.c_str());
			}
			break;
		}
		*/
		case 2:
		{
			LoadINIFile();
			break;
		}
		case 3: //every sec
		{
			int offset = m_second_count % m_interval;
			bool did_send = false;
			for(UINT i=0;i<v_hosts.size();i++)
			{
				if(v_hosts[i].m_retreve_status_offset == offset)
				{
					did_send = true;
					SendStatusRequest(v_hosts[i].m_ip.c_str());
				}
				else if(did_send)
					break;
			}
			m_second_count++;
			break;
		}
		case 4:
		{
			UpdateNetworkStatus();
			break;
		}
		case 5:
		{
			for(UINT i=0; i<v_hosts.size(); i++)
			{
				SendGenericMessage(v_hosts[i].m_ip.c_str(),StatusHeader::System_Info_Request);
			}
			break;
		}
		/*
		case 6:
		{
			m_dlg.UpdateSummary();
			break;
		}
		*/
	}
}

//
//
//
bool StatusDestDll::LoadINIFile()
{
	bool file_changed = false;
	unsigned int i;

	CStdioFile file;
	if(file.Open("network.ini",CFile::typeText|CFile::modeRead|CFile::shareDenyWrite)==FALSE)
	{
		MessageBox(NULL,"Could not open network.ini","StatusDest: Error",MB_OK|MB_ICONERROR);
		return false;
	}

	CString cstring;
	unsigned int interval;
	char buf[4096];
	string network;
	while(file.ReadString(cstring))
	{	
		cstring.TrimLeft();
		cstring.TrimRight();
		strcpy(buf,cstring);
		if(strstr(buf,"<interval>")==buf)
		{
			if(sscanf(&buf[strlen("<interval>")],"%u",&interval)==1)
			{
				m_interval=interval;
			}
		}
		else if(strstr(buf,"<network>")==buf)
		{
			char tmp[1024];
			if(sscanf(&buf[strlen("<network>")],"%s",tmp)==1)
			{
				cstring=tmp;
				cstring.TrimLeft();
				cstring.TrimRight();
				network=cstring;

				// See if this network is already in the network vector
				bool found=false;
				for(i=0;i<v_network_bandwidths.size();i++)
				{
					if(strcmp((char *)v_network_bandwidths[i]->m_network.c_str(),(char *)network.c_str())==0)
					{
						found=true;
						break;
					}
				}
				// If not found, add a new one
				if(!found)	
				{
					NetworkBandwidth* nb = new NetworkBandwidth(this);
					nb->m_network = network;
					v_network_bandwidths.push_back(nb);
					m_dlg.AddNetwork(network); //add the new network to the dialog
				}

				vector<NetworkBandwidth*>::iterator iter = v_network_bandwidths.begin();
				while(iter != v_network_bandwidths.end())
				{
					if(strcmp((*iter)->m_network.c_str(), network.c_str())== 0)
						break;
					iter++;
				}
				
				//request ips from nameserver
				NameServerInterface nsi;
				vector<string> IPs;
				nsi.RequestIP(network.c_str(), IPs);
				if(IPs.size() > 0 )
				{
					CString ip_range = IPs[0].c_str();
					ip_range += " - ";
					ip_range += IPs[(int)IPs.size()-1].c_str();
					m_dlg.AddIPRange(network.c_str(), ip_range);
				}
				
				if(!found) //if we were adding a new network
				{
					for(UINT i=0;i<IPs.size();i++)
					{
						SourceHost host;
						host.m_network = network;
						host.m_ip = IPs[i];
						v_hosts.push_back(host);
						file_changed = true;
					}
				}

				if((*iter)->v_hosts.size() != IPs.size())//check if we have new IPs for this network
				{
					(*iter)->v_hosts.clear();
					vector<NetworkBandwidthHost> hosts;
					for(UINT i=0;i<IPs.size();i++)
					{
						NetworkBandwidthHost host;
						host.m_ip = IPs[i];
						hosts.push_back(host);
						//see if this IP is in v_hosts already, if not, add it
						bool found_ip = false;
						for(UINT j=0; j<v_hosts.size(); j++)
						{
							if(strcmp(v_hosts[j].m_ip.c_str(), IPs[i].c_str())==0)
							{
								found_ip = true;
								break;
							}
						}
						if(!found_ip)
						{
							SourceHost host;
							host.m_network = network;
							host.m_ip = IPs[i];
							v_hosts.push_back(host);
							file_changed = true;
						}
					}
					(*iter)->v_hosts = hosts;
					//Update Network dialog IP list
					(*iter)->m_dlg.UpdateIPList(IPs);
				}

			}
		}
	}

	file.Close();
	if(file_changed)
		CalculatePingingInterval();
	return true;
}

//
//
//
void StatusDestDll::OpenNetworkDialog(CString network)
{
	for(UINT i=0; i<v_network_bandwidths.size(); i++)
	{
		if(strcmp(v_network_bandwidths[i]->m_network.c_str(), network)==0)
			v_network_bandwidths[i]->ShowNetworkGUI();
	}
}

//
//
//
void StatusDestDll::ReceivedStatus(char* from, StatusData* status)
{
	StatusDestThreadData* thread_data = new StatusDestThreadData();
	thread_data->m_from = from;
	thread_data->m_hwnd = m_dlg.GetSafeHwnd();
	thread_data->m_status = status;
	thread_data->pv_hosts = &v_hosts;
	thread_data->pv_network_bandwidths = &v_network_bandwidths;
	//thread_data->v_kazaa_mems = kazaa_mems;

	AfxBeginThread(StatusDestThreadProc,(LPVOID)thread_data,THREAD_PRIORITY_LOWEST);
/*
	unsigned int i,j;
	UINT host_index = 0;

	// See if there is already an item for this IP
	bool found=false;
	for(i=0;i<(unsigned int)v_hosts.size();i++)
	{
		if(strcmp(v_hosts[i].m_ip.c_str(),from)==0)
		{
			found=true;
			host_index = i;
			v_hosts[i].UpdateAvgNetworkBandwidth(status->m_total_used_bandwidth);
			v_hosts[i].UpdateAvgCpuUsage(status->m_percent_processor_usage);
			break;
		}
	}
	if(!found)
	{
		return; //ignore it
	}
	string network = v_hosts[host_index].m_network;
	// Find this ip's network in the network bandwidths
	for(i=0;i<v_network_bandwidths.size();i++)
	{
		if(strcmp(v_network_bandwidths[i]->m_network.c_str(), network.c_str())==0)
		{
			//update individual network dialog status
			v_network_bandwidths[i]->m_dlg.UpdateStatus(from, status, 
				v_hosts[host_index].GetAverageUsedNetworkBandwidth(), kazaa_mems); 
			for(j=0;j<v_network_bandwidths[i]->v_hosts.size();j++)
			{
				if(strcmp(v_network_bandwidths[i]->v_hosts[j].m_ip.c_str(),from)==0)
				{
					//set the latest status reply time to "now"
					v_network_bandwidths[i]->v_hosts[j].m_last_reply_time = CTime::GetCurrentTime(); 
					 
					//get the average bandwidth
					v_network_bandwidths[i]->v_hosts[j].m_bandwidth
						=(unsigned int)( v_hosts[host_index].GetAverageUsedNetworkBandwidth()*1024*1024 );
					
					//get the average cpu usage
					v_network_bandwidths[i]->v_hosts[j].m_cpu_usage 
						=(unsigned int)( v_hosts[host_index].GetAverageCpuUsage());

					//Update the main Network summary dialog
					UpdateNetworkStatus(v_network_bandwidths[i]->m_network);
					break;	// out of for(j)
				}
			}
			break; //out of for(i)
		}
	}
*/
}

//
//
//
void StatusDestDll::UpdateNetworkStatus(string& network)
{
	unsigned int i,j;
	
	// Find this network in the list
	unsigned int bandwidth=0;
	unsigned int upload_bandwidth=0;
	unsigned int download_bandwidth=0;
	UINT files_shared=0;
	UINT number_of_hosts =0;
	UINT number_of_hosts_up = 0;
	UINT cpu_usage = 0;

	for(i=0;i<v_network_bandwidths.size();i++)
	{
		if(strcmp(v_network_bandwidths[i]->m_network.c_str(),network.c_str())==0)
		{
			number_of_hosts = (UINT)v_network_bandwidths[i]->v_hosts.size();
			// Calculate the bandwidth total in Mb/s
			for(j=0;j<number_of_hosts;j++)
			{
				bandwidth+=v_network_bandwidths[i]->v_hosts[j].m_bandwidth;
				upload_bandwidth+=v_network_bandwidths[i]->v_hosts[j].m_upload_bandwidth;
				download_bandwidth+=v_network_bandwidths[i]->v_hosts[j].m_download_bandwidth;
				files_shared+=v_network_bandwidths[i]->v_hosts[j].m_files_shared;
				cpu_usage+=v_network_bandwidths[i]->v_hosts[j].m_cpu_usage;
				if(v_network_bandwidths[i]->v_hosts[j].IsUp())
					number_of_hosts_up++;
			}
			break;
		}
	}
	if(number_of_hosts_up!=0)
	{
		cpu_usage /= number_of_hosts_up;
		files_shared /= number_of_hosts_up;
	
		m_dlg.UpdateStatus(network, number_of_hosts, number_of_hosts_up,bandwidth, cpu_usage,
						upload_bandwidth, download_bandwidth, files_shared); //update the dialog
	}
}

//
//
//
void StatusDestDll::UpdateNetworkStatus()
{
	unsigned int i,j;

	for(i=0;i<v_network_bandwidths.size();i++)
	{
		// Find this network in the list
		unsigned int bandwidth=0;
		unsigned int upload_bandwidth=0;
		unsigned int download_bandwidth=0;
		UINT files_shared=0;
		UINT number_of_hosts =0;
		UINT number_of_hosts_up = 0;
		UINT cpu_usage = 0;

		number_of_hosts = (UINT)v_network_bandwidths[i]->v_hosts.size();
		// Calculate the bandwidth total in Mb/s
		for(j=0;j<number_of_hosts;j++)
		{
			bandwidth+=v_network_bandwidths[i]->v_hosts[j].m_bandwidth;
			upload_bandwidth+=v_network_bandwidths[i]->v_hosts[j].m_upload_bandwidth;
			download_bandwidth+=v_network_bandwidths[i]->v_hosts[j].m_download_bandwidth;
			files_shared+=v_network_bandwidths[i]->v_hosts[j].m_files_shared;
			cpu_usage+=v_network_bandwidths[i]->v_hosts[j].m_cpu_usage;
			if(v_network_bandwidths[i]->v_hosts[j].IsUp())
				number_of_hosts_up++;
		}
		if(number_of_hosts_up!=0)
		{
			cpu_usage /= number_of_hosts_up;
			files_shared /= number_of_hosts_up;

			m_dlg.UpdateStatus(v_network_bandwidths[i]->m_network.c_str(), number_of_hosts, number_of_hosts_up,
				bandwidth, cpu_usage, upload_bandwidth, download_bandwidth, files_shared); //update the dialog
		}
	}
}

//
//
//
bool StatusDestDll::IsComputerUp(string network, string ip)
{
	for(UINT i=0;i<v_network_bandwidths.size();i++)
	{
		if(strcmp(v_network_bandwidths[i]->m_network.c_str(),network.c_str())==0)
		{
			for(UINT j=0; j<v_network_bandwidths[i]->v_hosts.size();j++)
			{
				if(strcmp(v_network_bandwidths[i]->v_hosts[j].m_ip.c_str(),ip.c_str())==0)
				{
					return v_network_bandwidths[i]->v_hosts[j].IsUp();
				}
			}
		}
	}
	return false;
}

//
//
//
void StatusDestDll::OnStopAndKillKazaa(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		SendGenericMessage(selected_racks[i],StatusHeader::Kill_All_Kazaa);
	}
}

//
//
//
void StatusDestDll::OnResumeKazaa(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		SendGenericMessage(selected_racks[i],StatusHeader::Launch_All_Kazaa);
	}
}

//
//
//
void StatusDestDll::OnRestartKazaa(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		SendGenericMessage(selected_racks[i],StatusHeader::Restart_All_Kazaa);
	}
}

//
//
//
void StatusDestDll::OnRequestKazaaUploads(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		SendGenericMessage(selected_racks[i],StatusHeader::Num_Kazaa_Uploads_Request);
	}
}

//
//
//
void StatusDestDll::OnRestartComputer(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		SendGenericMessage(selected_racks[i],StatusHeader::Restart_Computer);
	}
}

//
//
//
void StatusDestDll::OnChangeMaxKazaa(vector<CString>& selected_racks, UINT max_kazaa)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		byte buf[sizeof(StatusHeader)+sizeof(UINT)];
		StatusHeader* header = (StatusHeader*)buf;
		header->op = StatusHeader::Current_Number_Of_Kazaa_To_Run;
		header->size = sizeof(UINT);

		UINT* pMax = (UINT*)&buf[sizeof(StatusHeader)];
		*pMax = max_kazaa;

		char destination[32];
		strcpy(destination, selected_racks[i]);
		m_com.SendReliableData(destination,buf,sizeof(StatusHeader)+sizeof(UINT));
	}
}

//
//
//
bool StatusDestDll::SendGenericMessage(CString dest, StatusHeader::op_code op_code)
{
	byte buf[sizeof(StatusHeader)];
	StatusHeader* header = (StatusHeader*)buf;
	header->op = op_code;
	header->size = 0;

	char destination[32];
	strcpy(destination, dest);
	return m_com.SendReliableData(destination,buf,sizeof(StatusHeader));
}

//
//
//
void StatusDestDll::ReceivedSystemInfoReply(char* from, SystemInfoData* sys_info)
{
	unsigned int i;
	UINT host_index = 0;

	// See if there is already an item for this IP
	bool found=false;
	for(i=0;i<(unsigned int)v_hosts.size();i++)
	{
		if(strcmp(v_hosts[i].m_ip.c_str(),from)==0)
		{
			found=true;
			host_index = i;
			break;
		}
	}
	if(!found)
	{
		return; //ignore it
	}
	string network = v_hosts[host_index].m_network;
	// Find this ip's network in the network bandwidths
	for(i=0;i<v_network_bandwidths.size();i++)
	{
		if(strcmp(v_network_bandwidths[i]->m_network.c_str(), network.c_str())==0)
		{
			//update individual network dialog status
			v_network_bandwidths[i]->m_dlg.UpdateSystemInfoStatus(from, sys_info);
			break;
		}
	}
}

//
//
//
void StatusDestDll::ReceivedKazaaUploads(char* from, int uploads)
{
	unsigned int i;
	UINT host_index = 0;

	// See if there is already an item for this IP
	bool found=false;
	for(i=0;i<(unsigned int)v_hosts.size();i++)
	{
		if(strcmp(v_hosts[i].m_ip.c_str(),from)==0)
		{
			found=true;
			host_index = i;
			break;
		}
	}
	if(!found)
	{
		return; //ignore it
	}
	string network = v_hosts[host_index].m_network;
	// Find this ip's network in the network bandwidths
	for(i=0;i<v_network_bandwidths.size();i++)
	{
		if(strcmp(v_network_bandwidths[i]->m_network.c_str(), network.c_str())==0)
		{
			//update individual network dialog status
			v_network_bandwidths[i]->m_dlg.UpdateKazaaUploadsStatus(from, uploads);
			break;
		}
	}
}

//
//
//
/*
void StatusDestDll::OnInitStatusDestThreadData(WPARAM wparam,LPARAM lparam)
{
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		CCriticalSection* pCritical_section=(CCriticalSection *)wparam;
		pCritical_section = &m_critical_section;
		StatusDestThreadData* pThread_data = (StatusDestThreadData*)lparam;
		SetEvent(pThread_data->m_events[1]);
		singleLock.Unlock();
	}

	StatusDestThreadData* pThread_data = (StatusDestThreadData*)lparam;
	SetEvent(pThread_data->m_events[1]);
}
*/
//
//
//
void StatusDestDll::OnUpdateNetworkSummary(WPARAM wparam,LPARAM lparam)
{
	string network = (const char*)wparam;
	UpdateNetworkStatus(network);
}

//
//
//
void StatusDestDll::CalculatePingingInterval(void)
{
	float offset = ((float)m_interval / (float)v_hosts.size());
	float interval = offset;
	for(UINT i=0; i<v_hosts.size();i++)
	{
		if(interval <= m_interval - 1)
			v_hosts[i].m_retreve_status_offset = (int)interval;
		else
			v_hosts[i].m_retreve_status_offset = m_interval - 1;
		interval+=offset;
	}
}

//
//
//
void StatusDestDll::ReceivedSharedFilesReply(char* from, FileInfos& reply)
{
	unsigned int i;
	UINT host_index = 0;

	// See if there is already an item for this IP
	bool found=false;
	for(i=0;i<(unsigned int)v_hosts.size();i++)
	{
		if(strcmp(v_hosts[i].m_ip.c_str(),from)==0)
		{
			found=true;
			host_index = i;
			break;
		}
	}
	if(!found)
	{
		return; //ignore it
	}
	string network = v_hosts[host_index].m_network;
	// Find this ip's network in the network bandwidths
	for(i=0;i<v_network_bandwidths.size();i++)
	{
		if(strcmp(v_network_bandwidths[i]->m_network.c_str(), network.c_str())==0)
		{
			//update individual network dialog status
			v_network_bandwidths[i]->m_dlg.ReceivedSharedFilesReply(from, reply);
			break;
		}
	}
}

//
//
//
void StatusDestDll::OnEmergencyRestart(vector<CString>& selected_racks)
{
	RackRecoverInterface rri;

	for(UINT i=0;i<selected_racks.size();i++)
	{
		rri.RestartRack(selected_racks[i]);
	}
}

//
//
//
void StatusDestDll::OnEmergencyRestartNetwork(vector<CString>& selected_networks)
{
	RackRecoverInterface rri;
	//request ips from nameserver
	NameServerInterface nsi;
	vector<string> IPs;
	
	for(UINT i=0;i<selected_networks.size();i++)
	{
		nsi.RequestIP(selected_networks[i], IPs);
		for(UINT j=0;j<IPs.size();j++)
		{
			rri.RestartRack(IPs[j].c_str());
		}
		IPs.clear();
	}
}

//
//
//
void StatusDestDll::RebootAllDownRacks()
{
	vector<CString> reboot_racks;
	for(UINT i=0;i<v_network_bandwidths.size();i++)
	{
		for(UINT j=0; j<v_network_bandwidths[i]->v_hosts.size();j++)
		{
			if(v_network_bandwidths[i]->v_hosts[j].IsUp() == false)
				reboot_racks.push_back(v_network_bandwidths[i]->v_hosts[j].m_ip.c_str());
		}
	}
	OnEmergencyRestart(reboot_racks);
}

//
//
//
void StatusDestDll::OnRemoveDll(vector<CString>& selected_racks, CString filename)
{
	RackRecoverInterface rri;

	for(UINT i=0;i<selected_racks.size();i++)
	{
		rri.DeleteFileOnRack(selected_racks[i],filename);
	}
}

//
//
//
void StatusDestDll::OnRemoveDllNetwork(vector<CString>& selected_networks, CString filename)
{
	RackRecoverInterface rri;
	//request ips from nameserver
	NameServerInterface nsi;
	vector<string> IPs;
	
	for(UINT i=0;i<selected_networks.size();i++)
	{
		nsi.RequestIP(selected_networks[i], IPs);
		for(UINT j=0;j<IPs.size();j++)
		{
			rri.DeleteFileOnRack(IPs[j].c_str(),filename);
		}
		IPs.clear();
	}
}