#include "StdAfx.h"
#include "SupernodeControllerDll.h"
#include "DllInterface.h"
#include "IPAddress.h"
#include "VectorIPAddress.h"
#include "..\NameServer\NameServerInterface.h"
#include "..\TKSyncher\TKSyncherInterface.h"
#include "imagehlp.h"
#include "osrng.h"	// for CryptoPP

inline bool UpTimeSort(const IPAddress& ip1, const IPAddress& ip2)
{
	if(ip1.m_connecting_time < ip2.m_connecting_time)
		return true;
	else
		return false;
}


SupernodeControllerDll::SupernodeControllerDll(void)
{
}

//
//
//
SupernodeControllerDll::~SupernodeControllerDll(void)
{
}

//
//
//
void SupernodeControllerDll::DllInitialize()
{
	srand( (unsigned)time( NULL ) );
}

//
//
//
void SupernodeControllerDll::DllUnInitialize()
{
	/*
	for(UINT i=0; i<v_main_supernode_list.size();i++)
	{
		delete v_main_supernode_list[i];
	}
	*/
}

//
//
//
void SupernodeControllerDll::DllStart()
{
	m_second_count = 0;
	m_max_same_supernode_assigned = 0;
	m_dlg.Create(IDD_MAIN_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);

	//Create and register the com interface
	m_com_interface.Register(this, DllInterface::GetAppID().m_app_id);


	vector<string> ips;
	NameServerInterface ns;
	ns.RequestIP("KAZAA", ips); //request all ips of kazaa racks
	ReceivedKazaaRackIPs(ips); //process the ip list
	ips.clear();
	
	ns.RequestIP("SUPERNODE-COLLECTOR", ips); //request all ips of kazaa racks
	ReceivedSupernodeCollectorIPs(ips); //process the ip list


	//Set timer
	m_dlg.SetTimer(1, 5*60*1000, 0); //send init message to all supernode collectors every 5 mins
	m_dlg.SetTimer(2, 60*1000, 0); //request new supernodes from collectors every 1 min
	m_dlg.SetTimer(3, 1000, 0); //1 second timer
	m_dlg.SetTimer(4, 5000, 0); //5 second timer
	m_dlg.SetTimer(5, 60*1000, 0);

	SetInitMsgOffsetForKazaaRacks();
	OnTimer(1); //send init msg to all collectors
	//SendGenericMessage("KAZAA",KazaaControllerHeader::Source_Init); //send init msg to all kazaa racks
	CString msg;
	CTime time = CTime::GetCurrentTime();
	msg = "Started running since ";
	msg += time.Format("%Y-%m-%d %H:%M:%S");
	m_dlg.Log(msg);
	p_available_supernode = NULL;

	//send out the addition shared folders if any
	ReadAndSendAdditionalSharedFolders();
}

//
//
//
void SupernodeControllerDll::SetInitMsgOffsetForKazaaRacks()
{
	//send init msg to all kazaa launcher racks every 30 mins
	float offset = ((float)DEFAULT_PING_TIME / (float)v_rack_list.size());
	float interval = offset;
	for(UINT i=0; i<v_rack_list.size();i++)
	{
		if(interval <= DEFAULT_PING_TIME - 1)
			v_rack_list[i].m_ping_offset = (int)interval;
		else
			v_rack_list[i].m_ping_offset = DEFAULT_PING_TIME - 1;
		interval+=offset;
	}
}

//
//
//
void SupernodeControllerDll::ReceivedSupernodeCollectorIPs(vector<string>& ips)
{
	//Import the ip list from name server to our collector list
	for(UINT i=0; i<ips.size(); i++)
	{
		bool found = false;
		//check if the ip is already on our list
		for(UINT j=0; j<v_collectors.size(); j++)
		{
			if(strcmp(ips[i].c_str(), v_collectors[j].m_ip)==0)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			SupernodeCollector collector;
			strcpy(collector.m_ip, ips[i].c_str());
			v_collectors.push_back(collector);
		}
	}
}

//
//
//
void SupernodeControllerDll::ReceivedKazaaRackIPs(vector<string>& ips)
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
			m_dlg.m_rack_status_dlg.AddRack(rack.m_rack_name.c_str());
		}
	}
}

