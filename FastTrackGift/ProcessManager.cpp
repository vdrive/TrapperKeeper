#include "StdAfx.h"
#include "processmanager.h"
#include <afxmt.h>	// for CCriticalSection
#include "FastTrackGiftDll.h"

#ifdef SPECIAL_SUPPLY_COLLECTING
#define SUPPLY_RAW_DATA_TTL		3	//keeping 3 day of raw data
#else
#define SUPPLY_RAW_DATA_TTL		1	//keeping 1 day of raw data
#endif

ProcessDBInterface ProcessManager::g_db_supply_interface;

//bool ProcessManager::m_is_processing = false;

bool ProcessManager::m_is_all_done = false;
bool ProcessManager::m_is_maintaining = false;
bool ProcessManager::m_is_supply_processing = false;

ProcessManager::ProcessManager(void)
{
	p_supply_critical_section=NULL;
	p_supply_thread_data=NULL;
}

//
//
//
ProcessManager::~ProcessManager(void)
{
}

//
//
//
void ProcessManager::KillThread()
{
	if(m_is_supply_processing)
	{
		if(p_supply_critical_section!=NULL)
		{
			SetEvent(p_supply_thread_data->m_events[0]);	// kill thread	
		}
	}
}

//
//
//
bool ProcessManager::IsProcessing()
{
	return (m_is_supply_processing || m_is_maintaining);
}

//
//
//
bool ProcessManager::IsAllDone()
{
	return m_is_all_done;
}

//
//
//
UINT ProcessManagerSupplyThreadProc(LPVOID pParam)
{
	CTime current_time = CTime::GetCurrentTime();
	UINT i;
	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	
	CCriticalSection critical_section;
	ProcessManagerThreadData thread_data;
	

	// Create the events
	
	HANDLE events[ProcessManagerThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data.m_events[i]=events[i];
	}
	
	PostMessage(hwnd,WM_INIT_PROCESS_MANAGER_SUPPLY_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	

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

		// start the raw data processing
		if(event==1)
		{
			DWORD new_event = -1;

			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				thread_data.m_supply_progress=0;

				//process raw supply data
				for(i=0; i<(UINT)thread_data.p_projects->size(); i++)
				{
					::PostMessage(hwnd, WM_SUPPLY_PROCESS_PROJECT, (WPARAM)(*thread_data.p_projects)[i].m_project_name.c_str(),0);
					char date[32];
					char start_timestamp[32];
					char end_timestamp[32];
					int year,month,day;
					memset(&date, 0, sizeof(date));
					memset(&start_timestamp, 0, sizeof(start_timestamp));
					memset(&end_timestamp, 0, sizeof(end_timestamp));
					bool more_supply = true;
					bool first_round = true;

					ProcessManager::g_db_supply_interface.GetLastProcessedDateForSupply((*thread_data.p_projects)[i].m_project_name.c_str(),date);
					if(strlen(date) == 0) //no previous results in processed database
					{
						ProcessManager::g_db_supply_interface.GetFirstSupplyDataInsertionTimestamp(
							(*thread_data.p_projects)[i].m_project_name.c_str(),date);
						if(strlen(date) == 0) //no results in raw database
							continue;
						else
						{
							sscanf(date, "%4d%2d%2d", &year, &month, &day);
							strcpy(start_timestamp, date);
						}
					}
					else
					{
						sscanf(date, "%d-%d-%d", &year, &month, &day);
						CTime time(year, month, day, 0,0,0);
						time += CTimeSpan(1,0,0,0);
						if(time.GetHour() == 23)
							time += CTimeSpan(0,1,0,0); //winter time
						else if(time.GetHour() == 1)
							time -= CTimeSpan(0,1,0,0); // summer time
						strcpy(start_timestamp, time.Format("%Y%m%d%H%M%S"));
						year = time.GetYear();
						month = time.GetMonth();
						day = time.GetDay();
					}
					
					while(more_supply)
					{
						if(!first_round)
						{
							CTime time(year, month, day, 0,0,0);
							time += CTimeSpan(1,0,0,0);
							if(time.GetHour() == 23)
								time += CTimeSpan(0,1,0,0); //winter time
							else if(time.GetHour() == 1)
								time -= CTimeSpan(0,1,0,0); // summer time
							strcpy(start_timestamp, time.Format("%Y%m%d%H%M%S"));
							year = time.GetYear();
							month = time.GetMonth();
							day = time.GetDay();
						}
						//check to see if we have reached today, then stop demand process for this project
						if(year == current_time.GetYear() && month == current_time.GetMonth() && day == current_time.GetDay())
							break;

						//get the end timestamp
						sprintf(end_timestamp, "%d%.2d%.2d%.2d%.2d%.2d", year,month,day,23,59,59);
						
						//get supply query and insert to processed database
						char on_date[32];
						sprintf(on_date, "%d-%.2d-%.2d", year, month, day);
						int total_tracks=0;
						
						for(UINT j=0; j<(*thread_data.p_projects)[i].m_supply_keywords.v_keywords.size(); j++)
						{
							if((*thread_data.p_projects)[i].m_supply_keywords.v_keywords[j].m_track > total_tracks)
								total_tracks = (*thread_data.p_projects)[i].m_supply_keywords.v_keywords[j].m_track;
						}

						ProcessManager::g_db_supply_interface.ProcessSupply((*thread_data.p_projects)[i].m_project_name.c_str(),
									start_timestamp, end_timestamp, on_date,total_tracks);
						first_round=false;
					}
					thread_data.m_supply_progress = (int)((float)i/(float)thread_data.p_projects->size()*(float)100);

					//check if the program is exiting
					new_event = WaitForMultipleObjects(num_events,events,wait_all,0);
					if(new_event==0)
					{
						ResetEvent(events[event]);
						break;
					}
				}
				thread_data.m_supply_progress = (int)((float)i/(float)thread_data.p_projects->size()*(float)100);

				singleLock.Unlock();
				break;
			}
		}
	}

	// Close the events
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		CloseHandle(events[i]);
	}
	thread_data.m_supply_progress=100;
	if(event!=0)
		::PostMessage(hwnd, WM_PROCESS_MANAGER_SUPPLY_DONE,0,0);
	
	return 0;	// exit the thread
}

