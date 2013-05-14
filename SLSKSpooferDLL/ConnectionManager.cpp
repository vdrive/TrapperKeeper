#include "stdafx.h"
#include "ConnectionManager.h"
#include "WSocket.h"
#include "SLSKController.h"
//#include "TrackInformation.h"
#include "ProjectKeywordsVector.h"
#include <fstream>
//#include "SLSKSpooferDlg.h"

ConnectionManager::ConnectionManager(void)
{
	m_server_mods = 0;
	m_parent_mods = 0;
	m_peer_mods = 0;
	ifstream fin("C:\\slsksettings.txt", ios::in);
	fin >> m_modnumber;
	fin.close();
}

ConnectionManager::~ConnectionManager(void)
{
	KillModules();
}

void ConnectionManager::AddModule(int state)
{
	
	//p_parent->Log("Adding a Module");
	// Create a new module
	switch(state)
	{
	case 1:
		{
			//TRACE("ADDING MOD 1! \n");
			ConnectionModule *mod=new ConnectionModule(1);
			mod->InitParent(this,(UINT)server_mods.size()+1);	// init parent pointer and module number
			server_mods.push_back(mod);
			m_server_mods++;
			char *tempstring = new char[10];
			sprintf(tempstring, "%d", m_server_mods);
			p_parent->setServerModStatus(tempstring);
			p_parent->Log("Adding a module 1");
			break;
		}
	case 2:
		{
			//TRACE("ADDING MOD 2! \n");
			ProjectKeywordsVector *projects = new ProjectKeywordsVector();
			for (int i = 0; i < (int)(p_parent->GetProjects()).v_projects.size(); i++)
			{
				if ((p_parent->GetProjects()).v_projects[i].m_soulseek_spoofing_enabled && i % 4 == m_modnumber)//(p_parent->GetProjects()).v_projects[i].m_project_name == "QUEENS OF THE STONE AGE-LULLABIES TO PARALYZE-2005-02-08")//m_soulseek_spoofing_enabled || (p_parent->GetProjects()).v_projects[i].m_soulseek_demand_enabled)//((p_parent->GetProjects()).v_projects[i].m_project_name == "WEEZER-TBD-2005-01-11" || (p_parent->GetProjects()).v_projects[i].m_project_name == "GARBAGE-BLEED LIKE ME-2005-01-10" || (p_parent->GetProjects()).v_projects[i].m_project_name == "LINDSAY LOHAN-SPEAK-2004-11-22" || (p_parent->GetProjects()).v_projects[i].m_project_name == "LLOYD BANKS-TBD-2004-05-20" || (p_parent->GetProjects()).v_projects[i].m_project_name == "JADAKISS-KISS OF DEATH-2004-04-14" ))
				{
					char* buf = new char[(p_parent->GetProjects()).v_projects[i].GetBufferLength()];
					(p_parent->GetProjects()).v_projects[i].WriteToBuffer(buf);
					ProjectKeywords pk;
					pk.ReadFromBuffer(buf);

			//	if (strcmp((p_parent->GetProjects()).v_projects[i].m_project_name.c_str(), "AMY GRANT-2005-01-01") == 0 || strcmp((p_parent->GetProjects()).v_projects[i].m_project_name.c_str(), "LINDSAY LOHAN-SPEAK-11-11") == 0)
					projects->v_projects.push_back(pk);
					delete [] buf;
			
				}
			}

			ConnectionModule *mod=new ConnectionModule(2, projects);
			mod->InitParent(this,(UINT)parent_mods.size()+1);	// init parent pointer and module number
			parent_mods.push_back(mod);
			m_parent_mods++;
			char *tempstring = new char[10];
			sprintf(tempstring, "%d", m_parent_mods);
			p_parent->setParentModStatus(tempstring);
			p_parent->Log("Adding a module 2");
			break;
		}
	case 3:
		{
			//TRACE("ADDING MOD 3! \n");
			ProjectKeywordsVector *projects = new ProjectKeywordsVector();
			for (int i = 0;  i < (int)(p_parent->GetProjects()).v_projects.size(); i++)
			{
				if ((p_parent->GetProjects()).v_projects[i].m_soulseek_spoofing_enabled && i % 4 == m_modnumber)//(p_parent->GetProjects()).v_projects[i].m_project_name == "QUEENS OF THE STONE AGE-LULLABIES TO PARALYZE-2005-02-08")//m_soulseek_spoofing_enabled || (p_parent->GetProjects()).v_projects[i].m_soulseek_demand_enabled)//((p_parent->GetProjects()).v_projects[i].m_project_name == "WEEZER-TBD-2005-01-11" || (p_parent->GetProjects()).v_projects[i].m_project_name == "GARBAGE-BLEED LIKE ME-2005-01-10" || (p_parent->GetProjects()).v_projects[i].m_project_name == "LINDSAY LOHAN-SPEAK-2004-11-22" || (p_parent->GetProjects()).v_projects[i].m_project_name == "LLOYD BANKS-TBD-2004-05-20" || (p_parent->GetProjects()).v_projects[i].m_project_name == "JADAKISS-KISS OF DEATH-2004-04-14" ))
				{
					char* buf = new char[(p_parent->GetProjects()).v_projects[i].GetBufferLength()];
					(p_parent->GetProjects()).v_projects[i].WriteToBuffer(buf);
					ProjectKeywords pk;
					pk.ReadFromBuffer(buf);

			//	if (strcmp((p_parent->GetProjects()).v_projects[i].m_project_name.c_str(), "AMY GRANT-2005-01-01") == 0 || strcmp((p_parent->GetProjects()).v_projects[i].m_project_name.c_str(), "LINDSAY LOHAN-SPEAK-11-11") == 0)
					projects->v_projects.push_back(pk);
					delete [] buf;
				}
			}
		/*	vector<TrackInformation *> *tracks = new vector<TrackInformation *>;
			for (int i = 0; i < (int)(p_parent->GetTracks()).size(); i++)
				tracks->push_back((p_parent->GetTracks())[i]);*/

			ConnectionModule *mod=new ConnectionModule(3, projects);
			mod->InitParent(this,(UINT)peer_mods.size()+1);	// init parent pointer and module number
			peer_mods.push_back(mod);
			m_peer_mods++;
			char *tempstring = new char[10];
			sprintf(tempstring, "%d", m_peer_mods);
			p_parent->setPeerModStatus(tempstring);
			p_parent->Log("Adding a module 3");
			break;
		}
//TRACE("DONE ADDING A MOD!\n");
	}
//	ConnectionModule *mod=new ConnectionModule;
//	mod->InitParent(this,(UINT)v_mods.size()+1);	// init parent pointer and module number
//	v_mods.push_back(mod);
	//CString s;
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.GetWindowText(s);
	//s.Append("Added Module\n");
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText(s);
}