//
//
//
void SupernodeControllerDll::DllShowGUI()
{
	m_dlg.EnableWindow();
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void SupernodeControllerDll::SendInitMsgToAll()
{
	SendGenericMessage("KAZAA",KazaaControllerHeader::Source_Init);
}

//
//
//
void SupernodeControllerDll::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
		case 1:
		{
			SendGenericMessage("KAZAA-COLLECTOR", KazaaControllerHeader::Source_Init);
			break;
		}
		case 2:
		{
			SendGenericMessage("KAZAA-COLLECTOR", KazaaControllerHeader::Request_New_Supernodes_List);
			CTime current_time = CTime::GetCurrentTime();
			if(current_time.GetHour() == 4 && current_time.GetMinute() == 0) //reset
				ResetAll();
			break;
		}
		case 3: //one second timer
		{
			int offset = m_second_count % DEFAULT_PING_TIME;
			bool did_send = false;
			for(UINT i=0;i<v_rack_list.size();i++)
			{
				if(v_rack_list[i].m_ping_offset == offset)
				{
					did_send = true;
					char rack[32];
					strcpy(rack, v_rack_list[i].m_rack_name.c_str());
					SendGenericMessage(rack,KazaaControllerHeader::Source_Init);
				}
				else if(did_send)
					break;
			}
			m_second_count++;
			break;
		}
		case 4:
		{
			int up_collectors = GetNumUpCollectors();
			int supernodes_used = 0;
			int launcher_supernodes = 0;
			GetNumConsumedSupernodes(supernodes_used, launcher_supernodes);
			m_dlg.UpdateStatus((int)v_collectors.size(),up_collectors,(int)hs_main_supernode_list.size(),supernodes_used,(int)v_rack_list.size(),
				launcher_supernodes);
			break;
		}
		case 5:
		{
			m_dlg.KillTimer(5);
			m_dlg.SetTimer(5,24*60*60*1000,0);
			SaveUserNames();
			ReadAndSendAdditionalSharedFolders();
			break;
		}
	}
}

//
//
//
int SupernodeControllerDll::GetNumUpCollectors()
{
	int num=0;
	for(UINT i=0; i<v_collectors.size(); i++)
	{
		CTimeSpan ts =  CTime::GetCurrentTime() - v_collectors[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() <= TIMEOUT)
			num++;
	}
	return num;
}

//
//
//
void SupernodeControllerDll::GetNumConsumedSupernodes(int& greater_0, int& launcher_supernodes)
{
	hash_set<IPAddress>::const_iterator iter = hs_main_supernode_list.begin();
	while(iter != hs_main_supernode_list.end())
	{
		if(iter->v_assigned_racks.size() > 0)
			greater_0++;
		iter++;
	}
	for(UINT i=0;i<v_rack_list.size();i++)
	{
		launcher_supernodes += (int)v_rack_list[i].v_supernodes.size();
	}
}

