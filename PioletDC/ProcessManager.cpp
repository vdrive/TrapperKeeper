#include "StdAfx.h"
#include "processmanager.h"
#include <afxmt.h>	// for CCriticalSection
#include "PioletDCDll.h"

ProcessDBInterface ProcessManager::g_db_interface;
bool ProcessManager::m_is_processing = false;
bool ProcessManager::m_is_all_done = false;
bool ProcessManager::m_is_maintaining = false;

ProcessManager::ProcessManager(void)
{
	p_critical_section=NULL;
	p_thread_data=NULL;
//	m_is_processing = false;
//	m_is_all_done = false;
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
	if(m_is_processing)
	{
		if(p_critical_section!=NULL)
		{
			SetEvent(p_thread_data->m_events[0]);	// kill thread	
		}
/*		if(p_maintenance_critical_section!=NULL)
		{
			SetEvent(p_maintenance_thread_data->m_events[0]);
		}
*/
	}
}

//
//
//
void ProcessManager::InitParent(PioletDCDll *parent)
{
	Manager::InitParent(parent);
}

//
//
//
bool ProcessManager::IsProcessing()
{
	return (m_is_processing || m_is_maintaining);
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
UINT ProcessManagerThreadProc(LPVOID pParam)
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
	
	PostMessage(hwnd,WM_INIT_PROCESS_MANAGER_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	

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
				thread_data.m_demand_progress=0;
				thread_data.m_supply_progress=0;

				//process raw demand data
				for(i=0; i<(UINT)thread_data.p_projects->size(); i++)
				{
					::PostMessage(hwnd, WM_DEMAND_PROCESS_PROJECT, (WPARAM)(*thread_data.p_projects)[i].m_project_name.c_str(),0);
					char date[32];
					char start_timestamp[32];
					char end_timestamp[32];
					int year,month,day;
					bool more_demand = true;
					bool first_round = true;

                    //while(more_demand)
					//{
					memset(&date, 0, sizeof(date));
					memset(&start_timestamp, 0, sizeof(start_timestamp));
					memset(&end_timestamp, 0, sizeof(end_timestamp));
					year=month=day=0;
					ProcessManager::g_db_interface.GetLastProcessedDateForDemand((*thread_data.p_projects)[i].m_project_name.c_str(),date);
					if(strlen(date) == 0) //no previous results in processed database
					{
						ProcessManager::g_db_interface.GetFirstDemandDataInsertionTimestamp(
							(*thread_data.p_projects)[i].m_project_name.c_str(),date);
						if(strlen(date) == 0) //no results in raw database
							continue;
						else
						{
							char tmp[32];
							strncpy(tmp, date, 4);
							year = atoi(tmp);
							char* ptr = date;
							ptr+=4;
							strncpy(tmp, ptr, 2);
							tmp[2]='\0';
							month = atoi(tmp);
							ptr+=2;
							strncpy(tmp, ptr, 2);
							tmp[2]='\0';
							day = atoi(tmp);							
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
					while(more_demand)
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
						
						//get demand query and insert to processed database
						char on_date[32];
						sprintf(on_date, "%d-%.2d-%.2d", year, month, day);
						ProcessManager::g_db_interface.ProcessDemand((*thread_data.p_projects)[i].m_project_name.c_str(),
											start_timestamp, end_timestamp, on_date);
						first_round = false;
					}

					thread_data.m_demand_progress = (int)((float)i/(float)thread_data.p_projects->size()*(float)100);

					//check if the program is exiting
					new_event = WaitForMultipleObjects(num_events,events,wait_all,0);
					if(new_event==0)
					{
						ResetEvent(events[event]);
						break;
					}
				}
				thread_data.m_demand_progress = (int)((float)i/(float)thread_data.p_projects->size()*(float)100);
				if(new_event==0)
				{
					singleLock.Unlock();
					break;
				}

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

                    //while(more_supply)
					//{
					ProcessManager::g_db_interface.GetLastProcessedDateForSupply((*thread_data.p_projects)[i].m_project_name.c_str(),date);
					if(strlen(date) == 0) //no previous results in processed database
					{
						ProcessManager::g_db_interface.GetFirstSupplyDataInsertionTimestamp(
							(*thread_data.p_projects)[i].m_project_name.c_str(),date);
						if(strlen(date) == 0) //no results in raw database
							continue;
						else
						{
							char tmp[32];
							strncpy(tmp, date, 4);
							year = atoi(tmp);
							char* ptr = date;
							ptr+=4;
							strncpy(tmp, ptr, 2);
							tmp[2]='\0';
							month = atoi(tmp);
							ptr+=2;
							strncpy(tmp, ptr, 2);
							tmp[2]='\0';
							day = atoi(tmp);							
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
						ProcessManager::g_db_interface.ProcessSupply((*thread_data.p_projects)[i].m_project_name.c_str(),
									start_timestamp, end_timestamp, on_date);
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
	
	if(event!=0)
		::PostMessage(hwnd, WM_PROCESST_MANAGER_DONE,0,0);
	
	return 0;	// exit the thread
}

//
//
//
UINT DBMaintenanceThreadProc(LPVOID pParam)
{
	CTime current_time = CTime::GetCurrentTime();
	CTime delete_from_date =  current_time - CTimeSpan(PIOLET_RAW_DATA_TTL,0,0,0);
	char delete_timestamp[32];
	sprintf(delete_timestamp, "%d%.2d%.2d000000", delete_from_date.GetYear(),delete_from_date.GetMonth(),delete_from_date.GetDay());

	HWND hwnd=(HWND)pParam;

	PostMessage(hwnd,WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA,0,0);
	
	UINT demand_records_delete = 0;
	UINT supply_records_delete = 0;

	//delete old raw data
	demand_records_delete = ProcessManager::g_db_interface.DeleteRawData(delete_timestamp, PIOLET_DEMAND_TABLE);
	supply_records_delete = ProcessManager::g_db_interface.DeleteRawData(delete_timestamp, PIOLET_SUPPLY_TABLE);

	vector<CString>tables;
	tables.push_back(PIOLET_DEMAND_TABLE);
	tables.push_back(PIOLET_SUPPLY_TABLE);
				
	//optimize and reclaim the unused space
	ProcessManager::g_db_interface.OptimizeTables(tables);

	::PostMessage(hwnd, WM_PROCESST_MANAGER_DB_MAINTENANCE_DONE,demand_records_delete,supply_records_delete);
	return 0;	// exit the thread
}

//
//
//
void ProcessManager::StartProcessRawData(DataBaseInfo& db_info, vector<ProjectKeywords>& keywords)
{
	if(!m_is_processing)
	{
		v_projects = keywords;
		m_is_processing = true;
		//db connection to processed database
		if(g_db_interface.OpenConnection(db_info.m_db_host.c_str(),db_info.m_db_user.c_str(),
										db_info.m_db_password.c_str(),"DCDATA"))
		{
			//db connection to raw data database
			if(g_db_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","piolet"))
				AfxBeginThread(ProcessManagerThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_LOWEST);
			else
				m_is_processing = false;
		}
		else
			m_is_processing = false;
	}
}

//
//
//
void ProcessManager::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	p_thread_data=(ProcessManagerThreadData *)lparam;

	if(p_critical_section!=NULL)
	{
		CSingleLock singleLock(p_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			p_thread_data->Clear();
			p_thread_data->p_projects = &v_projects;
			singleLock.Unlock();
		}
		SetEvent(p_thread_data->m_events[1]);	// start the process
	}
}

//
//
//
/*
void ProcessManager::InitMaintenanceThreadData(WPARAM wparam,LPARAM lparam)
{
	p_maintenance_critical_section=(CCriticalSection *)wparam;
	p_maintenance_thread_data=(ProcessManagerDBMaintenanceThreadData *)lparam;

	if(p_maintenance_critical_section!=NULL)
	{
		CSingleLock singleLock(p_maintenance_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			singleLock.Unlock();
		}
		SetEvent(p_maintenance_thread_data->m_events[1]);	// start the process
	}
}
*/
//
//
//
void ProcessManager::ProcessFinished()
{
	g_db_interface.CloseConnection();
	g_db_interface.CloseRawDataConnection();
	m_is_processing = false;
	m_is_all_done = true;
	p_thread_data = NULL;
	DBMaintenance(); //delete old raw data records
}

//
//
//
void ProcessManager::MaintenanceFinished()
{
	g_db_interface.CloseRawDataConnection();
	m_is_maintaining = false;
//	p_maintenance_thread_data = NULL;
	p_parent->DBMaintenanceFinished();
}

//
//
//
void ProcessManager::Reset()
{
	if(!m_is_processing)
		m_is_all_done = false;
}

//
//
//
void ProcessManager::GetProcessProgress(int& demand_progress, int& supply_progress)
{
	if(p_thread_data != NULL)
	{
		demand_progress = p_thread_data->m_demand_progress;
		supply_progress = p_thread_data->m_supply_progress;
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
		/*
		//db connection to raw data database
		if(g_db_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","piolet"))
		{
			p_parent->DBMaintenanceReadyToStart();
			AfxBeginThread(DBMaintenanceThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_LOWEST);
		}
		else
			m_is_maintaining = false;
		*/
		bool connected = false;
		int trial = 0;
		while(!connected)
		{
			//db connection to raw data database
			connected = g_db_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","piolet");
			trial++;
			if(trial > 10)
				break;
			Sleep(500);
		}
		if(connected)
		{
			p_parent->DBMaintenanceReadyToStart();
			AfxBeginThread(DBMaintenanceThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_LOWEST);
		}
		else
			m_is_maintaining = false;
	}
}
