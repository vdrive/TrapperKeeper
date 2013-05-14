// SupplyManager.cpp

#include "stdafx.h"
#include "SupplyManager.h"
#include "ConnectionManager.h"
#include "GnutellaProtectorDll.h"
#include "SupplyEntryDBInterface.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists

#define MAX_SPOOF_ENTRY_PER_TRACK	100

//
//
//
SupplyManager::SupplyManager()
{
	m_updating_supply_projects=false;
	m_retreiving_spoof_entries = false;
	//m_retrieve_spoof_entries_again = false;
	p_critical_section=NULL;
	p_thread_data=NULL;
	m_minute_counter = 1440;
	//m_second_counter = 0;
	//m_5_mins_passed = false;
	m_thread = NULL;

	
#ifdef DC2	
	m_gnutella_raw_db_info.m_db_host = "63.216.246.43";
#else
	m_gnutella_raw_db_info.m_db_host = "63.221.232.35";
#endif
	m_gnutella_raw_db_info.m_db_user = "onsystems";
	m_gnutella_raw_db_info.m_db_password = "ebertsux37";
	

	ReadSupplyFromFiles();
}

//
//
//
SupplyManager::~SupplyManager()
{

}


void WriteProjectSupplyToFile(SupplyProject& sp)
{
//	UINT i;
	int buf_len = sp.GetBufferLength();
	char * buf = new char[buf_len];

	sp.WriteToBuffer(buf);

	CFile project_supply_file;
	CString filename="Supply Files\\";
	BOOL ret=MakeSureDirectoryPathExists(filename);

	if(ret==TRUE)
	{
		CString project =sp.m_name.c_str();
		project.Replace('\\','_');			// replace the backslash with _
		project.Replace('\'', '_');		// replace the single quote "'" with _
		project.Replace('-', '_');
		project.Replace('&', '_');
		project.Replace('!', '_');
		project.Replace('$', '_');
		project.Replace('@', '_');
		project.Replace('%', '_');
		project.Replace('(', '_');
		project.Replace(')', '_');
		project.Replace('+', '_');
		project.Replace('~', '_');
		project.Replace('*', '_');
		project.Replace('.', '_');
		project.Replace(',', '_');
		project.Replace('?', '_');
		project.Replace(':', '_');
		project.Replace(';', '_');
		project.Replace('"', '_');
		project.Replace('/', '_');
		project.Replace('#', '_');
		filename += project;
		filename += ".sup";
		DeleteFile(filename);
		BOOL open = project_supply_file.Open(filename, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);

		if(open==TRUE)
		{
			project_supply_file.Write(buf, buf_len);
			project_supply_file.Close();
		}
	}

	delete [] buf;
}