//
//
//
bool SupernodeControllerDll::SendGenericMessage(char* dest, KazaaControllerHeader::op_code op_code)
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
void SupernodeControllerDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote SD Dll data
	KazaaControllerHeader* header = (KazaaControllerHeader*)data;
	byte *pData=(byte *)data;
	pData+=sizeof(KazaaControllerHeader);
	
	switch(header->op)
	{
		case(KazaaControllerHeader::Dest_Init_Response):
		{
			//Add the node to the rack list
			AddNodeToRackList(source_name);
			
			//Request remote's current supernodes list
			SendGenericMessage(source_name, KazaaControllerHeader::Source_Request_Current_Supernodes_list);
			//Send Number of kazaa to run for this rack
			//SendKazaaNum(source_name);

			break;
		}
		case(KazaaControllerHeader::Dest_Supernode_Request):
		{
			if(IsValidRack(source_name))
			{
				UINT* num = (UINT*)pData;
				//m_dlg.m_request_new_supernode_count++;

				//Send a list of supernodes to the rack
				if(*num < 200) //not allowing to request more than 200 supernodes
					SendSupernodesToRack(source_name,*num);
			}
			break;
		}
		case (KazaaControllerHeader::Dest_Current_Supernodes_list):
		{
			if(IsValidRack(source_name))
			{
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
				m_dlg.m_remote_supernode_list_count++;
				
				//Update our rack list's supernodes
				ReceivedRemoteSupernodeList(source_name, supernodes, *kazaa_num);
			}
			break;
		}
		case (KazaaControllerHeader::Dest_Request_New_Supernode):
		{
			if(IsValidRack(source_name))
			{
				int* old_ip = (int*)pData;
				pData+=sizeof(int);
				int* old_port = (int*)pData;
				IPAndPort old_ip_port;
				old_ip_port.m_ip = *old_ip;
				old_ip_port.m_port = *old_port;
				string supernode = GetIPStringFromIPInt(*old_ip);

				m_dlg.m_request_new_supernode_count++;

				ReceivedRequestForNewSupernode(source_name,old_ip_port,old_ip_port);
			}
			break;
		}
		case (KazaaControllerHeader::Dest_Check_New_Supernode):
		{
			if(IsValidRack(source_name))
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

				m_dlg.m_check_new_supernode_count++;

				ReceivedCheckNewSupernode(source_name, old_ip_port, new_ip_port);
			}
			break;
		}
		case (KazaaControllerHeader::Dest_Remove_Supernode):
		{
			if(IsValidRack(source_name))
			{
				int* ip = (int*)pData;
				pData+=sizeof(int);
				int* port = (int*)pData;
				string old_supernode = GetIPStringFromIPInt(*ip);

				m_dlg.m_remove_supernode_count++;

				TotallyRemoveSuperNode(source_name, *ip, *port);
			}
			break;
		}
		/*
		case (KazaaControllerHeader::Request_All_Supernodes):
		{
			TRACE("Kazaa Controller: Received message \"Request_All_Supernodes\" from %s\n",source_name);
            SendAllSupernodesToRack(source_name);
		}
		*/
		/** op code for the collectors **/
		case (KazaaControllerHeader::Collector_Init_Response):
		{

			//Add the node to the rack list
			if(AddNodeToCollector(source_name)==false)
			{
				//Request collector all supernodes
				SendGenericMessage(source_name, KazaaControllerHeader::Request_Whole_Supernodes_List);
			}
			break;
		}
		case (KazaaControllerHeader::Whole_Supernodes_List):
		{
			VectorIPAddress ips;
			ips.ReadFromBuffer((char*)pData);


			ReceivedCollectorSupernodes(ips.v_ip_addresses);
			break;
		}
		case (KazaaControllerHeader::New_Supernodes_List):
		{
			VectorIPAddress ips;
			ips.ReadFromBuffer((char*)pData);
			
			ReceivedCollectorSupernodes(ips.v_ip_addresses);
			break;
		}
	}
}

//
//
//
int SupernodeControllerDll::GetIntIPFromStringIP(string ip)
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
string SupernodeControllerDll::GetIPStringFromIPInt(int ip_int)
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
void SupernodeControllerDll::AddNodeToRackList(char* rack_name)
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
		m_dlg.m_rack_status_dlg.AddRack(rack_name);
	}
}