//
//
//
CString GetSupplyTableName(CString project)
{
	if(project.GetLength() > 50)
	{
		project.Truncate(50);
	}

	CString table = "SUPPLY_TABLE_";
	table += project;
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');
	return table;
}

//
//
//
UINT DBMaintenanceThreadProc(LPVOID pParam)
{
	//timestamp for supply
	CTime current_time = CTime::GetCurrentTime();
	CTime supply_delete_from_date =  current_time - CTimeSpan(SUPPLY_RAW_DATA_TTL,0,0,0);
	char supply_delete_timestamp[32];
	sprintf(supply_delete_timestamp, "%d%.2d%.2d000000", supply_delete_from_date.GetYear(),supply_delete_from_date.GetMonth(),supply_delete_from_date.GetDay());

	ProcessManager* manager = (ProcessManager*)pParam;
	HWND hwnd=manager->p_parent->m_dlg.GetSafeHwnd();

	PostMessage(hwnd,WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA,0,0);
	
	UINT supply_records_delete = 0;

	//delete old raw supply data
	vector<CString> supply_tables;
	supply_records_delete = 0;
	ProcessManager::g_db_supply_interface.GetAllSupplyTables(supply_tables);
	for(UINT i=0; i<supply_tables.size(); i++)
	{
		bool found=false;
		for(UINT j=0; j<manager->v_projects.size();j++)
		{
			CString table_name = GetSupplyTableName(manager->v_projects[j].m_project_name.c_str());
			if(supply_tables[i].CompareNoCase(table_name)==0)
			{
				found=true;
				break;
			}
		}
		if(found)
			supply_records_delete += ProcessManager::g_db_supply_interface.DeleteRawData(supply_delete_timestamp, supply_tables[i]);
		else
			ProcessManager::g_db_supply_interface.DeleteTable(supply_tables[i]);

	}

	if(supply_records_delete > 0)
	{
		ProcessManager::g_db_supply_interface.OptimizeTables(supply_tables);
	}

	::PostMessage(hwnd, WM_PROCESS_MANAGER_DB_MAINTENANCE_DONE,supply_records_delete,0);

	return 0;	// exit the thread
}

