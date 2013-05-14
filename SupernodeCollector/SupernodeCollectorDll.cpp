#include "StdAfx.h"
#include "supernodecollectordll.h"
#include "DllInterface.h"
#include "IPAddress.h"
#include <Psapi.h>
#include "VectorIPAddress.h"

SupernodeCollectorDll::SupernodeCollectorDll(void)
{
}

//
//
//
SupernodeCollectorDll::~SupernodeCollectorDll(void)
{
}

//
//
//
void SupernodeCollectorDll::DllInitialize()
{
//	m_connections = 0;
	WSocket::Startup(); //init winsock2
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int i=0;i<num_sockets;i++)
	{
		m_sockets[i].InitParent(this);
	}
}

//
//
//
void SupernodeCollectorDll::DllUnInitialize()
{
	SaveSupernodeList();
//	SaveUserNames();
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int j=0;j<num_sockets;j++)
	{
		m_sockets[j].Close();
	}
}

//
//
//
void SupernodeCollectorDll::DllStart()
{
	//read in previous supernodes collected from HDD
	ReadSupernodeList();
	//Getting kazaa path from the registry
	GetKazaaPathFromRegistry();
	//check to see if kazaa is running
	IsKazaaRunning();


	m_dlg.Create(IDD_SC_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);

	//Create and register the com interface
	m_com_interface.Register(this, DllInterface::GetAppID().m_app_id);

	//Set timer
	m_dlg.SetTimer(1, 10*1000, 0); //every 10 sec,import supernodes from registry
	m_dlg.SetTimer(2, 10*1000, 0); //reconnecting kazaa interval
	m_dlg.SetTimer(3, 5*60*1000, 0); //save the supernode list
	m_dlg.SetTimer(4, 1000, 0); //connection status and check for idle sockets
	m_dlg.SetTimer(5, 60*1000,0); //RESET all supernodes at 3am each day
}

//
//
//
void SupernodeCollectorDll::DllShowGUI()
{
	m_dlg.EnableWindow();
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void SupernodeCollectorDll::ReportStatus(HttpSocket* socket, char* ip,int port,int error,char *username,char *supernode_ip,
										   char *supernode_port)
{
	
	//Get the assigned rack ip for this supernode
	int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);

	int ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
	// Find the item that we are connecting to
	IPAddress ip_address;
	ip_address.m_ip = ip_int;
	ip_address.m_port = port;

	vector<IPAddress>::iterator result = NULL;
	result = find(v_temp_supernode_list.begin(),v_temp_supernode_list.end(),ip_address);
	if(result != v_temp_supernode_list.end() && result != NULL)
	{
		if(error==0)
		{
			// Check to see if this dude is a supernode
			if((strlen(supernode_ip)==0) && (strlen(supernode_port)==0))
			{
				//result->m_is_up = true;
				//result->m_down_time = CTime::GetCurrentTime();
				result->m_connecting = false;
				
				//check to see if this supernode is already existed
				bool existed = binary_search(v_supernode_list.begin(),v_supernode_list.end(),*(result));

				if(!existed)
				{
					result->m_username = username;
					//insertion sort
					vector<IPAddress>::iterator location;
					location = lower_bound(v_supernode_list.begin(),v_supernode_list.end(), *(result));
					v_supernode_list.insert(location,*(result));
					v_new_supernode_list.push_back(*(result));
					//p_dlg->m_supernode_distributer_page.RefreshSupernodeList(*(result));
					//p_dlg->m_supernode_distributer_page.ReportStatus(socket,ip,port,error,result->m_rack_name,result->m_up_time,
						//result->m_down_time,username,supernode_ip,supernode_port);
				}
			}
			else //add it's supernode to the big list
			{
				AddSuperNode(supernode_ip, supernode_port); //add it to the list
			}
		}
		v_temp_supernode_list.erase(result);
		//result->m_connecting = false;
	}
	socket->Close();
//	if(m_connections>0)
//		m_connections--;
	//m_dlg.UpdateProbingIndexStatus(m_connections,(int)v_temp_supernode_list.size(),m_probing_index);
	m_dlg.UpdateSupernodesStatus(v_supernode_list.size(),v_temp_supernode_list.size());
	ProbeNextItem();

}