//
//
//	return true if the collector existed and responsed within TIMEOUT before
bool SupernodeControllerDll::AddNodeToCollector(char* rack_name)
{
//	bool found = false;
	for(unsigned int i=0;i<v_collectors.size();i++)
	{
		if(strcmp(rack_name,v_collectors[i].m_ip)==0)
		{
			CTimeSpan ts =  CTime::GetCurrentTime() - v_collectors[i].m_last_init_response_time;
			v_collectors[i].m_last_init_response_time = CTime::GetCurrentTime();
			if(ts.GetTotalSeconds() > TIMEOUT)
				return false;
			else
				return true;
		}
	}

	SupernodeCollector node;
	strcpy(node.m_ip,rack_name);
	node.m_last_init_response_time = CTime::GetCurrentTime();
	v_collectors.push_back(node);
	
	//Update rack list in the dialog
//	m_dlg.m_rack_status_dlg.AddRack(rack_name);
	
	return false;
}
//
//
//
bool SupernodeControllerDll::SendSupernodesToRack(char* dest, UINT num_request)
{

	bool ret = false;
	if(hs_main_supernode_list.size() > 0 && p_available_supernode != NULL)
	{
		vector<IPAndPort> ip_list;
		UINT i=0;
		/*
		for(i=0; i<num_request; i++)
		{
			while(p_available_supernode->IsAssignedToThisRackAlready(dest))
			{
				p_available_supernode++;
				if(p_available_supernode == hs_main_supernode_list.end())
					p_available_supernode = hs_main_supernode_list.begin();
			}
			IPAndPort ip;
			ip.m_ip = p_available_supernode->m_ip;
			ip.m_port = p_available_supernode->m_port;
			ip_list.push_back(ip);
			p_available_supernode->v_assigned_racks.push_back(dest);
			if(p_available_supernode->v_assigned_racks.size() > m_max_same_supernode_assigned)
				m_max_same_supernode_assigned = (UINT)p_available_supernode->v_assigned_racks.size();

			p_available_supernode++;
			if(p_available_supernode == hs_main_supernode_list.end())
				p_available_supernode = hs_main_supernode_list.begin();
		}
		*/
		
		CryptoPP::AutoSeededRandomPool rng(true, 32);
		hash_set<int>random_used;
		for(i=0; i<num_request;i++)
		{

			while(true) //kludge
			{
				if(random_used.size() == hs_main_supernode_list.size())
					break;
				hash_set<IPAddress>::iterator iter = hs_main_supernode_list.begin();
				unsigned long index = rng.GenerateWord32(0, (unsigned long)hs_main_supernode_list.size()-1);
				//index = rand()%(int)hs_main_supernode_list.size();
				
				pair< hash_set<int>::iterator, bool > pr;
				pr = random_used.insert(index);
				while(pr.second == false)
				{
					index = rng.GenerateWord32(0, (unsigned long)hs_main_supernode_list.size()-1);
					//index = rand()%(int)hs_main_supernode_list.size();
					pr = random_used.insert(index);
				}

				while(index > 0)
				{
					iter++;
					index--;
				}

				if(iter->IsAssignedToThisRackAlready(dest) == true)
					continue;
				else
				{
					IPAndPort ip;
					ip.m_ip = iter->m_ip;
					ip.m_port = iter->m_port;
					ip_list.push_back(ip);
					iter->v_assigned_racks.push_back(dest);
					if(iter->v_assigned_racks.size() > m_max_same_supernode_assigned)
						m_max_same_supernode_assigned = (UINT)iter->v_assigned_racks.size();
					break;
				}
			}
		}

		byte* buf;
		UINT data_length = sizeof(KazaaControllerHeader)
							+sizeof(UINT) //number of IPs
							+2*sizeof(int)* (int)ip_list.size(); //IP + port
		buf = new byte[data_length]; 

		KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
		header->op = KazaaControllerHeader::Source_Supernodes_Reply;
		header->size = data_length - sizeof(KazaaControllerHeader);

		if(ip_list.size() > 0) //send the supernodes anyway even though it might less than the number the rack requested
		{
			hash_set<IPAddress>::iterator result = NULL;
			IPAddress ip;

			UINT *num = (UINT *)&buf[sizeof(KazaaControllerHeader)];
			*num= (UINT)ip_list.size();
			byte* ptr = &buf[sizeof(KazaaControllerHeader)+sizeof(UINT)];

			for(i=0;i<v_rack_list.size();i++)
			{
				if(strcmp(v_rack_list[i].m_rack_name.c_str(),dest)==0)
				{
					for(int j=0;j<(int)ip_list.size();j++)
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
				*((int *)ptr) = ip_list[i].m_ip;
				ptr += sizeof(int);
				*((int *)ptr) = ip_list[i].m_port;
				ptr += sizeof(int);
			}
			
			ret =  m_com_interface.SendReliableData(dest,buf,data_length);
		}
		delete [] buf;
	}
	return ret;
}

//
//
//
bool SupernodeControllerDll::ReceivedRequestForNewSupernode(char* source_name, IPAndPort& old_ip, IPAndPort& current_ip, bool delete_old)
{
	IPAndPort new_ip;

	UINT i;
	if(GetSupernode(source_name, old_ip, new_ip, delete_old))
	{
		//bool found = false;
		//bool found_old = false;
		for(i=0;i<v_rack_list.size();i++)//assigning the new supernode to the rack
		{
			if(strcmp(v_rack_list[i].m_rack_name.c_str(),source_name)==0)
			{
				v_rack_list[i].ReplaceSupernode(old_ip,new_ip);
				break;
			}
		}

				/*found = true;
				
				for(j=0;j<v_rack_list[i].v_supernodes.size();j++)
				{
					if(v_rack_list[i].v_supernodes[j] == old_ip)
					{
						v_rack_list[i].v_supernodes[j] =  new_ip;
						found_old = true;
						break;
					}
				}
				if(!found_old)
					v_rack_list[i].v_supernodes.push_back(new_ip);
			}
			if(found)
				break;

		}
		*/
		//Send the new supernode along with the rack current supernode to the rack
		return SendRenewSupernode(source_name, current_ip, new_ip);
	}
	return false;
}

//
//
//
bool SupernodeControllerDll::GetSupernode(char* source_name, IPAndPort& old_ip, IPAndPort& new_ip, bool delete_old)
{
	IPAddress old_ipaddress(old_ip);
	//bool found = false;

	if(delete_old)
	{
		//delete the old supernode from the main list
		hash_set<IPAddress>::iterator hsLocation = NULL;
		hsLocation = hs_main_supernode_list.find(old_ipaddress);
		if(hsLocation != hs_main_supernode_list.end())
		{
			hsLocation->RemoveThisAssignedRack(source_name);
			if(hsLocation->v_assigned_racks.size() <= 0)
			{
				hs_main_supernode_list.erase(hsLocation);
				m_dlg.m_supernode_removed++;
			}				
		}
		/*
		else
		{
			char msg[256+1];
			sprintf(msg,"From: %s, GetSupernode(...) - Couldn't find supernode", source_name);
			m_dlg.Log(msg);
		}
		*/
	}
	
	if(hs_main_supernode_list.size() > 0)
	{
		/*
		while(p_available_supernode->IsAssignedToThisRackAlready(source_name))
		{
			p_available_supernode++;
			if(p_available_supernode == hs_main_supernode_list.end())
				p_available_supernode = hs_main_supernode_list.begin();
		}
		new_ip.m_ip = p_available_supernode->m_ip;
		new_ip.m_port = p_available_supernode->m_port;
		p_available_supernode->v_assigned_racks.push_back(source_name);
		if(p_available_supernode->v_assigned_racks.size() > m_max_same_supernode_assigned)
				m_max_same_supernode_assigned = (UINT)p_available_supernode->v_assigned_racks.size();

		p_available_supernode++;
		if(p_available_supernode == hs_main_supernode_list.end())
			p_available_supernode = hs_main_supernode_list.begin();
		*/
		CryptoPP::AutoSeededRandomPool rng(true, 32);
		hash_set<int>random_used;
		while(true)
		{
			if(random_used.size() == hs_main_supernode_list.size())
				break;
			hash_set<IPAddress>::iterator iter = hs_main_supernode_list.begin();
			unsigned long index = rng.GenerateWord32(0, (unsigned long)hs_main_supernode_list.size()-1);
			//index = rand()%(int)hs_main_supernode_list.size();

			pair< hash_set<int>::iterator, bool > pr;
			pr = random_used.insert(index);
			while(pr.second == false)
			{
				index = rng.GenerateWord32(0, (unsigned long)hs_main_supernode_list.size()-1);
				//index = rand()%(int)hs_main_supernode_list.size();
				pr = random_used.insert(index);
			}

			while(index > 0)
			{
				iter++;
				index--;
			}
			if(iter->IsAssignedToThisRackAlready(source_name) == true)
				continue;
			else
			{
				new_ip.m_ip = iter->m_ip;
				new_ip.m_port = iter->m_port;
				iter->v_assigned_racks.push_back(source_name);
				if(iter->v_assigned_racks.size() > m_max_same_supernode_assigned)
					m_max_same_supernode_assigned = (UINT)iter->v_assigned_racks.size();
				break;
			}
		}
	}
	return true;
}

//
//
//
bool SupernodeControllerDll::SendRenewSupernode(char* dest, IPAndPort& old_ip, IPAndPort& new_ip)
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

	ret = m_com_interface.SendReliableData(dest, buf, buf_length);
	delete [] buf;

	return ret;
}

