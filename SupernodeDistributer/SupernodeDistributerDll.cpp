#include "StdAfx.h"
#include "supernodedistributerdll.h"
#include "DllInterface.h"
#include "KazaaControllerHeader.h"
#include "../NameServer/NameServerInterface.h"
#include <Psapi.h>
#include "../TKSyncher/TKSyncherInterface.h"
#include "imagehlp.h"

inline bool UpTimeSort(const IPAddress& ip1, const IPAddress& ip2)
{
	if(ip1.m_up_time < ip2.m_up_time)
		return true;
	else
		return false;
}

//
//
//
SupernodeDistributerDll::SupernodeDistributerDll(void)
{
	m_probing_index = 0;
	m_connections = 0;
	p_com_interface = NULL;
	p_dlg = NULL;
	m_second_count=0;
}

//
//
//
SupernodeDistributerDll::~SupernodeDistributerDll(void)
{

}

//
//
//
void SupernodeDistributerDll::DllInitialize()
{
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int i=0;i<num_sockets;i++)
	{
		m_sockets[i].InitParent(this);
	}
}

//
//
//
void SupernodeDistributerDll::DllUnInitialize()
{
	SaveSupernodeList();
	SaveUserNames();
//	SaveRacksMaxKazaaNumber();
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int j=0;j<num_sockets;j++)
	{
		m_sockets[j].Close();
	}
	if(p_dlg != NULL)
	{
		p_dlg->DestroyWindow();
		p_dlg->OnExit();
		delete p_dlg;
	}
	if(p_com_interface != NULL)
	{
		delete p_com_interface;
	}
}

//
//
//
void SupernodeDistributerDll::DllStart()
{
	//Getting kazaa path from the registry
	GetKazaaPathFromRegistry();
	//check to see if kazaa is running
	IsKazaaRunning();

	WSocket::Startup(); //init winsock2
	p_dlg = new CMainDlg("Kazaa Controller",CWnd::GetDesktopWindow());	//create the dialog
	p_dlg->InitParent(this);
	p_dlg->Create(CWnd::GetDesktopWindow(),WS_DISABLED|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SIZEBOX);

	//Create and register the com interface
	p_com_interface = new SDComInterface();
	p_com_interface->Register(this, DllInterface::GetAppID().m_app_id);

	//Init name server
	//m_nameserver.InitParent(this);
	//m_nameserver.Register(this);
	vector<string> ips;
	NameServerInterface ns;
	ns.RequestIP("KAZAA", ips); //request all ips of kazaa racks
	ReceivedIPs(ips); //process the ip list

//	ReadRacksMaxKazaaNumber(); //read in saved max kazaa numbers for each rack


	CalculatePingOffset();

	
	//Loading the saved supernodes list
	LoadSupernodeList();
	//Set timer
	p_dlg->m_supernode_distributer_page.SetTimer(1, 10*1000, 0); //every 10 sec,import supernodes from registry
	p_dlg->m_supernode_distributer_page.SetTimer(2, p_dlg->m_supernode_distributer_page.m_launching_interval*1000, 0); //reconnecting kazaa interval
	p_dlg->m_supernode_distributer_page.SetTimer(3, 15*60*1000,0); //send init to all racks every 15 mins
	p_dlg->m_supernode_distributer_page.SetTimer(4, 60*1000, 0); //check for idle sockets every min
	p_dlg->m_supernode_distributer_page.SetTimer(5, 30*60*1000, 0); //Maintenance  timer - 30 mins
	p_dlg->m_supernode_distributer_page.SetTimer(6, 5*1000, 0); //delay first init message to all racks to 5 sec
	p_dlg->m_supernode_distributer_page.SetTimer(7, 1000, 0); //1 sec
}

//
//
//
void SupernodeDistributerDll::DllShowGUI()
{
	p_dlg->EnableWindow();
	p_dlg->ShowWindow(SW_NORMAL);
	p_dlg->BringWindowToTop();
}

//
//
//
void SupernodeDistributerDll::ReportStatus(HttpSocket* socket, char* ip,int port,int error,char *username,char *supernode_ip,
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
				result->m_is_up = true;
				result->m_down_time = CTime::GetCurrentTime();
				result->m_connecting = false;
				
				//check to see if this supernode is already existed
				bool existed = binary_search(v_supernode_list.begin(),v_supernode_list.end(),*(result));

				if(!existed)
				{
					//insertion sort
					vector<IPAddress>::iterator location;
					location = lower_bound(v_supernode_list.begin(),v_supernode_list.end(), *(result));
					v_supernode_list.insert(location,*(result));
					p_dlg->m_supernode_distributer_page.RefreshSupernodeList(*(result));
					p_dlg->m_supernode_distributer_page.ReportStatus(socket,ip,port,error,result->m_rack_name,result->m_up_time,
						result->m_down_time,username,supernode_ip,supernode_port);
				}
			}
			else //add it's supernode to the big list
			{
				AddSuperNode(supernode_ip, supernode_port); //add it to the list
			}
		}/*
		else
		{
			result->m_is_up = false;
		}
		*/
		v_temp_supernode_list.erase(result);
		//result->m_connecting = false;
	}
	socket->Close();
	if(m_connections>0)
		m_connections--;
	if(m_probing_index > 0)
		m_probing_index--;
	p_dlg->m_supernode_distributer_page.UpdateProbingIndexStatus(m_connections,(int)v_temp_supernode_list.size());
	ProbeNextItem();
}

