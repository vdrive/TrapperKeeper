// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"
#include "ConnectionModuleMessageWnd.h"
#include "ProjectStatus.h"

//
//
//
ConnectionSockets::ConnectionSockets()
{
	// Initialize Critical Section
	::InitializeCriticalSection(&m_psq_lock);
	m_one_hour_timer=0;
	p_file_sharing_manager = NULL;

	ReadInUserNames();
	m_dlg_hwnd=NULL;
	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	// Init the parent pointers and message window handles
	for(int i=0;i<num_socket_events;i++)
	{
		m_sockets[i].InitParent(this);
	}
	
	// Create the reserved events
	for(i=0;i<num_reserved_events;i++)
	{
		m_events[i]=WSACreateEvent();
		if(m_events[i]==WSA_INVALID_EVENT)
		{
			char msg[1024];
			strcpy(msg,"Could not create a valid reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}

	// Fully initialize events array
	for(i=0;i<num_socket_events;i++)
	{
		m_events[num_reserved_events+i]=m_sockets[i].ReturnEventHandle();
	}
	
	m_rng.Reseed(true,32);

}

//
//
//
ConnectionSockets::~ConnectionSockets()
{
	// Delete Critical Section
	::DeleteCriticalSection(&m_psq_lock);

	int num_reserved_events=ReturnNumberOfReservedEvents();
	
	// Close the reserved events
	for(int i=0;i<num_reserved_events;i++)
	{
		if(WSACloseEvent(m_events[i])==FALSE)
		{
			char msg[1024];
			strcpy(msg,"Could not close reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}
	// close all sockets
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].Close();
	}
}

//
//
//
DWORD ConnectionSockets::ReturnNumberOfEvents()
{
	return NumberOfEvents;
}

//
//
//
int ConnectionSockets::ReturnNumberOfReservedEvents()
{
	return NumberOfReservedEvents;
}

//
//
//
int ConnectionSockets::ReturnNumberOfSocketEvents()
{
	return NumberOfSocketEvents;
}

//
//
//
void ConnectionSockets::ReportStatus(ConnectionModuleStatusData& status_data)
{
	m_one_hour_timer++;
	int i;

	int num_socket_events=ReturnNumberOfSocketEvents();

	// First tell all of the sockets that the timer has fired
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].TimerHasFired();
	}

	// See how many sockets are idle (and therefore need ips).  Any non-idle sockets, get their host connection status
	for(i=0;i<num_socket_events;i++)
	{
		if(m_sockets[i].IsSocket()==false)
		{
			m_status_data.m_idle_socket_count++;
		}
		else
		{
			if(m_sockets[i].IsConnected())
			{
				m_status_data.m_connected_socket_count++;
				// Get the ip of the connection and the connection status of the socket
				m_status_data.v_connected_host_ips.push_back(m_sockets[i].ReturnHostIP());
				m_sockets[i].ReportOtherHosts(m_status_data.v_other_hosts);
			}
			else	// connecting
			{
				m_status_data.m_connecting_socket_count++;
			}


			//m_status_data.m_cached_query_hit_count+=m_sockets[i].ReturnCachedQueryHitCount();
		}
	}

	//ConnectionModuleStatusData ret=m_status_data;
	status_data=m_status_data;
	
	m_status_data.ClearCounters();

	if( (m_one_hour_timer%3600) == 0 )// reload username from file every hour
		ReadInUserNames();
	
	//return ret;
}

//
//
//
void ConnectionSockets::ConnectToHosts(vector<SupernodeHost> &hosts)
{
	UINT i,j;
	// Find idle sockets to tell these hosts to
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<hosts.size();i++)
	{
		for(j=0;j<(UINT)num_socket_events;j++)
		//for(j=0;j<1;j++)
		{
			if(m_sockets[j].IsSocket()==false)
			{
				//m_sockets[j].Create();
				m_sockets[j].ConnectToSupernode(hosts[i]);
				break;
			}
		}
	}
}

//
//
//
/*
void ConnectionSockets::ReportHosts(vector<SupernodeHost> &hosts)
{
	int i,j;

	// Add these hosts to the hosts vector
	for(i=0;i<(int)hosts.size();i++)
	{
		// Check to see if they are already in the hosts vector
		bool found=false;
		for(j=(int)m_status_data.v_host_cache.size()-1;j>=0;j--)
		{
			if(hosts[i]==m_status_data.v_host_cache[j])
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			m_status_data.v_host_cache.push_back(hosts[i]);
		}
	}
}
*/
//
//
//
void ConnectionSockets::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	int i;
	int num_socket_events=ReturnNumberOfSocketEvents();
	::EnterCriticalSection(&m_psq_lock);
	hash_set<ProjectSupplyQuery>::iterator iter = hs_project_supply_queries.begin();
	while(iter!=hs_project_supply_queries.end())
	{
		if( (iter->m_search_id < (psq.m_search_id)-MAX_SEARCH) && iter->m_is_searching )
		{
			for(i=0;i<num_socket_events;i++)
			{
				m_sockets[i].SendStopSearching(iter->m_search_id);
			}
			/*
			CString log;
			log.Format("Stop searching project \"%s\", search ID: %d",iter->m_project.c_str(),iter->m_search_id);
			Log(log);
			*/
			iter->m_is_searching = false;
		}
		if(iter->m_project_id == psq.m_project_id)
		{
			hs_project_supply_queries.erase(iter);
			iter = hs_project_supply_queries.begin();
		}
		else
			iter++;
	}
	hs_project_supply_queries.insert(psq);
	::LeaveCriticalSection(&m_psq_lock);

	// Tell all of the sockets to perform this query, unless this is a searcher query, where we only tell 5 ultrapeer sockets
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].PerformProjectSupplyQuery(psq);
	}
}