//
//
//
void SupernodeCollectorDll::GetKazaaPathFromRegistry()
{
	CHAR reg_sub_key[] = TEXT("Software\\Kazaa\\Cloudload");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE, &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			BYTE temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = MAX_PATH;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				temp, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ExeDir") == 0)
			{
				m_kazaa_path = (char*)temp;
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void SupernodeCollectorDll::IsKazaaRunning()
{
	//look for kazaa process
	BOOL bret = FALSE;
	bool kazaa_is_running = false;
	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if(stricmp("Kazaa.kpp",name)==0)//if process is named kazaa, K++ edition
			{
				kazaa_is_running = true;
				CloseHandle(handle);
				break;
			}
		}
		CloseHandle(handle);
	}
	if(!kazaa_is_running) //launch kazaa
	{
		LPCTSTR lpszExePath;
//		LPCTSTR lpszRunIn;
		LPTSTR lpszCommand;
		lpszExePath = m_kazaa_path.c_str();
		char command[256];
		/*
		CString kpp_filename = m_kazaa_path.c_str();
		int index = kpp_filename.ReverseFind('\\');
		kpp_filename.Delete(index, kpp_filename.GetLength()-index);
		kpp_filename+="\\KazaaLite.kpp";
		strcat(command, " ");
		*/
		strcpy(command, m_kazaa_path.c_str()); //K++ edition
		lpszCommand =  command;
//		string run_in_dir = GetFreeRunInDir();
//		lpszRunIn = run_in_dir.c_str();

		PROCESS_INFORMATION pi;
		STARTUPINFOA si = {sizeof si};
		bret = CreateProcess(
			lpszExePath,
			//lpszCommand,
			NULL,
			NULL,
			NULL,
			FALSE,
			NORMAL_PRIORITY_CLASS,
			NULL,
			//lpszRunIn,
			NULL,
			&si,
			&pi);
	}
}

//
//
//
void SupernodeCollectorDll::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
		case 1://importing supernodes from registry
		{
			ImportSuperNodeList();
			break;
		}
		case 2://reconnecting kazaa
		{
			ReconnectKazaa();
			break;
		}
		case 3:
		{
			IsKazaaRunning();
			SaveSupernodeList();
			break;
		}
		case 4://check for idle sockets and get connection status
		{
			KillAllIdleSockets();
			break;
		}
		case 5:
		{
			CTime current_time = CTime::GetCurrentTime();
			if(current_time.GetHour()==3 && current_time.GetMinute()==0)
				ResetSupernodes();
			break;
		}
	}
}

//
//
//
void SupernodeCollectorDll::ImportSuperNodeList()
{
	vector<IPAddress> ip_list;
	m_supernode_collector.ImportSupernodeList(ip_list);
	vector<IPAddress>::iterator iter = ip_list.begin();
	while(iter!=ip_list.end())
	{
		IPAddress ip = *(iter);
		bool found = binary_search(v_supernode_list.begin(),v_supernode_list.end(),ip );
		vector<IPAddress>::iterator finding = NULL;
		finding = find(v_temp_supernode_list.begin(),v_temp_supernode_list.end(),ip);
		if(finding != NULL && finding != v_temp_supernode_list.end())
			found = true;
		if(found)
			ip_list.erase(iter);
		else
			iter++;
	}
	vector<IPAddress>::iterator location;
	for(unsigned int i=0; i<ip_list.size();i++)
	{

		v_temp_supernode_list.push_back(ip_list[i]);
	}
	if(ip_list.size()!=0)
		//p_dlg->m_supernode_distributer_page.RefreshSupernodeList(ip_list);
		ProbeNextItem();
}