//
//
//
void SupernodeDistributerDll::TotallyRemoveSuperNode(char* source, int ip, int port)
{
	// Find the item that we are removing
	IPAddress ip_address;
	ip_address.m_ip = ip;
	ip_address.m_port = port;
	vector<IPAddress>::iterator result = NULL;
	result = find(v_supernode_list.begin(),v_supernode_list.end(),ip_address);
	if(result != v_supernode_list.end() && result != NULL)
	{
		v_supernode_list.erase(result); //remove it from the main list
	}

	for(UINT i=0;i<v_rack_list.size();i++) //remove the supernode from v_rack_list
	{
		bool found = false;
		if(v_rack_list[i].m_rack_name.compare(source)==0)
		{
			found = true;
			vector<IPAndPort>::iterator iter = v_rack_list[i].v_supernodes.begin();
			while(iter != v_rack_list[i].v_supernodes.end())
			{
				if(iter->IsEqual(ip, port))
				{
					v_rack_list[i].v_supernodes.erase(iter);
					break;
				}
				iter++;
			}
		}
		if(found)
			break;
	}

	//remove the supernode from the dialog
	IPAndPort ip_port(ip, port);
	p_dlg->m_supernode_distributer_page.RemoveSuperNode(ip_port);
}

//
//
//
void SupernodeDistributerDll::AddSuperNode(char* supernode_ip, char* supernode_port)
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
UINT SupernodeDistributerDll::GetSuperNodeListSize()
{
	return (UINT)v_supernode_list.size();
}

//
//
//
void SupernodeDistributerDll::ProbeNextItem()
{
	// Find the next item that is not connected before
	int j;
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);

	while(true)
	{
		if(m_probing_index < v_temp_supernode_list.size())
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
			v_temp_supernode_list[m_probing_index].m_connecting = true;
			//p_dlg->m_supernode_distributer_page.SetConnectingStatus(v_supernode_list[m_probing_index].m_ip);

			char ip[15+1];	// xxx.xxx.xxx.xxx\0

			sprintf(ip,"%u.%u.%u.%u",(v_temp_supernode_list[m_probing_index].m_ip>>0)&0xFF,
				(v_temp_supernode_list[m_probing_index].m_ip>>8)&0xFF,
				(v_temp_supernode_list[m_probing_index].m_ip>>16)&0xFF,
				(v_temp_supernode_list[m_probing_index].m_ip>>24)&0xFF);
			
			m_sockets[socket_index].Create();
			m_sockets[socket_index].Connect(ip,v_temp_supernode_list[m_probing_index].m_port,CTime::GetCurrentTime());
			m_connections++;
			m_probing_index++;
			/*
			if(v_temp_supernode_list[m_probing_index].m_is_up == false)//reset the up time if it was down
				v_temp_supernode_list[m_probing_index].m_up_time = CTime::GetCurrentTime();
			*/
			p_dlg->m_supernode_distributer_page.UpdateProbingIndexStatus(m_connections,(int)v_temp_supernode_list.size());
		}
		else
		{
			//m_probing_index = 0;
			p_dlg->m_supernode_distributer_page.UpdateProbingIndexStatus(m_connections,(int)v_temp_supernode_list.size());
			break;
		}
	}
}

//
//
//
void SupernodeDistributerDll::LoadSupernodeList(void)
{
	CFile file;
	vector<IPAddress> ip_list;
	if( file.Open("supernodes.dat",CFile::typeBinary|CFile::modeRead|CFile::shareDenyWrite)==1)
	{
		IPAddress node;
		while(file.Read(&node, sizeof(IPAddress)))
		{
			node.Clear();
			ip_list.push_back(node);
		}
		file.Close();
	}
	vector<IPAddress>::iterator iter = ip_list.begin();
	while(iter!=ip_list.end())
	{
		IPAddress ip = *(iter);
		bool found = binary_search(v_supernode_list.begin(),v_supernode_list.end(),ip );
		if(found)
			ip_list.erase(iter);
		else
			iter++;
	}
	bool updated = false;
	for(unsigned int i=0; i<ip_list.size();i++)
	{
		updated = true;
		v_temp_supernode_list.push_back(ip_list[i]);
	}
	if(updated)
	{
		sort(v_temp_supernode_list.begin(),v_temp_supernode_list.end());
		//p_dlg->m_supernode_distributer_page.RefreshSupernodeList(ip_list);
		ProbeNextItem();
	}	
}

//
//
//
void SupernodeDistributerDll::SaveSupernodeList(void)
{
	if(v_supernode_list.size() > 0 || v_temp_supernode_list.size() > 0)
	{
		CFile file;
		if( file.Open("supernodes.dat",
			CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite)==1)
		{
			vector<IPAddress>::iterator iter = v_supernode_list.begin();
			while(iter != v_supernode_list.end())
			{
				file.Write(&(*iter), sizeof(IPAddress));
				iter++;
			}
			iter=v_temp_supernode_list.begin();
			while(iter != v_temp_supernode_list.end())
			{
				file.Write(&(*iter),sizeof(IPAddress));
				iter++;
			}
			file.Close();
		}
	}
}

//
//
//
void SupernodeDistributerDll::ImportSuperNodeList(void)
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
		/*
		//insertion sort
		location = lower_bound(v_supernode_list.begin(),v_supernode_list.end(), ip_list[i]);
		v_supernode_list.insert(location,ip_list[i]);
		*/
		v_temp_supernode_list.push_back(ip_list[i]);
	}
	if(ip_list.size()!=0)
		//p_dlg->m_supernode_distributer_page.RefreshSupernodeList(ip_list);
		ProbeNextItem();
}

//
//
//
void SupernodeDistributerDll::ReconnectKazaa(void)
{
	m_supernode_collector.ReconnectKazaa();
}

//
//
//
void SupernodeDistributerDll::OnMaintenanceTimer(void)
{
	/*
	vector<IPAddress> remove_list;
	vector<IPAddress>::iterator iter = v_supernode_list.begin();
	CTimeSpan ts;
	while(iter != v_supernode_list.end())
	{
		ts = CTime::GetCurrentTime() - iter->m_down_time;
		if(ts > CTimeSpan(3,0,0,0)) // remove all supernodes have been down for 3 days or more
			remove_list.push_back(*(iter));
		iter++;
	}
	p_dlg->m_supernode_distributer_page.RemoveSuperNodes(remove_list);
	for(int i=0; i<remove_list.size();i++)
	{
		iter = find(v_supernode_list.begin(),v_supernode_list.end(),remove_list[i]);
		if(iter!=v_supernode_list.end())
			v_supernode_list.erase(iter);
	}
*/
	SaveSupernodeList();
	SaveUserNames();
}