UINT SupplyManagerThreadProc(LPVOID pParam)
{

	UINT i;

	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	SupplyManagerThreadData thread_data;
	vector<SupplyProject> supply_projects;
	SupplyEntryDBInterface supply_entry_db_interface;


	// Create the events
	HANDLE events[SupplyManagerThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data.m_events[i]=events[i];
	}

	thread_data.p_thread_supply_projects = &supply_projects;
	
	PostMessage(hwnd,WM_INIT_SUPPLY_MANAGER_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready


	// Start the thread
	DWORD num_events=3;
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

		// Get the latest supply entris from gnutella raw data database
		if(event==1)
		{
			bool connected = false;
//			CSingleLock singleLock(&critical_section);
//			singleLock.Lock();
//			if(singleLock.IsLocked())
//			{
				ResetEvent(events[event]);	// reset event
				if(supply_projects.size() == 0)
				{
					if(thread_data.p_sm_supply_projects != NULL)
					{
						supply_projects.clear();
						SupplyProject sp;
						for(UINT i=0; i<thread_data.p_sm_supply_projects->size(); i++)
						{
							sp.Clear();
							sp.m_interval = (*thread_data.p_sm_supply_projects)[i].m_interval;
							sp.m_name = (*thread_data.p_sm_supply_projects)[i].m_name;
							sp.m_project_id = (*thread_data.p_sm_supply_projects)[i].m_project_id;
							sp.m_num_spoofs = (*thread_data.p_sm_supply_projects)[i].m_num_spoofs;
							sp.m_offset = (*thread_data.p_sm_supply_projects)[i].m_offset;
							sp.m_total_tracks = (*thread_data.p_sm_supply_projects)[i].m_total_tracks;
							sp.v_spoof_entries = (*thread_data.p_sm_supply_projects)[i].v_spoof_entries;
							supply_projects.push_back(sp);
						}
					}
				}

				if(thread_data.p_gnutella_raw_db_info!=NULL && !thread_data.m_abort_supply_retrieval)
				{
					if(strlen(thread_data.p_gnutella_raw_db_info->m_db_host.c_str())!= 0)
					{
						int trial = 0;
						while(!connected && trial < 900) // keeping trying for 15 mins
						{
							connected = supply_entry_db_interface.OpenConnection(thread_data.p_gnutella_raw_db_info->m_db_host.c_str(),
								thread_data.p_gnutella_raw_db_info->m_db_user.c_str(), thread_data.p_gnutella_raw_db_info->m_db_password.c_str(), "gnutella_raw_supply");
							if(!connected)
							{
								Sleep(1000);
								trial++;
							}
							if(thread_data.m_abort_supply_retrieval)
							{
								connected=false;
								break;
							}
						}
						if(connected)
						{
							//delete all old hashes
							//supply_entry_db_interface.DeleteOldHashes();
							for(UINT i=0; i<supply_projects.size(); i++)
							{
								if(!thread_data.m_retrieve_zero_size_only || supply_projects[i].v_spoof_entries.size()==0)
								{
									supply_projects[i].v_spoof_entries.clear();
									supply_entry_db_interface.RetrieveSupplyEntry(supply_projects[i],MAX_SPOOF_ENTRY_PER_TRACK, hwnd);
									WriteProjectSupplyToFile(supply_projects[i]);
									supply_projects[i].v_spoof_entries.clear();
								}
								SendMessage(hwnd,WM_SUPPLY_MANAGER_THREAD_PROGRESS, (WPARAM)i, (LPARAM)supply_projects.size());
								if(thread_data.m_abort_supply_retrieval)
									break;
							}
							supply_projects.clear();
							SendMessage(hwnd,WM_SUPPLY_MANAGER_THREAD_PROGRESS, (WPARAM)supply_projects.size(), (LPARAM)supply_projects.size());
							supply_entry_db_interface.CloseConnection();
						}
					}
				}
//				singleLock.Unlock();
//			}
			if(thread_data.m_abort_supply_retrieval)
			{
				PostMessage(hwnd,WM_SUPPLY_MANAGER_RETRIEVING_SUPPLY_ABORTED,0,0);
			}
			else if(connected)
				SendMessage(hwnd,WM_UPDATE_SUPPLY_MANAGER_SUPPLY_PROJECTS,(WPARAM)0,(LPARAM)0);
			else
				PostMessage(hwnd,WM_SUPPLY_MANAGER_RETRIEVING_SUPPLY_ABORTED,0,0);
		}
		if(event==2) //SupplyManager's supply projects updated
		{
			ResetEvent(events[event]);
//			CSingleLock singleLock(&critical_section);
//			singleLock.Lock();
//			if(singleLock.IsLocked())
//			{
				ResetEvent(events[event]);	// reset event
				if(thread_data.p_sm_supply_projects != NULL)
				{
					supply_projects.clear();
					SupplyProject sp;
					for(UINT i=0; i<thread_data.p_sm_supply_projects->size(); i++)
					{
						sp.Clear();
						sp.m_interval = (*thread_data.p_sm_supply_projects)[i].m_interval;
						sp.m_name = (*thread_data.p_sm_supply_projects)[i].m_name;
						sp.m_project_id = (*thread_data.p_sm_supply_projects)[i].m_project_id;
						sp.m_num_spoofs = (*thread_data.p_sm_supply_projects)[i].m_num_spoofs;
						sp.m_offset = (*thread_data.p_sm_supply_projects)[i].m_offset;
						sp.m_total_tracks = (*thread_data.p_sm_supply_projects)[i].m_total_tracks;
						supply_projects.push_back(sp);
					}
				}
//				singleLock.Unlock();
//			}
	
		}
	}

	// Close the events
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		CloseHandle(events[i]);
	}

	TRACE("Terminating SupplyManagerThreadProc\n");
	return 0;	// exit the thread
}