void ConnectionManager::WriteToLog(const char* log)
{
	char* msg = new char[150];
	strcpy(msg, (CTime::GetCurrentTime()).Format("%Y-%m-%d %H:%M:%S - "));
	strcat(msg, log);

	ofstream fout;
	fout.open("C:\\ConnectionLog.txt", ios::app);
	fout << msg << endl;
	fout.close();
	delete [] log;
	delete [] msg;
}

void ConnectionManager::CalcServerConnections(void)
{
	return;
	int connections = 0;
	for(int i = 0; i < server_mods.size(); i++)
	{
		connections += server_mods[i]->m_server_connected;
		server_mods[i]->m_server_connected = 0;
	}

	char* status = new char[10];
	sprintf(status,  "%d", connections);
	p_parent->setServerStatus(status);
}

void ConnectionManager::CalcParentConnections(void)
{
	return;
	int connections = 0, connecting = 0;

	for (int i = 0; i < parent_mods.size(); i++)
	{
		connections += parent_mods[i]->m_parent_connected;
		parent_mods[i]->m_parent_connected = 0;
		connecting += parent_mods[i]->m_parent_connecting;
		parent_mods[i]->m_parent_connecting = 0;
	}


	char* status = new char[10];
	sprintf(status,"%d", connections);
	p_parent->setParentStatus(status);
	char* status2 = new char[10];
	sprintf(status2, "%d", connecting);
	p_parent->setParentConnectingStatus(status2);
}