//
//
//
void SupernodeDistributerDll::KillAllIdleSockets(void)
{
	m_connections = 0;
	int num_sockets=sizeof(m_sockets)/sizeof(HttpSocket);
	for(int i=0;i<num_sockets;i++)
	{
		if(m_sockets[i].IsSocket())
		{
			m_connections++;
			CTime current_time = CTime::GetCurrentTime();
			CTimeSpan elapse_time = current_time - m_sockets[i].m_start_time;
			LONGLONG seconds = elapse_time.GetTotalSeconds();
			if(seconds > 30)
			{
				m_sockets[i].TimeOut();
			}
		}
	}
	if(m_connections == 0)
		m_probing_index = 0;
}

//
//
//
void SupernodeDistributerDll::SaveUserNames(void)
{
	if(v_supernode_list.size() > 0)
	{
		CStdioFile file;
		MakeSureDirectoryPathExists("c:\\syncher\\src\\Kazaa-Launcher\\");
		if( file.Open("c:\\syncher\\src\\Kazaa-Launcher\\namelist.txt",CFile::modeWrite|CFile::modeCreate|CFile::typeText) == 1)
		{
			for(int i=0; i<p_dlg->m_supernode_distributer_page.m_ip_list.GetItemCount();i++)
			{
				char username[128+1];
				memset(&username, 0, sizeof(username));
				p_dlg->m_supernode_distributer_page.m_ip_list.GetItemText(i, SUB_USERNAME, username, sizeof(username));
				if(strlen(username) > 0)
				{
					if(strcmp(username, "*Unknown*")!=0)
					{
						file.WriteString(username);
						file.WriteString("\n");
					}
				}
			}
			file.Close();
			//notify the syncher to re-scan the source
			TKSyncherInterface tki;
			tki.RescanSource("Kazaa-Launcher");
		}
	}
}

//
//
//
vector<IPAndPort> SupernodeDistributerDll::GetSupernodesFromRackList(string& rack_name, UINT& kazaa_running)
{
	for(unsigned int i=0; i<v_rack_list.size(); i++)
	{
		if(strcmp(v_rack_list[i].m_rack_name.c_str(),rack_name.c_str())==0)
		{
			kazaa_running = v_rack_list[i].m_num_kazaa_running;
			return v_rack_list[i].v_supernodes;
		}
	}
	return vector<IPAndPort>();
}

//
//
//
void SupernodeDistributerDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote SD Dll data
	KazaaControllerHeader* header = (KazaaControllerHeader*)data;
	byte *pData=(byte *)data;
	pData+=sizeof(KazaaControllerHeader);
	
	switch(header->op)
	{
		case(KazaaControllerHeader::Dest_Init_Response):
		{
			TRACE("Kazaa Controller: Received message \"Dest_Init_Response\" from %s\n",source_name);
			//Add the node to the rack list
			AddNodeToRackList(source_name);
			
			//Request remote's current supernodes list
			SendControllerGenericMessage(source_name, KazaaControllerHeader::Source_Request_Current_Supernodes_list);
			TRACE("Kazaa Controller: Sent message \"Source_Request_Current_Supernodes_list\" to %s\n",source_name);

			
			//Send Number of kazaa to run for this rack
			//SendKazaaNum(source_name);
			break;
		}
		case(KazaaControllerHeader::Dest_Supernode_Request):
		{
			TRACE("Kazaa Controller: Received message \"Dest_Supernode_Request\" from %s\n",source_name);
			UINT* num = (UINT*)pData;
			//Send a list of supernodes to the rack
			if(*num < 200) //not allowing to request more than 200 supernodes
				SendSupernodesToRack(source_name,*num);
			break;
		}
		case (KazaaControllerHeader::Dest_Current_Supernodes_list):
		{
			TRACE("Kazaa Controller: Received message \"Dest_Current_Supernodes_list\" from %s\n",source_name);
			vector<IPAndPort> supernodes;
			IPAndPort ip_port;
			UINT* kazaa_num = (UINT*)pData;
			pData+=sizeof(UINT);
			UINT num = *((UINT*)pData);
			pData+=sizeof(UINT);
			for(UINT i=0;i<num;i++)
			{
				ip_port.m_ip = *((int*)pData);
				pData+=sizeof(int);
				ip_port.m_port = *((int*)pData);
				pData+=sizeof(int);
				supernodes.push_back(ip_port);
			}
			
			//Update our rack list's supernodes
			ReceivedRemoteSupernodeList(source_name, supernodes, *kazaa_num);
			
			//Send more supernodes to the rack if it isn't running the number of kazaa we ask for
			break;
		}
		case (KazaaControllerHeader::Dest_Request_New_Supernode):
		{
			int* old_ip = (int*)pData;
			pData+=sizeof(int);
			int* old_port = (int*)pData;
			IPAndPort old_ip_port;
			old_ip_port.m_ip = *old_ip;
			old_ip_port.m_port = *old_port;
			string supernode = GetIPStringFromIPInt(*old_ip);
			TRACE("Kazaa Controller: Received message \"Dest_Request_New_Supernode\" from %s Supernode: %s:%d\n",source_name,supernode.c_str(),*old_port);
			ReceivedRequestForNewSupernode(source_name,old_ip_port,old_ip_port);
			break;
		}
		case (KazaaControllerHeader::Dest_Check_New_Supernode):
		{
			int* old_ip = (int*)pData;
			pData+=sizeof(int);
			int* old_port = (int*)pData;
			pData+=sizeof(int);
			int* new_ip = (int*)pData;
			pData+=sizeof(int);
			int* new_port = (int*)pData;
			pData+=sizeof(int);

			IPAndPort old_ip_port,new_ip_port;
			old_ip_port.m_ip = *old_ip;
			old_ip_port.m_port = *old_port;
			new_ip_port.m_ip = *new_ip;
			new_ip_port.m_port = *new_port;

			string old_supernode = GetIPStringFromIPInt(*old_ip);
			string new_supernode = GetIPStringFromIPInt(*new_ip);

			TRACE("Kazaa Controller: Received message \"Dest_Check_New_Supernode\" from %s Old supernode: %s:%d New supernode: %s:%d\n",source_name,old_supernode.c_str(),*old_port,new_supernode.c_str(),*new_port);
			ReceivedCheckNewSupernode(source_name, old_ip_port, new_ip_port);
			break;
		}
		case (KazaaControllerHeader::Dest_Remove_Supernode):
		{
			TRACE("Kazaa Controller: Received message \"Dest_Remove_Supernode\" from %s\n",source_name);
			int* ip = (int*)pData;
			pData+=sizeof(int);
			int* port = (int*)pData;
			TotallyRemoveSuperNode(source_name, *ip, *port);
			break;
		}
		case (KazaaControllerHeader::Request_All_Supernodes):
		{
			TRACE("Kazaa Controller: Received message \"Request_All_Supernodes\" from %s\n",source_name);
            SendAllSupernodesToRack(source_name);
		}
	}
}