//
//
//
void SupplyManager::InitParent(GnutellaProtectorDll* parent)
{
	p_parent = parent;
	m_thread = AfxBeginThread(SupplyManagerThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_BELOW_NORMAL);
}

//
//
//
void SupplyManager::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	if(v_tmp_supply_projects.size()>0)
	{
		UpdateProjects(v_tmp_supply_projects);
		v_tmp_supply_projects.clear();
	}
	p_thread_data =(SupplyManagerThreadData *)lparam;
	p_thread_data->p_sm_supply_projects = &v_supply_projects;
//#ifdef _DEBUG
#ifdef BEARSHARE_ONLY
	m_gnutella_raw_db_info.m_db_host = "205.134.239.3";
#else
#ifdef DC2	
	m_gnutella_raw_db_info.m_db_host = "63.216.246.43";
#else
	m_gnutella_raw_db_info.m_db_host = "63.221.232.35";
#endif
#endif
	m_gnutella_raw_db_info.m_db_password = "ebertsux37";
	m_gnutella_raw_db_info.m_db_user = "onsystems";
//#endif
	p_thread_data->p_gnutella_raw_db_info = &m_gnutella_raw_db_info;

	if(p_connection_manager != NULL)
		p_connection_manager->ProjectSupplyUpdated(NULL);	// update the sockets (all the projects)

}

//
//
//
void SupplyManager::InitConnectionManager(ConnectionManager* cm)
{
	p_connection_manager = cm;
}

//
//
//
void SupplyManager::UpdateProjects(vector<SupplyProject> &supply_projects)
{
	// notify the thread to update the supply projects
//	while(1)
//	{
		if(p_critical_section!=NULL)
		{
			m_updating_supply_projects = true;
			Sleep(50);
			CSingleLock singleLock(p_critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				vector<SupplyProject>::iterator updated_iter = supply_projects.begin();
				vector<SupplyProject>::iterator this_iter = v_supply_projects.begin();

				//find existing project within the updated projects and update it.  If not found, delete the project
				while(this_iter != v_supply_projects.end())
				{
					bool project_found = false;
					updated_iter = supply_projects.begin();
					while(updated_iter != supply_projects.end())
					{
						if(this_iter->m_project_id==updated_iter->m_project_id)
						{
							project_found = true;
							this_iter->m_interval = updated_iter->m_interval;
							this_iter->m_num_spoofs = updated_iter->m_num_spoofs;
							this_iter->m_offset=updated_iter->m_offset;
							this_iter->m_total_tracks=updated_iter->m_total_tracks;
							this_iter->m_uber_dist_enabled=updated_iter->m_uber_dist_enabled;
							//this_iter->v_spoof_entries=updated_iter->v_spoof_entries;
							if(strcmp(this_iter->m_name.c_str(), updated_iter->m_name.c_str())!=0)
							{
								this_iter->DeleteSupplyFile();
								this_iter->m_name = updated_iter->m_name;
							}
							break;
						}
						updated_iter++;
					}
					if(project_found == false)
					{
						this_iter->DeleteSupplyFile();
						v_supply_projects.erase(this_iter);
					}
					else
					{
						this_iter++;
					}
				}

				//find new project
				updated_iter = supply_projects.begin();
				this_iter = v_supply_projects.begin();
				while(updated_iter != supply_projects.end())
				{
					bool project_found = false;
					this_iter = v_supply_projects.begin();
					while(this_iter != v_supply_projects.end())
					{
						if(updated_iter->m_project_id==this_iter->m_project_id)
						{
							project_found = true;
							break;
						}
						this_iter++;
					}
					if(project_found==false)
					{
						v_supply_projects.push_back(*updated_iter);
					}
					updated_iter++;
				}
				/*
				if(p_critical_section!=NULL)
				{
					SetEvent(p_thread_data->m_events[2]);
				}
				*/
				//p_thread_data->p_sm_supply_projects = &v_supply_projects;
				singleLock.Unlock();
			}
			m_updating_supply_projects = false;
			p_connection_manager->ProjectSupplyUpdated(NULL);	// update the sockets (all the projects)
//			break;
		}
		else
		{
			v_tmp_supply_projects = supply_projects;
		}
//	}
}