//
//
//
void ConnectionSockets::StopSearchingProject(const char* project_name)
{
	int i;
	int num_socket_events=ReturnNumberOfSocketEvents();
	::EnterCriticalSection(&m_psq_lock);
	hash_set<ProjectSupplyQuery>::iterator iter = hs_project_supply_queries.begin();
	while(iter!=hs_project_supply_queries.end())
	{
		if( (stricmp(project_name, iter->m_project.c_str())==0) && iter->m_is_searching )
		{
			for(i=0;i<num_socket_events;i++)
			{
				m_sockets[i].SendStopSearching(iter->m_search_id);
			}
			iter->m_is_searching = false;
			break;
		}
		iter++;
	}
	::LeaveCriticalSection(&m_psq_lock);
}

//
//
//

void ConnectionSockets::UpdateKeywords(vector<ProjectKeywords> &keywords)
{
	UINT i,j;
	
	v_keywords=keywords;

	// Make sure that the project status data is still valid since the keywords have been updated...basically the indexes for each project in
	// the keywords vector have to be the same as the indexes for the same project in the project status vector.
	vector<ProjectStatus> old_project_status=m_status_data.v_project_status;
	m_status_data.v_project_status.clear();

	for(i=0;i<v_keywords.size();i++)
	{
		bool found=false;
		for(j=0;j<old_project_status.size();j++)
		{
			if(strcmp(old_project_status[j].ProjectName(),v_keywords[i].m_project_name.c_str())==0)
			{
				found=true;
				m_status_data.v_project_status.push_back(old_project_status[j]);
				break;
			}
		}

		// If not found, create a new project status object (this must be a new project, then)
		if(!found)
		{
			ProjectStatus ps;
			ps.ProjectName((char *)keywords[i].m_project_name.c_str());
			m_status_data.v_project_status.push_back(ps);
		}
	}

	::EnterCriticalSection(&m_psq_lock);
	// Make sure all of the project keyword data of the project supply queries are still correct
	hash_set<ProjectSupplyQuery>::iterator psq_iter=hs_project_supply_queries.begin();
	while(psq_iter!=hs_project_supply_queries.end())
	{
		bool found=false;
		for(i=0;i<v_keywords.size();i++)
		{
			if(strcmp(psq_iter->m_project.c_str(),v_keywords[i].m_project_name.c_str())==0)
			{
				found=true;
				psq_iter->ExtractProjectKeywordData(&v_keywords[i]);
				psq_iter->m_project_status_index=i;
				break;
			}
		}

		// If not found, then this project no longer exists, so remove the query from the vector
		if(!found)
		{
			hs_project_supply_queries.erase(psq_iter);
			psq_iter=hs_project_supply_queries.begin();
		}
		else
		{
			psq_iter++;
		}
	}
	::LeaveCriticalSection(&m_psq_lock);
}