//
//
//
bool SupernodeDistributerDll::ReceivedCheckNewSupernode(char* source_name, IPAndPort& old_ip_port, IPAndPort& new_ip_port)
{
	bool supernode_is_taken = false;
	IPAddress new_ipaddress(new_ip_port);
	IPAddress old_ipaddress(old_ip_port);
	vector<IPAddress>::iterator location = NULL;

	//delete the old supernode from the main list
	location = find(v_supernode_list.begin(),v_supernode_list.end(), old_ipaddress);
	if(location != NULL && location != v_supernode_list.end())
	{
		v_supernode_list.erase(location);
		//remove the supernode from the dialog
		p_dlg->m_supernode_distributer_page.RemoveSuperNode(old_ip_port);
	}

	//look for the new one
	location = find(v_supernode_list.begin(),v_supernode_list.end(),new_ipaddress);
	if(location != NULL && location != v_supernode_list.end()) //if we already have the supernode on the list
	{
		if(strcmp(location->m_rack_name.c_str(), "NULL")!=0) //if the supernode is taken already
			supernode_is_taken = true;
		else	//it's not taken, assigning it to this rack
			location->m_rack_name = source_name;
	}
	else //add this node to the main list
	{
		TRACE("Kazaa Controller: New supernode from the rack added to our lists\n");
		new_ipaddress.m_rack_name = source_name;					
		//insertion sort
		vector<IPAddress>::iterator insert_location;
		insert_location = lower_bound(v_supernode_list.begin(),v_supernode_list.end(), new_ipaddress);
		v_supernode_list.insert(location,new_ipaddress);

		//add new supernode to the dialog
		p_dlg->m_supernode_distributer_page.AddSuperNode(new_ipaddress,"*Unknown*",source_name);

		//update the rack list
		for(UINT i=0;i<v_rack_list.size();i++)
		{
			bool found = false;
			if(strcmp(v_rack_list[i].m_rack_name.c_str(),source_name)==0)
			{
				for(UINT j=0;j<v_rack_list[i].v_supernodes.size();j++)
				{
					if(v_rack_list[i].v_supernodes[j] == old_ip_port)
					{
						v_rack_list[i].v_supernodes[j] = new_ip_port;
						found = true;
						break;
					}
				}
				if(!found) //add this supernode to the rack list
				{
					v_rack_list[i].v_supernodes.push_back(new_ip_port);
					found = true;
				}
			}
			if(found)
				break;
		}
        return true;
	}
	if(supernode_is_taken) // we will send a renew supernode message to the rack
	{
		return ReceivedRequestForNewSupernode(source_name, old_ip_port, new_ip_port);
	}
	return true;
}

//
//
//
bool SupernodeDistributerDll::ReceivedRequestForNewSupernode(char* source_name, IPAndPort& old_ip, IPAndPort& current_ip)
{
	UINT i,j;
	IPAndPort new_ip;
	if(GetSupernode(source_name, old_ip, new_ip))
	{
		bool found = false;
		for(i=0;i<v_rack_list.size();i++)//assigning the new supernode to the rack
		{
			if(strcmp(v_rack_list[i].m_rack_name.c_str(),source_name)==0)
			{
				for(j=0;j<v_rack_list[i].v_supernodes.size();j++)
				{
					if(v_rack_list[i].v_supernodes[j] == old_ip)
					{
						v_rack_list[i].v_supernodes[j] =  new_ip;
						found = true;
						break;
					}
				}
			}
			if(found)
				break;
		}
	}
	//Send the new supernode along with the rack current supernode to the rack
	return SendRenewSupernode(source_name, current_ip, new_ip);
}

//
//
//
bool SupernodeDistributerDll::SendRenewSupernode(char* dest, IPAndPort& old_ip, IPAndPort& new_ip)
{
	bool ret = false;
	UINT buf_length = sizeof(KazaaControllerHeader)+sizeof(int)*4;
	byte* buf = new byte[buf_length];

	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Source_Renew_Supernode;
	header->size = buf_length - sizeof(KazaaControllerHeader);

	byte* ptr = &buf[sizeof(KazaaControllerHeader)];
	*((int*)ptr) = old_ip.m_ip;
	ptr+=sizeof(int);
	*((int*)ptr) = old_ip.m_port;
	ptr+=sizeof(int);
	*((int*)ptr) = new_ip.m_ip;
	ptr+=sizeof(int);
	*((int*)ptr) = new_ip.m_port;
	ptr+=sizeof(int);

	ret = p_com_interface->SendReliableData(dest, buf, buf_length);
	TRACE("Kazaa Controller: Sent message \"Source_Renew_Supernode\" to %s\n",dest);
	delete [] buf;
	return ret;
}