void ConnectionManager::CalcPeerConnections(void)
{
	return;
	int connections = 0, connecting = 0;

	for (int i = 0; i < peer_mods.size(); i++)
	{
		connections += peer_mods[i]->m_peer_connected;
		peer_mods[i]->m_peer_connected = 0;
		connecting += peer_mods[i]->m_peer_connecting;
		peer_mods[i]->m_peer_connecting = 0;
	}

	char* status = new char[10];
	sprintf(status, "%d", connections);
	p_parent->setPeerStatus(status);
	char* status2 = new char[10];
	sprintf(status2, "%d", connecting);
	p_parent->setPeerConnectingStatus(status2);
}

void ConnectionManager::ConnectNewHosts(vector<SupernodeHost> &con, int state)
{
	//p_parent->Log("Connecting new host");
	switch(state)
	{
	case 1:
		{
			for (int i = 0; i < (int)server_mods.size(); i++)
	{
		if (server_mods[i]->NumConnected() < 60)
		{
			if (server_mods[i]->NumConnected() + con.size() <= 60)
			{
				if (server_mods[i]->ConnectToHosts(con) == false)
				{
			
					return;
				}
			}
			else
			{
				vector<SupernodeHost>::iterator iter = con.begin();
				vector<SupernodeHost>::iterator beginning = con.begin();
				vector<SupernodeHost> temp;

				

				for (int j = 0; j < 60 - server_mods[i]->NumConnected(); j++, iter++)
				{
					temp.push_back(con[j]);
				}

				con.erase(beginning, iter);

				if (server_mods[i]->ConnectToHosts(temp) == false)
				{
					for (j = 0; j < (int)temp.size(); j++)
						con.push_back(temp[j]);
					temp.clear();
					return;
				}
			}		
		}
	}

	if (con.size() > 0)
	{
		AddModule(1);
	}
			break;
		}
	case 2:
		{
			CString s = "Number of parents modules = ";
			s.AppendFormat("%d", parent_mods.size());
			p_parent->Log((const char*)(LPCTSTR)s);

			for (int i = 0; i < (int)parent_mods.size(); i++)
			{
				if (parent_mods[i]->NumConnected() < 60)
				{
					if (parent_mods[i]->NumConnected() + con.size() <= 60)
					{
						if (parent_mods[i]->ConnectToHosts(m_user_name, con) == false)
						{
							return;
						}
					}
					else
					{
						vector<SupernodeHost>::iterator iter = con.begin();
						vector<SupernodeHost>::iterator beginning = con.begin();
						vector<SupernodeHost> temp;

				

						for (int j = 0; j < 60 - parent_mods[i]->NumConnected(); j++, iter++)
						{
							temp.push_back(con[j]);
						}

						con.erase(beginning, iter);

						if (parent_mods[i]->ConnectToHosts(m_user_name, temp) == false)
						{
							for (j = 0; j < (int)temp.size(); j++)
								con.push_back(temp[j]);
							temp.clear();
							return;
						}
					}		
				}
			}

			for (int i = 0; i < (int)parent_mods.size(); i++)
			{
				if (parent_mods[i]->NumConnected() < 60)
				return;
			}

			AddModule(2);

		}
	}
}

void ConnectionManager::ConnectNewHosts(SLSKtask *t)
{
	for (int i = 0; i < (int)peer_mods.size(); i++)
	{
		if (peer_mods[i]->NumConnected() + 1 <= 60)
		{
			if (peer_mods[i]->ConnectToHosts(m_user_name, t) == false)
			{
				tasks.push_back(t);
				return;
			}
			else
				return;
		}
	}

	AddModule(3);
}