//
//
//
bool SupernodeControllerDll::SendSupernodeIsOk(char* dest, IPAndPort& ip)
{
	bool ret = false;
	UINT buf_length = sizeof(KazaaControllerHeader)+sizeof(int)*2;
	byte* buf = new byte[buf_length];

	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Current_Supernode_Is_Ok;
	header->size = buf_length - sizeof(KazaaControllerHeader);

	byte* ptr = &buf[sizeof(KazaaControllerHeader)];
	*((int*)ptr) = ip.m_ip;
	ptr+=sizeof(int);
	*((int*)ptr) = ip.m_port;
	ptr+=sizeof(int);

	ret = m_com_interface.SendReliableData(dest, buf, buf_length);
	delete [] buf;

	return ret;
}

//
//
//
void SupernodeControllerDll::TotallyRemoveSuperNode(char* source, int ip, int port)
{
	// Find the item that we are removing
	IPAddress ip_address;
	ip_address.m_ip = ip;
	ip_address.m_port = port;

	//delete the old supernode from the main list
	hash_set<IPAddress>::iterator hsLocation = NULL;
	hsLocation = hs_main_supernode_list.find(ip_address);
	if(hsLocation != hs_main_supernode_list.end())
	{
		hsLocation->RemoveThisAssignedRack(source);
	}
/*	else
	{
		char msg[256+1];
		sprintf(msg,"From: %s, TotallyRemoveSuperNode(...) - Couldn't find supernode", source);
		m_dlg.Log(msg);
	}
*/
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
}