//
//
//
bool SupernodeDistributerDll::GetSupernode(char* source_name, IPAndPort& old_ip, IPAndPort& new_ip)
{
	IPAddress old_ipaddress(old_ip);

	vector<IPAddress> tmp_list = v_supernode_list;		//copy from the main list
	sort(tmp_list.begin(),tmp_list.end(), UpTimeSort);	//sort the temp list by up time
	UINT j=0;
	bool found = false;
	while(j<tmp_list.size())
	{
		if(strcmp(tmp_list[j].m_rack_name.c_str(),"NULL")==0 && tmp_list[j].m_is_up == true)
		{
			new_ip.m_ip = tmp_list[j].m_ip;
			new_ip.m_port = tmp_list[j].m_port;
			j++;
			found = true;
			break;
		}
		j++;
	}
	//delete the old supernode from the main list
	vector<IPAddress>::iterator location = NULL;
	location = find(v_supernode_list.begin(),v_supernode_list.end(),old_ipaddress);
	if(location != NULL && location != v_supernode_list.end())
	{
		v_supernode_list.erase(location);
		//remove the supernode from the dialog
		p_dlg->m_supernode_distributer_page.RemoveSuperNode(old_ip);
	}
	IPAddress new_ipaddress(new_ip);
	//assign the new_supernode to the rack on the main list
	location = NULL;
	location = find(v_supernode_list.begin(),v_supernode_list.end(),new_ipaddress);
	if(location != NULL && location != v_supernode_list.end())
	{
		location->m_rack_name = source_name;
	}
	p_dlg->m_supernode_distributer_page.RefreshSuperNodeListWithRack(source_name,new_ip);
	return found;
}

//
//
//
bool SupernodeDistributerDll::SendSupernodesToRack(char* dest, UINT num_request)
{
	bool ret = false;
	byte* buf;
	UINT data_length = sizeof(KazaaControllerHeader)
						+sizeof(UINT) //number of IPs
						+2*sizeof(int)*num_request; //IP + port
	buf = new byte[data_length]; 

	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Source_Supernodes_Reply;
	header->size = data_length - sizeof(KazaaControllerHeader);

	vector<IPAndPort> ip_list;
	vector<IPAddress> tmp_list = v_supernode_list;
	sort(tmp_list.begin(),tmp_list.end(), UpTimeSort);

	UINT i=0;
	UINT j=0;
	for(i=0; i<num_request;i++)
	{
		while(j<tmp_list.size())
		{
			if(strcmp(tmp_list[j].m_rack_name.c_str(),"NULL")==0 && tmp_list[j].m_is_up == true)
			{
				IPAndPort ip;
				ip.m_ip = tmp_list[j].m_ip;
				ip.m_port = tmp_list[j].m_port;
				ip_list.push_back(ip);
				j++;
				break;
			}
			j++;
		}
	}

	if(ip_list.size() == num_request)
	{
		vector<IPAddress>::iterator result = NULL;
		IPAddress ip;

		UINT *num = (UINT *)&buf[sizeof(KazaaControllerHeader)];
		*num= (UINT)ip_list.size();
		byte* ptr = &buf[sizeof(KazaaControllerHeader)+sizeof(UINT)];


		for(i=0;i<v_rack_list.size();i++)
		{
			if(strcmp(v_rack_list[i].m_rack_name.c_str(),dest)==0)
			{
				for(j=0;j<ip_list.size();j++)
				{
					v_rack_list[i].v_supernodes.push_back(ip_list[j]);
				}
				break;
			}
		}
		for(i=0;i<ip_list.size();i++)
		{
			ip.m_ip = ip_list[i].m_ip;
			ip.m_port = ip_list[i].m_port;
			result = find(v_supernode_list.begin(),v_supernode_list.end(),ip);
			if(result != v_supernode_list.end())
				result->m_rack_name = dest;
			*((int *)ptr) = ip_list[i].m_ip;
			ptr += sizeof(int);
			*((int *)ptr) = ip_list[i].m_port;
			ptr += sizeof(int);
		}
		ret =  p_com_interface->SendReliableData(dest,buf,data_length);
		TRACE("Kazaa Controller: Send message \"Source_Supernodes_Reply\" to %s\n",dest);
		p_dlg->m_supernode_distributer_page.RefreshSuperNodeListWithRack(dest,ip_list);
	}
	/*
	else //we ran out of unused supernodes, send back a message to the rack to request supernodes later
	{
		ret = SendControllerGenericMessage(dest, KazaaControllerHeader::Source_Ran_Out_Of_Supernodes);
		TRACE("Kazaa Controller: Sent message \"Source_Ran_Out_Of_Supernodes\" to %s\n", dest);
	}
	*/
	delete [] buf;
	return ret;
}

//
//
//
/*
bool SupernodeDistributerDll::SendKazaaNum(char* dest)
{
	UINT buf_size = sizeof(KazaaControllerHeader) + sizeof(UINT);
	byte* buf = new byte[buf_size];
	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Source_Number_Of_Kazaa_To_Run;
	header->size = buf_size - sizeof(KazaaControllerHeader);

	UINT* num_to_run = (UINT*)&buf[sizeof(KazaaControllerHeader)];
	bool found = false;
	bool ret;
	UINT i;
	for(i=0;i<v_rack_list.size();i++)
	{
		if(strcmp(v_rack_list[i].m_rack_name.c_str(),dest)==0)
		{
			found = true;
			break;
		}
	}
	if(found)
	{
		*num_to_run = v_rack_list[i].m_num_kazaa;
		ret = p_com_interface->SendReliableData(dest,buf,buf_size);
		TRACE("Kazaa Controller: Sent message \"Source_Number_Of_Kazaa_To_Run\" to %s\n",dest);
	}
	else
		TRACE("Kazaa Controller: Error occurred in SendKazaaNum(): Coudn't find the rack in v_rack_list");
	delete [] buf;
	return ret;
}
*/
//
//
//
void SupernodeDistributerDll::AddNodeToRackList(char* rack_name)
{
	bool found = false;
	for(unsigned int i=0;i<v_rack_list.size();i++)
	{
		if(strcmp(rack_name,v_rack_list[i].m_rack_name.c_str())==0)
		{
			found = true;
			break;
		}
	}
	if(!found)
	{
		RackSuperNodes node;
		node.m_rack_name = rack_name;
		v_rack_list.push_back(node);
		//Update rack list in the dialog
		p_dlg->m_racks_supernodes_page.AddRack(node);
	}
}