unsigned int ConnectionManager::ReturnModCount(int state)
{
	if (state == 1)
		return (UINT)server_mods.size();
	if (state == 2)
		return (UINT)parent_mods.size();
	if (state == 3)
		return (UINT)peer_mods.size();

	return (UINT)-1;
}

//
// Called when the dll is going to be removed...so that the threads (hopefully) aren't still looking for the GUID cache when everything's freed
//
void ConnectionManager::KillModules()
{
	char *tempstring;
	// Free memory
	for(int i=0;i<(int)server_mods.size();i++)
	{
		delete server_mods[i];
	}
	server_mods.clear();
	m_server_mods--;
	tempstring = new char[10];
	sprintf(tempstring, "%d", m_server_mods);
	p_parent->setServerModStatus(tempstring);

	for(int i=0;i<(int)parent_mods.size();i++)
	{
		delete parent_mods[i];
	}
	parent_mods.clear();
	m_parent_mods--;
	tempstring = new char[10];
	sprintf(tempstring, "%d", m_parent_mods);
	p_parent->setParentModStatus(tempstring);

	for(int i=0;i<(int)peer_mods.size();i++)
	{
		delete peer_mods[i];
	}
	peer_mods.clear();
	m_peer_mods--;
	tempstring = new char[10];
	sprintf(tempstring, "%d", m_peer_mods);
	p_parent->setPeerModStatus(tempstring);
}


void ConnectionManager::KillModules(int state)
{
	if (state == 1)
	{
		for(int i=0;i<(int)server_mods.size();i++)
		{
			delete server_mods[i];
		}
		server_mods.clear();
		m_server_mods--;
		char *tempstring = new char[10];
		sprintf(tempstring, "%d", m_server_mods);
		p_parent->setServerModStatus(tempstring);
	}

	if (state == 2)
	{
		for(int i=0;i<(int)parent_mods.size();i++)
		{
			delete parent_mods[i];
		}
		parent_mods.clear();
		m_parent_mods--;
		char *tempstring = new char[10];
		sprintf(tempstring, "%d", m_parent_mods);
		p_parent->setParentModStatus(tempstring);
	}

	if (state == 3)
	{
		for(int i=0;i<(int)peer_mods.size();i++)
		{
			delete peer_mods[i];
		}
		peer_mods.clear();
		m_peer_mods--;
		char *tempstring = new char[10];
		sprintf(tempstring, "%d", m_peer_mods);
		p_parent->setPeerModStatus(tempstring);
	}
}

void ConnectionManager::LimitModuleCount(int state, int count)
{
	if (state == 1)
	{
		while((int)server_mods.size()>count)
		{
			delete *(server_mods.end()-1);
			server_mods.erase(server_mods.end()-1);
		}
	}

	if (state == 2)
	{
		while((int)parent_mods.size()>count)
		{
			delete *(parent_mods.end()-1);
			parent_mods.erase(parent_mods.end()-1);
		}
	}

	if (state == 3)
	{
		while((int)peer_mods.size()>count)
		{
			delete *(peer_mods.end()-1);
			peer_mods.erase(peer_mods.end()-1);
		}
	}
}