//
//
//
bool SupernodeControllerDll::ReceivedCheckNewSupernode(char* source_name, IPAndPort& old_ip_port, IPAndPort& new_ip_port)
{
	bool supernode_is_taken = false;
	IPAddress old_ipaddress(old_ip_port);
	hash_set<IPAddress>::iterator hsLocation = NULL;

	//reset the assigned flag of the old supernode from the main list
	hsLocation = hs_main_supernode_list.find(old_ipaddress);
	if(hsLocation !=  hs_main_supernode_list.end())
	{
		hsLocation->RemoveThisAssignedRack(source_name);
	}
	/*
	else
	{
		char msg[256+1];
		string ip = GetIPStringFromIPInt(old_ipaddress.m_ip);
		sprintf(msg,"From: %s, ReceivedCheckNewSupernode(...)(old supernode) - Couldn't find supernode %s", source_name,ip.c_str());
		m_dlg.Log(msg);
	}
	*/

	//look for the new one
	IPAddress new_ipaddress(new_ip_port);
	new_ipaddress.v_assigned_racks.push_back(source_name);
	pair< hash_set<IPAddress>::iterator, bool > pr;
	pr = hs_main_supernode_list.insert(new_ipaddress);
	if(pr.second == false)
	{
		if( (pr.first)->IsAssignedToThisRackAlready(source_name) )
			supernode_is_taken = true;
		else
		{
			(pr.first)->v_assigned_racks.push_back(source_name);
			if((pr.first)->v_assigned_racks.size() > m_max_same_supernode_assigned)
				m_max_same_supernode_assigned = (UINT)(pr.first)->v_assigned_racks.size();
			SendSupernodeIsOk(source_name,new_ip_port);
			for(UINT i=0; i<v_rack_list.size(); i++)
			{
				if(strcmp(v_rack_list[i].m_rack_name.c_str(), source_name)==0)
				{
					v_rack_list[i].ReplaceSupernode(old_ip_port, new_ip_port);
					break;
				}
			}
		}
	}
	else
	{
		SendSupernodeIsOk(source_name,new_ip_port);
		for(UINT i=0; i<v_rack_list.size(); i++)
		{
			if(strcmp(v_rack_list[i].m_rack_name.c_str(), source_name)==0)
			{
				v_rack_list[i].ReplaceSupernode(old_ip_port, new_ip_port);
				break;
			}
		}
	}

	if(supernode_is_taken) // we will send a renew supernode message to the rack
	{
		ReceivedRequestForNewSupernode(source_name, old_ip_port, new_ip_port, false);
		m_dlg.m_supernode_taken++;
	}
/*
	//keeping the main list less than 50,000 supernodes
	while(hs_main_supernode_list.size() > 50000)
	{
		hs_main_supernode_list.erase(hs_main_supernode_list.begin());
	}
*/	return true;
}