//
//
//
void SupplyManager::OnOneMinuteTimer()
{
	if(!m_retreiving_spoof_entries)
	{
		if(RetreiveNewSpoofEntries())
		{
			if(p_critical_section!=NULL)
			{
				m_retreiving_spoof_entries = true;
				SetEvent(p_thread_data->m_events[1]);
				p_parent->SpoofEntriesRetrievalStarted();
			}
		}
	}
	m_minute_counter++;
}

//
//
//
bool SupplyManager::RetreiveNewSpoofEntries()
{
	CTime current_time = CTime::GetCurrentTime();
	CStdioFile file;
	if(file.Open("last_spoof_entries_received.ini", CFile::modeRead|CFile::typeText)!=NULL)
	{
		CString read_in;
		file.ReadString(read_in);
		file.Close();
		int year,month,day;
		sscanf(read_in,"%d-%d-%d",&year,&month,&day);
		if(year==current_time.GetYear() && month == current_time.GetMonth() && day==current_time.GetDay())
			return false;		
	}
	if(current_time.GetHour() >= 4) //retreive the spoof entries only after 4 am.
		return true;
	else
		return false;
}

//
//
//
void SupplyManager::DoneRetreivingSpoofEntries()
{
//	if(m_retrieve_spoof_entries_again==false)
//	{
		m_retreiving_spoof_entries = false;
		if(p_thread_data != NULL)
			p_thread_data->m_retrieve_zero_size_only = false;
		CStdioFile file;
		if(file.Open("last_spoof_entries_received.ini", CFile::modeWrite|CFile::modeCreate|CFile::typeText)!=NULL)
		{
			CTime current_time = CTime::GetCurrentTime();
			CString date = current_time.Format("%Y-%m-%d\n");
			file.WriteString(date);
			file.Close();
		}
	//}
	//else
	//{
	//	if(p_critical_section!=NULL)
	//	{
	//		m_retrieve_spoof_entries_again=false;
	//		m_retreiving_spoof_entries = true;
	//		SetEvent(p_thread_data->m_events[1]);
	//		p_parent->SpoofEntriesRetrievalStarted();
	//	}
	//}
}

//
//
//
void SupplyManager::AbortedRetreivingSpoofEntries()
{
	//if(m_retrieve_spoof_entries_again==false)
	//{
	//	m_retreiving_spoof_entries = false;
	//	if(p_thread_data != NULL)
	//		p_thread_data->m_retrieve_zero_size_only = false;
	//}
	//else
	//{
	if(p_thread_data->m_abort_supply_retrieval)
	{
		if(p_critical_section!=NULL)
		{
			//m_retrieve_spoof_entries_again=false;
			p_thread_data->m_abort_supply_retrieval=false;
			m_retreiving_spoof_entries = true;
			SetEvent(p_thread_data->m_events[1]);
			p_parent->SpoofEntriesRetrievalStarted();
		}
	}
	else
	{
		m_retreiving_spoof_entries=false;
	}
}

//
//
//
void SupplyManager::WriteProjectSupplyToFile()
{
	UINT i;
	for(i=0; i<v_supply_projects.size(); i++)
	{
		int buf_len = v_supply_projects[i].GetBufferLength();
		char * buf = new char[buf_len];

		v_supply_projects[i].WriteToBuffer(buf);

		CFile project_supply_file;
		CString filename="Supply Files\\";
		BOOL ret=MakeSureDirectoryPathExists(filename);

		if(ret==TRUE)
		{
			CString project = v_supply_projects[i].m_name.c_str();
			project.Replace('\\','_');			// replace the backslash with _
			project.Replace('\'', '_');		// replace the single quote "'" with _
			project.Replace('-', '_');
			project.Replace('&', '_');
			project.Replace('!', '_');
			project.Replace('$', '_');
			project.Replace('@', '_');
			project.Replace('%', '_');
			project.Replace('(', '_');
			project.Replace(')', '_');
			project.Replace('+', '_');
			project.Replace('~', '_');
			project.Replace('*', '_');
			project.Replace('.', '_');
			project.Replace(',', '_');
			project.Replace('?', '_');
			project.Replace(':', '_');
			project.Replace(';', '_');
			project.Replace('"', '_');
			project.Replace('/', '_');
			project.Replace('#', '_');
			filename += project;
			filename += ".sup";
			DeleteFile(filename);
			BOOL open = project_supply_file.Open(filename, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);

			if(open==TRUE)
			{
				project_supply_file.Write(buf, buf_len);
				project_supply_file.Close();
			}
		}

		delete [] buf;
	}
}