//
//
//
void SupernodeCollectorDll::ProbeNextItem()
{
	// Find the next item that is not connected before
	int j, i;
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);

	for(i=v_temp_supernode_list.size()-1; i>=0;i--)
	{
		if(v_temp_supernode_list[i].m_connecting == false)
		{
			// See if we can find another idle socket
			int socket_index=-1;
			for(j=0;j<num_sockets;j++)
			{
				if(m_sockets[j].IsSocket()==false)
				{
					socket_index=j;
					break;
				}
			}

			// We couldn't find another idle socket
			if(socket_index==-1)
			{
				break;
			}
			v_temp_supernode_list[i].m_connecting = true;

			char ip[15+1];	// xxx.xxx.xxx.xxx\0

			sprintf(ip,"%u.%u.%u.%u",(v_temp_supernode_list[i].m_ip>>0)&0xFF,
				(v_temp_supernode_list[i].m_ip>>8)&0xFF,
				(v_temp_supernode_list[i].m_ip>>16)&0xFF,
				(v_temp_supernode_list[i].m_ip>>24)&0xFF);
			
			m_sockets[socket_index].Create();
			m_sockets[socket_index].Connect(ip,v_temp_supernode_list[i].m_port,CTime::GetCurrentTime());
//			m_connections++;
//			m_dlg.UpdateProbingIndexStatus(m_connections,(int)v_temp_supernode_list.size(),i);
		}
	}
}

//
//
//
void SupernodeCollectorDll::ReconnectKazaa()
{
	m_supernode_collector.ReconnectKazaa();
}

//
//
//
void SupernodeCollectorDll::KillAllIdleSockets()
{
	int connections = 0;
	int connecting = 0;
	int timeout_connections=0;
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int i=0;i<num_sockets;i++)
	{
		if(m_sockets[i].IsSocket())
		{
			CTime current_time = CTime::GetCurrentTime();
			CTimeSpan elapse_time = current_time - m_sockets[i].m_start_time;
			LONGLONG seconds = elapse_time.GetTotalSeconds();
			if(seconds > 30)
			{
				m_sockets[i].TimeOut();
				timeout_connections++;
			}
			else
				connections++;
		}
	}
//	if(connections == 0)
//		m_probing_index = 0;
//	char msg[128];
//	sprintf(msg, "Connecting sockets: %d, Timeout Sockets: %d", connections, timeout_connections);
//	m_dlg.Log(msg);
	vector<IPAddress>::iterator iter = v_temp_supernode_list.begin();
	while(iter != v_temp_supernode_list.end())
	{
		if(iter->m_connecting == true)
		{
			CTime current_time = CTime::GetCurrentTime();
			CTimeSpan elapse_time = current_time - iter->m_connecting_time;
			LONGLONG seconds = elapse_time.GetTotalSeconds();
			if(seconds > 30)
			{
				v_temp_supernode_list.erase(iter);
				continue;
			}
			else
				connecting++;
		}
		iter++;
	}

	m_dlg.UpdateConnectionStatus(connections,num_sockets-connections-timeout_connections,timeout_connections,connecting);
}