//
//
//
void ConnectionSockets::Log(const char* log)
{
	if(m_dlg_hwnd != NULL)
		::SendMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)log,0);
}

//
//
//
void ConnectionSockets::ReConnectAll()
{
	UINT j;
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(j=0;j<(UINT)num_socket_events;j++)
	{
		m_sockets[j].Close();
	}
}


//
//
//
void ConnectionSockets::GetRandomUserName(CString& username)
{
	if(v_usernames.size() > 0)
	{
		int index = m_rng.GenerateWord32(0,v_usernames.size()-1);
		char* new_username = new char[v_usernames[index].GetLength()+6];
		strcpy(new_username,v_usernames[index]);
		ChangeUserName(new_username);
		username = new_username;
		delete [] new_username;
	}
	else
	{
		/*
		CStdioFile file;
		file.Open("winmx_error.txt",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeText|CFile::shareDenyWrite);
		file.SeekToEnd();
		file.WriteString("No usernames available\n");
		file.Close();
		*/
		char new_username[13];
		int random_subfix = m_rng.GenerateWord32(100,999);
		char tmp[4];
		itoa(random_subfix,tmp,10);
		username = "anonymous";
		username += tmp;
	}
}

//
//
//
void ConnectionSockets::ChangeUserName(char * username)
{
	CString string = username;
	char* tempuser = new char[strlen(username)+6];
	int len;
	int num;
	int randnum;
	char numchar[1+1];

	//Remove all the numbers
	string.Remove('0');
	string.Remove('1');
	string.Remove('2');
	string.Remove('3');
	string.Remove('4');
	string.Remove('5');
	string.Remove('6');
	string.Remove('7');
	string.Remove('8');
	string.Remove('9');

	len = string.GetLength();
	randnum = 2;

	strcpy(tempuser,string);
	for (int i = 0;i<randnum;i++)
	{
		num = rand()%10;
		_itoa(num,numchar,10);
		strcat(tempuser,numchar);
	}

	num = (len%10);
	_itoa(num,numchar,10);
	strcat(tempuser,numchar);

	strcpy(username,tempuser);
	delete [] tempuser;
}

//
//
//
void ConnectionSockets::ReadInUserNames()
{
	CStdioFile file;
	CString filestring;
	char tempstring[256+1];
	if(file.Open("c:\\syncher\\rcv\\Kazaa-Launcher\\namelist.txt", CFile::modeRead|CFile::typeText|CFile::shareDenyNone) != 0)
	{
		v_usernames.clear();
		LPTSTR data = file.ReadString(tempstring, 256);
		while (data!= NULL)
		{
			filestring = tempstring;
			filestring.TrimRight();
			filestring.TrimLeft();
			v_usernames.push_back(filestring);
			data = file.ReadString(tempstring, 256);
		}
		/*
		CString msg;
		msg.Format("Read in %d usernames",v_usernames.size());
		MessageBox(NULL,msg,"username",MB_OK);
		*/
	}
	else
	{
		//MessageBox(NULL, "Coudn't find the file \"namelist.txt\"", "ERROR", MB_OK|MB_ICONERROR);
		return;
	}
	file.Close();
}