//
//
//
/*
void SupernodeDistributerDll::OnEditKazaaNumber(char* rack, int num)
{
	for(unsigned int i=0; i<v_rack_list.size();i++)
	{
		if(strcmp(v_rack_list[i].m_rack_name.c_str(), rack)==0)
		{
			if(v_rack_list[i].m_num_kazaa != num)
			{
				v_rack_list[i].m_num_kazaa = num;
				//Send the new number of kazaa to the rack here
				SendKazaaNum(rack);
			}
		}
	}
	//save current setting
	SaveRacksMaxKazaaNumber();
}
*/
//
//
//
bool SupernodeDistributerDll::SendControllerGenericMessage(char* dest, KazaaControllerHeader::op_code op_code)
{
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = op_code;
	header->size = 0;
	return p_com_interface->SendReliableData(dest,buf,sizeof(KazaaControllerHeader));
}

//
//
//
void SupernodeDistributerDll::ReceivedRemoteSupernodeList(char* source_name, vector<IPAndPort>& supernodes, UINT kazaa_running)
{
	UINT i,j;
	//check for duplicated supernodes
	vector<IPAndPort> duplicated_supernodes;
	for(i=0; i<supernodes.size();i++)
	{
		int occurance = 0;
		for(j=0; j<supernodes.size();j++)
		{
			if(supernodes[i] == supernodes[j])
				occurance++;
		}
		if(occurance > 1)
			duplicated_supernodes.push_back(supernodes[i]);
	}
	
	for(i=0;i<v_rack_list.size();i++)
	{
		if(strcmp(v_rack_list[i].m_rack_name.c_str(), source_name)==0)
		{
			v_rack_list[i].m_num_kazaa_running = kazaa_running;
			vector<IPAndPort> old_list = v_rack_list[i].v_supernodes;
			//re-assign the old up time to the new supernodes
			for(UINT k=0; k<supernodes.size();k++)
			{
				for(UINT m=0; m<old_list.size();m++)
				{
					if(supernodes[k]==old_list[m])
					{
						supernodes[k].m_up_since = old_list[m].m_up_since;
						break;
					}
				}
			}

			v_rack_list[i].v_supernodes = supernodes;
			//for each supernode in the old list, if it doesn't exist anymore in the new list, remove it from the v_supernode_list
			vector<IPAndPort>::iterator old_iter = old_list.begin();
			while(old_iter!=old_list.end()) //remove supernodes from old_list which appear in supernodes
			{
				bool found = false;
				vector<IPAndPort>::iterator new_iter = supernodes.begin();
				while(new_iter!=supernodes.end())
				{
					if(*(new_iter) == *(old_iter))
					{
						found = true;
						break;
					}
					new_iter++;
				}
				if(found)
				{
					old_list.erase(old_iter); //getting rid of the duplicated ones
					supernodes.erase(new_iter);
				}
				else
					old_iter++;
			}
			if(old_list.size()>0)
			{
				for(j=0;j<old_list.size();j++)
				{
					//remove them from v_supernode_list
					IPAddress ip_address;
					ip_address.m_ip = old_list[j].m_ip;
					ip_address.m_port = old_list[j].m_port;
					vector<IPAddress>::iterator result = NULL;
					result = find(v_supernode_list.begin(),v_supernode_list.end(),ip_address);
					if(result != v_supernode_list.end() && result != NULL)
					{
						v_supernode_list.erase(result);
					}
				}
				//remove the supernode from the dialog
				p_dlg->m_supernode_distributer_page.RemoveSuperNodes(old_list);
			}
			if(supernodes.size()>0) //add news supernodes to v_supernode_list
			{
				//vector<IPAddress> temp_list = v_temp_supernode_list;
				//sort(temp_list.begin(),temp_list.end());

				for(j=0;j<supernodes.size();j++)
				{
					IPAddress ip_address;
					ip_address.m_ip = supernodes[j].m_ip;
					ip_address.m_port = supernodes[j].m_port;
					ip_address.m_rack_name = source_name;

					bool existed = binary_search(v_supernode_list.begin(),v_supernode_list.end(),ip_address);
									//|| binary_search(temp_list.begin(),temp_list.end(),ip_address);

					if(!existed)
					{
						//insertion sort
						vector<IPAddress>::iterator location;
						location = lower_bound(v_supernode_list.begin(),v_supernode_list.end(), ip_address);
						v_supernode_list.insert(location,ip_address);

						//add new supernode to the dialog
						p_dlg->m_supernode_distributer_page.AddSuperNode(ip_address,"*Unknown*",source_name);
					}
				}
			}
		}
	}
	//remove the duplicated supernodes from the main list and from the dialog
	for(i=0; i<duplicated_supernodes.size(); i++)
	{
		IPAddress old_ipaddress(duplicated_supernodes[i]);
		vector<IPAddress>::iterator location = NULL;
		location = find(v_supernode_list.begin(),v_supernode_list.end(),old_ipaddress);
		if(location != NULL && location != v_supernode_list.end())
		{
			v_supernode_list.erase(location);
			//remove the supernode from the dialog
			p_dlg->m_supernode_distributer_page.RemoveSuperNode(duplicated_supernodes[i]);
		}
		
	}
	//send renew supernodes for the duplicated supernodes
	for(i=0; i<duplicated_supernodes.size(); i++)
	{
		ReceivedRequestForNewSupernode(source_name, duplicated_supernodes[i], duplicated_supernodes[i]);
	}
}