//
//
//
void SupernodeControllerDll::ReceivedRemoteSupernodeList(char* source_name, vector<IPAndPort>& supernodes, UINT kazaa_running)
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
	
	//remove the duplicated supernodes from supernodes
	for(j=0; j<duplicated_supernodes.size(); j++)
	{
		vector<IPAndPort>::iterator iter = supernodes.begin();
		while(iter != supernodes.end())
		{
			if(*iter == duplicated_supernodes[j])
				supernodes.erase(iter);
			else
				iter++;
		}
	}
	for(i=0;i<v_rack_list.size();i++)
	{
		if(strcmp(v_rack_list[i].m_rack_name.c_str(), source_name)==0)
		{
			v_rack_list[i].m_num_kazaa_running = kazaa_running;
			v_rack_list[i].v_supernodes = supernodes;
            break;
			/*
			vector<IPAndPort> old_list = v_rack_list[i].v_supernodes;
			v_rack_list[i].v_supernodes = supernodes;

			//for each supernode in the old list, if it doesn't exist anymore in the new list, decrement the assigned counter
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
					//supernodes.erase(new_iter);
				}
				else
					old_iter++;
			}
			if(old_list.size()>0)
			{
				for(j=0;j<old_list.size();j++)
				{
					//decrement the assigned counter
					IPAddress ip_address;
					ip_address.m_ip = old_list[j].m_ip;
					ip_address.m_port = old_list[j].m_port;
					hash_set<IPAddress>::iterator hsResult = NULL;
					hsResult = hs_main_supernode_list.find(ip_address);
					if( hsResult != hs_main_supernode_list.end())
					{
						hsResult->RemoveThisAssignedRack(source_name);
					}
					else
					{
						char msg[256+1];
						sprintf(msg,"From: %s, ReceivedRemoteSupernodeList(...)(RemoveThisAssignedRack(...) in old_list) - Couldn't find supernode",source_name);
						m_dlg.Log(msg);
					}
				}
			}
			
			if(supernodes.size()>0) //add news supernodes to v_supernode_list 
			{
				for(j=0;j<supernodes.size();j++)
				{
					IPAddress ip_address;
					ip_address.m_ip = supernodes[j].m_ip;
					ip_address.m_port = supernodes[j].m_port;
					ip_address.v_assigned_racks.push_back(source_name);
					

					pair<hash_set<IPAddress>::iterator, bool>pr;
					pr = hs_main_supernode_list.insert(ip_address);

					if(pr.second == false)
					{
						if(pr.first->IsAssignedToThisRackAlready(source_name) == false)
						{
							pr.first->v_assigned_racks.push_back(source_name);
							if((pr.first)->v_assigned_racks.size() > m_max_same_supernode_assigned)
								m_max_same_supernode_assigned = (UINT)(pr.first)->v_assigned_racks.size();
						}
					}
				}
			}
			*/
		}
	}

	//reset the assigned rack name for this rack
	hash_set<IPAddress>::iterator hs_iter = hs_main_supernode_list.begin();
	while(hs_iter != hs_main_supernode_list.end())
	{
		hs_iter->RemoveAllThisAssignedRack(source_name);
		hs_iter++;
	}
	for(j=0;j<supernodes.size();j++)
	{
		IPAddress ip(supernodes[j]);
		ip.v_assigned_racks.push_back(source_name);
		pair< hash_set<IPAddress>::iterator, bool > pr;
		pr = hs_main_supernode_list.insert(ip);
		if(pr.second == false)
		{
            (pr.first)->v_assigned_racks.push_back(source_name);
			if((pr.first)->v_assigned_racks.size() > m_max_same_supernode_assigned)
				m_max_same_supernode_assigned = (UINT)(pr.first)->v_assigned_racks.size();
		}
	}

	//send renew supernodes for the duplicated supernodes
	for(i=0; i<duplicated_supernodes.size(); i++)
	{
		ReceivedRequestForNewSupernode(source_name, duplicated_supernodes[i], duplicated_supernodes[i], false);
		m_dlg.m_duplicated_supernode++;
	}

/*	//keeping the main list less than 50,000 supernodes
	while(hs_main_supernode_list.size() > 50000)
	{
		hs_main_supernode_list.erase(hs_main_supernode_list.begin());
	}
*/
}

//
//
//
void SupernodeControllerDll::ReceivedCollectorSupernodes(vector<IPAddress>& supernodes)
{

	for(unsigned int i=0; i<supernodes.size();i++)
	{
		//pair< hash_set<IPAddress>::iterator, bool > pr;
		hs_main_supernode_list.insert(supernodes[i]);
	}
/*	//keeping the main list less than 50,000 supernodes
	while(hs_main_supernode_list.size() > 50000)
	{
		hs_main_supernode_list.erase(hs_main_supernode_list.begin());
	}
*/	if(p_available_supernode == NULL)
		p_available_supernode = hs_main_supernode_list.begin();
}