//
//
//
void SupernodeCollectorDll::AddSuperNode(char* supernode_ip, char* supernode_port)
{
	UINT ip1,ip2,ip3,ip4,supernode_ip_int,supernode_port_int;
	sscanf(supernode_ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	supernode_ip_int=(ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
	sscanf(supernode_port,"%u", &supernode_port_int);
	
	IPAddress ip_address;
	ip_address.m_ip = supernode_ip_int;
	ip_address.m_port = supernode_port_int;
	
	//add this new supernode into the list if it is not already there
	bool found = binary_search(v_supernode_list.begin(),v_supernode_list.end(),ip_address );
	vector<IPAddress>::iterator finding = NULL;
	finding = find(v_temp_supernode_list.begin(),v_temp_supernode_list.end(),ip_address);
	if(finding != NULL && finding != v_temp_supernode_list.end())
		found = true;

	if(!found)
	{
		//insertion sort
		//vector<IPAddress>::iterator location;
		//location = lower_bound(v_supernode_list.begin(),v_supernode_list.end(), ip_address);
		//v_supernode_list.insert(location,ip_address);
		//p_dlg->m_supernode_distributer_page.AddSuperNode(supernode_ip,supernode_port);
		v_temp_supernode_list.push_back(ip_address);
	}
}

//
//
//
void SupernodeCollectorDll::SaveSupernodeList()
{
	if(v_supernode_list.size() > 0 )
	{
		CFile file;
		if( file.Open("supernodes.dat",
			CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite)==1)
		{
			for(UINT i=0; i<v_supernode_list.size(); i++)
			{
				int buf_len = v_supernode_list[i].GetBufferLength();
				char *buf = new char[buf_len];
				v_supernode_list[i].WriteToBuffer(buf);
				file.Write(&buf_len, sizeof(int));
				file.Write(buf, buf_len);
				delete [] buf;
			}
			file.Close();
		}
	}
}

//
//
//
void SupernodeCollectorDll::ReadSupernodeList()
{
	CFile file;
	if( file.Open("supernodes.dat",
			CFile::typeBinary|CFile::modeRead|CFile::shareDenyWrite)==1)
	{
		int buf_len = 0;
		while(file.Read(&buf_len, sizeof(int)))
		{
			unsigned char * data =  new unsigned char[buf_len];
			file.Read(data, buf_len);
			IPAddress new_ip;
			new_ip.ReadFromBuffer((char*)data);
			new_ip.m_connecting = false;
			new_ip.m_connecting_time = CTime::GetCurrentTime();
			new_ip.m_num_assigned = 0;
			v_temp_supernode_list.push_back(new_ip);
			delete [] data;
		}
		file.Close();
	}
	ProbeNextItem();
}

//
//
//
void SupernodeCollectorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote SD Dll data
	KazaaControllerHeader* header = (KazaaControllerHeader*)data;
	byte *pData=(byte *)data;
	pData+=sizeof(KazaaControllerHeader);
	
	switch(header->op)
	{
		case(KazaaControllerHeader::Source_Init):
		{
			TRACE("SupernodeCollector: Received message \"Source_Init\" from %s\n",source_name);
			SendControllerGenericMessage(source_name, KazaaControllerHeader::Collector_Init_Response);
			TRACE("SupernodeCollector: Sent message \"Collector_Init_Response\" to %s\n",source_name);
			break;
		}
		case(KazaaControllerHeader::Request_Whole_Supernodes_List):
		{
			TRACE("SupernodeCollector: Received message \"Request_Whole_Supernodes_List\" from %s\n",source_name);
			if(SendWholeSupernodesList(source_name))
				v_new_supernode_list.clear();
			break;
		}
		case (KazaaControllerHeader::Request_New_Supernodes_List):
		{
			TRACE("SupernodeCollector: Received message \"Request_New_Supernodes_List\" from %s\n",source_name);
			if(SendNewSupernodesList(source_name))
				v_new_supernode_list.clear();
			break;
		}
	}
}

//
//
//
bool SupernodeCollectorDll::SendControllerGenericMessage(char* dest, KazaaControllerHeader::op_code op_code)
{
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = op_code;
	header->size = 0;
	return m_com_interface.SendReliableData(dest,buf,sizeof(KazaaControllerHeader));
}

//
//
//
bool SupernodeCollectorDll::SendWholeSupernodesList(char* dest)
{
	VectorIPAddress ip_addresses;
	for(UINT i=0; i<v_supernode_list.size();i++)
	{
		ip_addresses.v_ip_addresses.push_back(v_supernode_list[i]);
	}
	int buf_len = sizeof(KazaaControllerHeader) + ip_addresses.GetBufferLength();
	char* buf = new char[buf_len];
	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Whole_Supernodes_List;
	header->size = buf_len - sizeof(KazaaControllerHeader);
	char* ptr = &buf[sizeof(KazaaControllerHeader)];
	ip_addresses.WriteToBuffer(ptr);
	bool ret = m_com_interface.SendReliableData(dest,buf,buf_len);
	delete [] buf;
	return ret;
}

//
//
//
bool SupernodeCollectorDll::SendNewSupernodesList(char* dest)
{
	VectorIPAddress ip_addresses;
	for(UINT i=0; i<v_new_supernode_list.size();i++)
	{
		ip_addresses.v_ip_addresses.push_back(v_new_supernode_list[i]);
	}
	int buf_len = sizeof(KazaaControllerHeader) + ip_addresses.GetBufferLength();
	char* buf = new char[buf_len];
	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::New_Supernodes_List;
	header->size = buf_len - sizeof(KazaaControllerHeader);
	char* ptr = &buf[sizeof(KazaaControllerHeader)];
	ip_addresses.WriteToBuffer(ptr);
	bool ret = m_com_interface.SendReliableData(dest,buf,buf_len);
	delete [] buf;
	return ret;
}

//
//
//

void SupernodeCollectorDll::ResetSupernodes(void)
{
	SaveSupernodeList();
	v_supernode_list.clear();
	v_temp_supernode_list.clear();
	v_new_supernode_list.clear();
	ReadSupernodeList();
}