//
//
//
void SupernodeDistributerDll::OnTimer(int nIDEvent)
{
	switch(nIDEvent)
	{
	case 1: //importing supernodes from registry
		{
			ImportSuperNodeList();
			break;
		}
	case 2: //reconnecting kazaa
		{
			ReconnectKazaa();
			break;
		}
	case 3:	//Send Init to all racks
		{
			//SendControllerGenericMessage("KAZAA",KazaaControllerHeader::Source_Init);
			TRACE("Kazaa Controller: Sent message \"Source_Init\" to All Kazaa Racks\n");
			SendSharedFoldersToAllRacks(p_dlg->m_supernode_distributer_page.GetSharedFolderList());
			//Getting kazaa path from the registry
			GetKazaaPathFromRegistry();
			//check to see if kazaa is running
			IsKazaaRunning();

			break;
		}
	case 4: //check for idle sockets
		{
			KillAllIdleSockets();
			break;
		}
	case 5: //Maintenance timer
		{
			OnMaintenanceTimer();
			break;
		}
	case 6:	//first time Init to all racks
		{
			//SendControllerGenericMessage("KAZAA",KazaaControllerHeader::Source_Init);
			//TRACE("Kazaa Controller: Sent message \"Source_Init\" to All Kazaa Racks\n");
			SendSharedFoldersToAllRacks(p_dlg->m_supernode_distributer_page.GetSharedFolderList());
			p_dlg->m_supernode_distributer_page.KillTimer(6);			
			break;
		}
	case 7:
		{
			int offset = m_second_count % 600;
			bool did_send = false;
			for(UINT i=0;i<v_rack_list.size();i++)
			{
				if(v_rack_list[i].m_ping_offset == offset)
				{
					did_send = true;
					char rack[32];
					strcpy(rack, v_rack_list[i].m_rack_name.c_str());
					SendControllerGenericMessage(rack,KazaaControllerHeader::Source_Init);
				}
				else if(did_send)
					break;
			}
			m_second_count++;
			break;
		}
	case 8: //resetting all supernodes
		{
			ResetAllSupernodes();
			SendControllerGenericMessage("KAZAA",KazaaControllerHeader::Source_Init);
			break;
		}
	}	
}

//
//
//
void SupernodeDistributerDll::OnStopAndKillKazaa(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		char* rack_name	= new char[selected_racks[i].GetLength()+1];
		strcpy(rack_name, selected_racks[i]);
		SendControllerGenericMessage(rack_name,KazaaControllerHeader::Source_Kill_All_Kazaa);
		RemoveAllSupernodesAssociatedWithRack(rack_name);
		delete [] rack_name;
	}
}

//
//
//
void SupernodeDistributerDll::OnResumeKazaa(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		char* rack_name	= new char[selected_racks[i].GetLength()+1];
		strcpy(rack_name, selected_racks[i]);
		SendControllerGenericMessage(rack_name,KazaaControllerHeader::Source_Launch_All_Kazaa);
		delete [] rack_name;
	}
}

//
//
//
void SupernodeDistributerDll::OnRestartKazaa(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		char* rack_name	= new char[selected_racks[i].GetLength()+1];
		strcpy(rack_name, selected_racks[i]);
		SendControllerGenericMessage(rack_name,KazaaControllerHeader::Source_Restart_All_Kazaa);
		RemoveAllSupernodesAssociatedWithRack(rack_name);
		delete [] rack_name;
	}
}

//
//
//
void SupernodeDistributerDll::OnGetRemoteSupernodes(vector<CString>& selected_racks)
{
	for(UINT i=0; i<selected_racks.size(); i++)
	{
		char* rack_name	= new char[selected_racks[i].GetLength()+1];
		strcpy(rack_name, selected_racks[i]);
		SendControllerGenericMessage(rack_name,KazaaControllerHeader::Source_Init);
		delete [] rack_name;
	}
}

//
//
//
void SupernodeDistributerDll::RemoveAllSupernodesAssociatedWithRack(char* rack_name)
{
	UINT i,j;
	for(i=0;i<v_rack_list.size();i++)	//remove them from v_rack_list
	{
		bool found = false;
		if(strcmp(v_rack_list[i].m_rack_name.c_str(),rack_name)==0)
		{
			found = true;

			for(j=0;j<v_rack_list[i].v_supernodes.size();j++)
			{
				//reset them from v_supernode_list
				IPAddress ip_address;
				ip_address.m_ip = v_rack_list[i].v_supernodes[j].m_ip;
				ip_address.m_port = v_rack_list[i].v_supernodes[j].m_port;
				vector<IPAddress>::iterator result = NULL;
				result = find(v_supernode_list.begin(),v_supernode_list.end(),ip_address);
				if(result != v_supernode_list.end() && result != NULL)
				{
					result->m_rack_name = "NULL";
				}
				//update rack name of the supernode from the dialog
				p_dlg->m_supernode_distributer_page.RefreshSuperNodeListWithRack("NULL",ip_address.m_ip);
			}
			v_rack_list[i].v_supernodes.clear();
			break;
		}
	}

}