/*void ConnectionManager::ReportHosts(vector<SupernodeHost> &hosts)
{
	int i,j;
	vector<SupernodeHost> filtered_host_cache;
	
	// Filter the host cache with the people we are already connected to
	for(i=0;i<(int)hosts.size();i++)
	{
		// Check with all of the mods with the hosts that they are reporting as being currently connected to
		bool found=false;
		for(j=0;j<(int)v_mods.size();j++)
		{
			if(v_mods[j]->IsConnected(hosts[i].m_ip))
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			filtered_host_cache.push_back(hosts[i]);
		}
	}

	// Add these hosts to the host cache
	for(i=0;i<(int)filtered_host_cache.size();i++)
	{
		// Check to see if they are already in the hosts vector
		pair< hash_set<SupernodeHost>::iterator, bool > pr;
		pr = hs_host_cache.insert(filtered_host_cache[i]);
	}
}
*/
void ConnectionManager::ReConnectAll(int state)
{
	//p_parent->RemoveAllModules();
//	p_parent->Log("Reconnecting to all supernodes...");
	// Tell all of the mods to update their keywords vectors
	//delete all modules
	if (state == 1)
	{
		for(int i=0;i<(int)server_mods.size();i++)
		{
		//delete v_mods[i];
			server_mods[i]->ReConnectAll();
		}
	}

	if (state == 2)
	{
		for(int i=0;i<(int)parent_mods.size();i++)
		{
		//delete v_mods[i];
			parent_mods[i]->ReConnectAll();
		}
	}

	if (state == 3)
	{
		for(int i=0;i<(int)peer_mods.size();i++)
		{
		//delete v_mods[i];
			peer_mods[i]->ReConnectAll();
		}
	}
	//v_mods.clear();
	//AddModule();
}
void ConnectionManager::setParentConnecting(const char* msg)
{
	p_parent->setParentConnectingStatus(msg);
}

void ConnectionManager::setPeerConnecting(const char* msg)
{
	p_parent->setPeerConnectingStatus(msg);
}

void ConnectionManager::LogMsg(const char* msg)
{
	p_parent->Log(msg);
}

void ConnectionManager::ServerMsg(const char* msg)
{
	p_parent->setServerStatus(msg);
}

void ConnectionManager::ParentMsg(const char* msg)
{
	p_parent->setParentStatus(msg);
}

void ConnectionManager::PeerMsg(const char* msg)
{
	p_parent->setPeerStatus(msg);
}

void ConnectionManager::SetNewInformation(void)
{
	ProjectKeywordsVector *projects=NULL;  //FIX ME

	for (int i = 0; i < (int)(p_parent->GetProjects()).v_projects.size(); i++)
		projects->v_projects.push_back((p_parent->GetProjects()).v_projects[i]);


	/*vector<TrackInformation *> *tracks=NULL;  //FIX ME

	for (i = 0;i < (int)(p_parent->GetTracks()).size(); i++)
		tracks->push_back((p_parent->GetTracks())[i]);*/

	for (int i = 0; i < (int)parent_mods.size(); i++)
	{
		parent_mods[i]->SetProjects(projects);
	}

	for (i = 0; i < (int)peer_mods.size(); i++)
	{
		peer_mods[i]->SetProjects(projects);
	//	peer_mods[i]->SetTracks(tracks);
	}
}
void ConnectionManager::AddParent(SupernodeHost *newparent)
{
	if (parent_ips.size() < 200)
	{
		SupernodeHost nh;
		nh.m_ip = newparent->m_ip;
		nh.m_port = newparent->m_port;
		delete newparent;
		parent_ips.push_back(nh);
	}
}

void ConnectionManager::SendTask(SLSKtask *t, int state)
{
	//SLSKtask newtask = new SLSKtask();
	//newtask = t;
	//delete t;

//	vector<SLSKtask *> tasks;
//	tasks.push_back(t);

	if (state == 1)
	{
        ConnectNewHosts(t);
	}
	if (state == 2)
	{
		p_parent->Log("In send task");
		server_mods[0]->GetTask(t);
	}

	if (state == 3)
	{
		SLSKtask *newtask = new SLSKtask();
		newtask->m_token = t->m_token;
		newtask->m_user_name = t->m_user_name;
		newtask->m_state = 1001;
		server_mods[0]->GetTask(newtask);
		p_parent->SendTask(t);
	}

}


void ConnectionManager::SetUserName(CString *pUn)
{
	m_user_name = *pUn;
	delete pUn;
}