//
//
//
void ProcessManager::StartProcessRawData(DataBaseInfo& db_info, vector<ProjectKeywords>* keywords)
{
	//Supply Process
	if(!m_is_supply_processing)
	{
		v_projects = *keywords;
		m_is_supply_processing = true;
		//db connection to processed database
#ifdef DC2
		if(g_db_supply_interface.OpenSupplyConnection(db_info.m_db_host.c_str(),db_info.m_db_user.c_str(),
										db_info.m_db_password.c_str(),"DCDATA2"))
#else
		if(g_db_supply_interface.OpenSupplyConnection(db_info.m_db_host.c_str(),db_info.m_db_user.c_str(),
										db_info.m_db_password.c_str(),"DCDATA"))
#endif
		{
			//db connection to raw data database
//#ifdef _DEBUG
//			if(g_db_supply_interface.OpenRawDataConnection("38.119.64.75","onsystems","ebertsux37","fasttrack_raw_supply"))
//#else
			if(g_db_supply_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","fasttrack_raw_supply"))
//#endif
				AfxBeginThread(ProcessManagerSupplyThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_LOWEST);
			else
				m_is_supply_processing = false;
		}
		else
			m_is_supply_processing = false;
	}
}

//
//
//
void ProcessManager::InitSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
	p_supply_critical_section=(CCriticalSection *)wparam;
	p_supply_thread_data=(ProcessManagerThreadData *)lparam;

	if(p_supply_critical_section!=NULL)
	{
		CSingleLock singleLock(p_supply_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			p_supply_thread_data->Clear();
			p_supply_thread_data->p_projects = &v_projects;
			singleLock.Unlock();
		}
		SetEvent(p_supply_thread_data->m_events[1]);	// start the process
	}
}

//
//
//
void ProcessManager::SupplyProcessFinished()
{
	m_is_supply_processing = false;
	g_db_supply_interface.CloseSupplyConnection();
	g_db_supply_interface.CloseRawDataConnection();
	
//	if( !m_is_demand_processing && !m_is_supply_processing && !m_is_reverse_dns_processing )
		m_is_all_done = true;
	p_supply_thread_data = NULL;

	if(m_is_all_done)
		DBMaintenance(); //delete old raw data records
}

//
//
//
void ProcessManager::MaintenanceFinished()
{
	g_db_supply_interface.CloseRawDataConnection();

	m_is_maintaining = false;
	p_parent->DBMaintenanceFinished();
}

//
//
//
void ProcessManager::Reset()
{
	if(!m_is_supply_processing)
		m_is_all_done = false;
}

//
//
//
void ProcessManager::GetProcessProgress(int& supply_progress)
{
	if(p_supply_thread_data != NULL)
	{
		supply_progress = p_supply_thread_data->m_supply_progress;
	}
}

//
//
// delete old raw data from data base
void ProcessManager::DBMaintenance(void)
{
	if(!m_is_maintaining)
	{
		m_is_maintaining = true;
		bool connected = false;
		int trial = 0;
		while(!connected)
		{
			//db connection to raw data database
			connected = g_db_supply_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","fasttrack_raw_supply");
			trial++;
			if(trial > 10)
				break;
			Sleep(500);
		}
		if(connected)
		{
			p_parent->DBMaintenanceReadyToStart();
			AfxBeginThread(DBMaintenanceThreadProc,(LPVOID)this/*p_parent->m_dlg.GetSafeHwnd()*/,THREAD_PRIORITY_LOWEST);
		}
		else
			m_is_maintaining = false;
	}
}

//
//
//
bool ProcessManager::IsMaintaining()
{
	return m_is_maintaining;
}