//
//
//
int SupernodeDistributerDll::GetIntIPFromStringIP(string ip)
{
	int ip_int = 0;
	if(ip.length()==0)
		return ip_int;
	
	int ip1,ip2,ip3,ip4;
	sscanf(ip.c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
	return ip_int;
}

//
//
//
string SupernodeDistributerDll::GetIPStringFromIPInt(int ip_int)
{
	string ip;
	char ip_str[16];
	sprintf(ip_str,"%u.%u.%u.%u",(ip_int>>0)&0xFF,(ip_int>>8)&0xFF,(ip_int>>16)&0xFF,(ip_int>>24)&0xFF);
	ip = ip_str;
	return ip;
}

//
//
//
void SupernodeDistributerDll::ReceivedIPs(vector<string>& ips)
{
	//Import the ip list from name server to our rack list
	for(UINT i=0; i<ips.size(); i++)
	{
		bool found = false;
		//check if the ip is already on our list
		for(UINT j=0; j<v_rack_list.size(); j++)
		{
			if(strcmp(ips[i].c_str(), v_rack_list[j].m_rack_name.c_str())==0)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			RackSuperNodes rack;
			rack.m_rack_name = ips[i];
			v_rack_list.push_back(rack);
			p_dlg->m_racks_supernodes_page.AddRack(rack);
		}
	}
}
/*
//
//
//
void SupernodeDistributerDll::SaveRacksMaxKazaaNumber()
{
	if(v_rack_list.size()>0)
	{
		CStdioFile file;
		if(file.Open("racksmaxkazaanumber.dat", CFile::modeWrite|CFile::modeCreate|CFile::typeText) != 0)
		{
			for(UINT i=0; i<v_rack_list.size(); i++)
			{
				file.WriteString(v_rack_list[i].m_rack_name.c_str());
				file.WriteString(" ");
				char num[8];
				sprintf(num, "%d", v_rack_list[i].m_num_kazaa);
				file.WriteString(num);
				file.WriteString("\n");
			}
			file.Close();
		}
	}
}

//
//
//

void SupernodeDistributerDll::ReadRacksMaxKazaaNumber()
{
	CStdioFile file;
	if(file.Open("racksmaxkazaanumber.dat", CFile::modeRead|CFile::typeText) != 0)
	{
		CString read_in;
		while(file.ReadString(read_in))
		{
			char rack[256];
			int num = 0;
			sscanf(read_in, "%s %d", rack, &num);
			for(UINT i=0;i<v_rack_list.size();i++)
			{
				if(strcmp(v_rack_list[i].m_rack_name.c_str(), rack)==0)
				{
					v_rack_list[i].m_num_kazaa = num;
					p_dlg->m_racks_supernodes_page.UpdateRackMaxKazaaNum(v_rack_list[i]);
					break;
				}
			}
		}
		file.Close();
	}
}
*/

//
//
//
void SupernodeDistributerDll::GetKazaaPathFromRegistry()
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
	char* ptr = strstr(m_kazaa_path.c_str(), ".exe");
	if(ptr == NULL)
	{
#ifdef KAZAA_KPP
		m_kazaa_path += "\\kpp.exe"; //K++ edition
#else
		m_kazaa_path += "\\Kazaa.exe";
#endif
	}
}

//
//
//
void SupernodeDistributerDll::IsKazaaRunning()
{
	//look for kazaa process
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
#ifdef KAZAA_KPP
			if(stricmp("KazaaLite.kpp",name)==0)//if process is named kazaa, K++ edition
#else
			if(stricmp("Kazaa.exe",name)==0)//if process is named kazaa
#endif
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
#ifdef KAZAA_KPP			
		CString kpp_filename = m_kazaa_path.c_str();
		int index = kpp_filename.ReverseFind('\\');
		kpp_filename.Delete(index, kpp_filename.GetLength()-index);
		kpp_filename+="\\KazaaLite.kpp";
#endif
		strcpy(command, m_kazaa_path.c_str());
#ifdef KAZAA_KPP
		strcat(command, " ");
		strcat(command, kpp_filename); //K++ edition
#endif
		strcat(command, " /SYSTRAY");
		lpszCommand =  command;
//		string run_in_dir = GetFreeRunInDir();
//		lpszRunIn = run_in_dir.c_str();

		PROCESS_INFORMATION pi;
		STARTUPINFOA si = {sizeof si};
		BOOL bret = CreateProcess(
			lpszExePath,
			lpszCommand,
			NULL,
			NULL,
			FALSE,
			BELOW_NORMAL_PRIORITY_CLASS,
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
bool SupernodeDistributerDll::SendAllSupernodesToRack(char* dest)
{
	bool ret = false;
	byte* buf;
	UINT data_length = sizeof(KazaaControllerHeader)
						+sizeof(UINT) //number of IPs
						+2*sizeof(int)*(UINT)v_supernode_list.size(); //IP + port
	buf = new byte[data_length]; 

	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Source_Supernodes_Reply;
	header->size = data_length - sizeof(KazaaControllerHeader);

	UINT *num = (UINT *)&buf[sizeof(KazaaControllerHeader)];
	*num= (UINT)v_supernode_list.size();
	byte* ptr = &buf[sizeof(KazaaControllerHeader)+sizeof(UINT)];

	for(UINT i=0;i<v_supernode_list.size();i++)
	{
		*((int *)ptr) = v_supernode_list[i].m_ip;
		ptr += sizeof(int);
		*((int *)ptr) = v_supernode_list[i].m_port;
		ptr += sizeof(int);
	}
	ret =  p_com_interface->SendReliableData(dest,buf,data_length);
	TRACE("Kazaa Controller: Send message \"Source_Supernodes_Reply\" to %s\n",dest);
	
	delete [] buf;
	return ret;
}

//
//
//
void SupernodeDistributerDll::SendSharedFoldersToAllRacks(vector<CString>folders)
{
	int buf_len = sizeof(KazaaControllerHeader)+sizeof(UINT);
	for(UINT i=0;i<folders.size();i++)
	{
		buf_len += folders[i].GetLength()+1;
	}
	byte* buf = new byte[buf_len];
	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Source_Shared_Folders;
	header->size = buf_len - sizeof(KazaaControllerHeader);

	byte* ptr = &buf[sizeof(KazaaControllerHeader)];
	*(UINT*)ptr = (UINT)folders.size();
	ptr += sizeof(UINT);
	for(UINT i=0;i<folders.size();i++)
	{
		strcpy((char*)ptr, folders[i]);
		ptr+=folders[i].GetLength()+1;
	}

	p_com_interface->SendReliableData("KAZAA",buf,buf_len);
	delete [] buf;
}

//
//
//
void SupernodeDistributerDll::CalculatePingOffset()
{
	float offset = ((float)600 / (float)v_rack_list.size());
	float interval = offset;
	for(UINT i=0; i<v_rack_list.size();i++)
	{
		if(interval <= 600 - 1)
			v_rack_list[i].m_ping_offset = (int)interval;
		else
			v_rack_list[i].m_ping_offset = 600 - 1;
		interval+=offset;
	}
}

//
//
//
void SupernodeDistributerDll::ResetAllSupernodes(void)
{
	DeleteFile("supernodes.dat");
	v_supernode_list.clear();
	v_temp_supernode_list.clear();
}