//
//
//

void SupplyManager::ReadSupplyFromFiles()
{
	char *folder="Supply Files\\";
	string path;
	
	v_supply_projects.clear();

	WIN32_FIND_DATA file_data;
	path=folder;
	path+="*.sup";
	HANDLE search_handle = ::FindFirstFile(path.c_str(), &file_data);
	BOOL found = FALSE;

	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}

	while(found == TRUE)
	{
		CFile supply_data_file;
		path=folder;
		path+=file_data.cFileName;
		BOOL open = supply_data_file.Open(path.c_str(),CFile::typeBinary|CFile::modeRead|CFile::shareDenyNone);

		if(open==TRUE)
		{
			unsigned char * supply_data =  new unsigned char[(UINT)supply_data_file.GetLength()];

			supply_data_file.Read(supply_data, (UINT)supply_data_file.GetLength());

			SupplyProject new_supply;
			new_supply.ReadFromBuffer((char*)supply_data);
			
			v_supply_projects.push_back(new_supply);

			supply_data_file.Close();

			delete [] supply_data;
		}

		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);
}

//
//
//
void SupplyManager::UpdateGnutellaRawDBInfo(DataBaseInfo& raw_db)
{
	m_gnutella_raw_db_info = raw_db;
	p_thread_data->p_gnutella_raw_db_info = &m_gnutella_raw_db_info;
}

//
//
//
void SupplyManager::UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam)
{
	m_updating_supply_projects = true;
	//debug log
	CTime time=CTime::GetCurrentTime();
	CString time_str = time.Format("%Y-%m-%d %H:%M:%S");
	CStdioFile log;
	log.Open("supply_entries_received_at.txt", CFile::modeWrite|CFile::modeCreate|CFile::typeText);
	log.WriteString(time_str);
	log.Close();

	/*
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		v_supply_projects.clear();
		v_supply_projects = *p_thread_data->p_thread_supply_projects;
		singleLock.Unlock();
	}
	*/
	ReadSupplyFromFiles();
	p_connection_manager->ProjectSupplyUpdated(NULL);	// update the sockets (all the projects)
	m_updating_supply_projects=false;
	//WriteProjectSupplyToFile();
}

//
//
//
bool SupplyManager::IsSupplyProjectsBeingUpdated()
{
	return m_updating_supply_projects;
}

//
//
//
//
//
//
void SupplyManager::TerminateThread()
{
	if(p_critical_section!=NULL)
	{
		SetEvent(p_thread_data->m_events[0]);	// kill thread
	}
	if(m_thread!=NULL)
	{
		if(WaitForSingleObject(m_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
			//ASSERT(0);
		}
	}
}

//
//
//
void SupplyManager::RetrieveSupplyEntriesWithZeroSize()
{
	if(!m_retreiving_spoof_entries)
	{
		if(p_critical_section!=NULL)
		{
			p_thread_data->m_retrieve_zero_size_only = true;
			m_retreiving_spoof_entries = true;
			SetEvent(p_thread_data->m_events[1]);
			p_parent->SpoofEntriesRetrievalStarted();
		}
	}
}

//
//
//
void SupplyManager::KeywordsUpdated()
{
	if(m_retreiving_spoof_entries)
	{
		//m_retrieve_spoof_entries_again = true;
		if(p_critical_section!=NULL)
		{
			p_thread_data->m_abort_supply_retrieval = true;
		}
	}
}