//
//
//
void SupernodeControllerDll::SaveUserNames(void)
{
	if(hs_main_supernode_list.size() > 0)
	{
		CStdioFile file;
		MakeSureDirectoryPathExists("c:\\syncher\\src\\Kazaa-Launcher\\");
		if( file.Open("c:\\syncher\\src\\Kazaa-Launcher\\namelist.txt",CFile::modeWrite|CFile::modeCreate|CFile::typeText) == 1)
		{
			//saving only 5000 usernames
			//CryptoPP::AutoSeededRandomPool rng(true, 32);
			//for(UINT i=0; i<5000; i++)
			//{
				hash_set<IPAddress>::iterator iter = hs_main_supernode_list.begin();
				//int index = 0;
				//index = rand()%(int)hs_main_supernode_list.size();
				while(iter!=hs_main_supernode_list.end())
				{
				//unsigned long index = rng.GenerateWord32(0, (unsigned long)hs_main_supernode_list.size()-1);
				//while(index > 0)
				//{
				//	iter++;
				//	index--;
				//}
					CString username = iter->m_username;
					username.MakeLower();
					if(username.GetLength() > 0 && username.Find("anonymous")==-1
						&& username.Find("kazaa")==-1)
					{
						file.WriteString(iter->m_username);
						file.WriteString("\n");
					}
					iter++;
				}
				//else if( i > 0)
				//	i--;
			//}

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
vector<IPAndPort> SupernodeControllerDll::GetSupernodesFromRackList(string& rack_name, UINT& kazaa_running)
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
/*
bool SupernodeControllerDll::IsSorted()
{
	bool main_sorted = true;
	UINT i;
	int last_ip = v_main_supernode_list[0].m_ip;
	for(i=1;i<v_main_supernode_list.size();i++)
	{
		int this_ip = v_main_supernode_list[i].m_ip;
		if(last_ip > this_ip)
		{
			main_sorted = false;
			break;
		}
		else
			last_ip = this_ip;
	}
	return main_sorted;
}
*/

//
//
//
UINT SupernodeControllerDll::GetMaxHashSetSize()
{
	return (UINT) hs_main_supernode_list.max_size();
}

//
//
//
void SupernodeControllerDll::ResetAll()
{
	hs_main_supernode_list.clear();
	for(UINT i=0; i<v_rack_list.size(); i++)
	{
		v_rack_list[i].v_supernodes.clear();
	}
	SendGenericMessage("KAZAA-COLLECTOR", KazaaControllerHeader::Request_Whole_Supernodes_List);
	SendInitMsgToAll();
	ReadAndSendAdditionalSharedFolders();
}

//
//
//
bool SupernodeControllerDll::IsValidRack(const char* rack)
{
	for(UINT j=0; j<v_rack_list.size(); j++)
	{
		if(strcmp(rack, v_rack_list[j].m_rack_name.c_str())==0)
		{
			return true;
		}
	}
	return false;
}

//
//
//
void SupernodeControllerDll::SaveSupernodes()
{
	hash_set<IPAddress>::iterator iter = hs_main_supernode_list.begin();
	CStdioFile file;
	if(file.Open("supernodes.txt",CFile::modeWrite|CFile::modeCreate|CFile::typeText|CFile::shareDenyWrite)!=0)
	{
		while(iter != hs_main_supernode_list.end())
		{
			CString output;
			char ip[16];
			memset(&ip,0,sizeof(ip));
			sprintf(ip,"%u.%u.%u.%u",(iter->m_ip>>0)&0xFF,(iter->m_ip>>8)&0xFF,(iter->m_ip>>16)&0xFF,(iter->m_ip>>24)&0xFF);
			output.AppendFormat("%s\t%d\t%s\n",ip, iter->m_port,iter->m_username);
			file.WriteString(output);
			iter++;
		}
		file.Close();
	}
}

//
//
//
void SupernodeControllerDll::ReadAndSendAdditionalSharedFolders()
{
	CStdioFile file;
	vector<CString> v_additional_shared_folders;
	
	if(file.Open("kazaa_shared_folders.txt",CFile::modeRead|CFile::typeText)!=0)
	{
		CString folder;
		while(file.ReadString(folder))
		{
			if(folder.GetLength() > 3)
				v_additional_shared_folders.push_back(folder);
		}
		file.Close();
	}
	if(v_additional_shared_folders.size() > 0)
	{
		int buf_len = sizeof(KazaaControllerHeader)+sizeof(UINT);
		for(UINT i=0;i<v_additional_shared_folders.size();i++)
		{
			buf_len += v_additional_shared_folders[i].GetLength()+1;
		}
		byte* buf = new byte[buf_len];
		KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
		header->op = KazaaControllerHeader::Source_Shared_Folders;
		header->size = buf_len - sizeof(KazaaControllerHeader);

		byte* ptr = &buf[sizeof(KazaaControllerHeader)];
		*(UINT*)ptr = (UINT)v_additional_shared_folders.size();
		ptr += sizeof(UINT);
		for(UINT i=0;i<v_additional_shared_folders.size();i++)
		{
			strcpy((char*)ptr, v_additional_shared_folders[i]);
			ptr+=v_additional_shared_folders[i].GetLength()+1;
		}

		m_com_interface.SendReliableData("KAZAA",buf,buf_len);
		delete [] buf;
	}
}