#include "StdAfx.h"
#include "processmanager.h"
#include <afxmt.h>	// for CCriticalSection
#include "GnutellaSupplyDll.h"

ProcessDBInterface ProcessManager::g_db_demand_interface;
ProcessDBInterface ProcessManager::g_db_supply_interface;
ProcessDBInterface ProcessManager::g_db_hash_interface;
ProcessDBInterface ProcessManager::g_processed_db_interface;

//bool ProcessManager::m_is_processing = false;

bool ProcessManager::m_is_all_done = false;
bool ProcessManager::m_is_maintaining = false;
bool ProcessManager::m_is_demand_processing = false;
bool ProcessManager::m_is_supply_processing = false;
//bool ProcessManager::m_is_reverse_dns_processing = false;

ProcessManager::ProcessManager(void)
{
	p_demand_critical_section=NULL;
	p_demand_thread_data=NULL;

	p_supply_critical_section=NULL;
	p_supply_thread_data=NULL;
	m_maintenance_progress=0;

	//p_reverse_dns_critical_section=NULL;
	//p_reverse_dns_thread_data=NULL;

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
	if(m_is_demand_processing)
	{
		if(p_demand_critical_section!=NULL)
		{
			SetEvent(p_demand_thread_data->m_events[0]);	// kill thread	
		}
	}
	if(m_demand_thread!=NULL)
	{
		if(WaitForSingleObject(m_demand_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
			//ASSERT(0);
		}
	}

	if(m_is_supply_processing)
	{
		if(p_supply_critical_section!=NULL)
		{
			SetEvent(p_supply_thread_data->m_events[0]);	// kill thread	
		}
	}
	if(m_supply_thread!=NULL)
	{
		if(WaitForSingleObject(m_supply_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
			//ASSERT(0);
		}
	}
}

//
//
//
void ProcessManager::InitParent(GnutellaSupplyDll *parent)
{
	Manager::InitParent(parent);
}

//
//
//
bool ProcessManager::IsProcessing()
{
	//return (m_is_processing || m_is_maintaining);
	return (m_is_demand_processing || m_is_supply_processing || m_is_maintaining/* || m_is_reverse_dns_processing*/ );
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
UINT ProcessManagerDemandThreadProc(LPVOID pParam)
{
	/*
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
	
	PostMessage(hwnd,WM_INIT_PROCESS_MANAGER_DEMAND_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	

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

					memset(&date, 0, sizeof(date));
					memset(&start_timestamp, 0, sizeof(start_timestamp));
					memset(&end_timestamp, 0, sizeof(end_timestamp));
					year=month=day=0;
					ProcessManager::g_db_demand_interface.GetLastProcessedDateForDemand((*thread_data.p_projects)[i].m_project_name.c_str(),date);
					if(strlen(date) == 0) //no previous results in processed database
					{
						ProcessManager::g_db_demand_interface.GetFirstDemandDataInsertionTimestamp(
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
						ProcessManager::g_db_demand_interface.ProcessDemand((*thread_data.p_projects)[i].m_project_name.c_str(),
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
	
	thread_data.m_demand_progress = 100;
	if(event!=0)
		::PostMessage(hwnd, WM_PROCESS_MANAGER_DEMAND_DONE,0,0);
	*/
	return 0;	// exit the thread
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

		// start the raw data processing
		else if(event==1)
		{
			while(thread_data.v_projects.size())
			{
				ProjectKeywords* project = new ProjectKeywords();
				CSingleLock singleLock(&critical_section);
				singleLock.Lock();
				if(singleLock.IsLocked())
				{
					*project = *(*(thread_data.v_projects.begin()));
					delete *(thread_data.v_projects.begin());
					thread_data.v_projects.erase(thread_data.v_projects.begin());
					thread_data.m_supply_progress = (int)thread_data.v_projects.size();
					singleLock.Unlock();
				}
				if(project->m_id!=0) //there is a project waiting to be processed
				{
					::SendMessage(hwnd, WM_SUPPLY_PROCESS_PROJECT, (WPARAM)project->m_project_name.c_str(),0);
					//ProcessManager::g_db_supply_interface.DeleteProcessedIPs(*project);
					ProcessManager::g_db_supply_interface.ProcessRealTimeSupply(*project);
					//delete raw data
					ProcessManager::g_db_supply_interface.DeleteAllRawData(project->m_project_name.c_str());
					//ProcessManager::g_db_supply_interface.OptimizeProcessedIPs(*project);
				}
				delete project;
			}
			ResetEvent(events[event]);
		}
		else if(event==2)
		{

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
//UINT ProcessManagerReverseDNSThreadProc(LPVOID pParam)
//{
//	CTime current_time = CTime::GetCurrentTime();
//	UINT i;
//	// Init message window handle
//	HWND hwnd=(HWND)pParam;
//
//	// Init the message data structure and send it
//	
//	CCriticalSection critical_section;
//	ProcessManagerThreadData thread_data;
//	
//
//	// Create the events
//	
//	HANDLE events[ProcessManagerThreadData::NumberOfEvents];
//	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
//	{
//		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
//		thread_data.m_events[i]=events[i];
//	}
//	
//	PostMessage(hwnd,WM_INIT_PROCESS_MANAGER_REVERSE_DNS_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
//	
//
//	// Start the thread
//	
//	DWORD num_events=2;
//	BOOL wait_all=FALSE;
//	DWORD timeout=INFINITE;
//	DWORD event;	// which event fired
//
//	while(1)
//	{
//		event=WaitForMultipleObjects(num_events,events,wait_all,timeout);
//
//		// Check to see if this is the kill thread events (event 0)
//		if(event==0)
//		{
//			ResetEvent(events[event]);
//			break;
//		}
//
//		// start the raw data processing
//		if(event==1)
//		{
//			DWORD new_event = -1;
//
//			CSingleLock singleLock(&critical_section);
//			singleLock.Lock();
//			if(singleLock.IsLocked())
//			{
//				thread_data.m_reverse_dns_progress=0;
//
//				//process reverse dns data
//				for(i=0; i<(UINT)thread_data.p_projects->size(); i++)
//				{
//					bool table_full=false;
//					::PostMessage(hwnd, WM_REVERSE_DNS_PROCESS_PROJECT, (WPARAM)(*thread_data.p_projects)[i].m_project_name.c_str(),0);
//					char date[32];
//					char start_timestamp[32];
//					char end_timestamp[32];
//					int year,month,day;
//					bool more_demand = true;
//					bool first_round = true;
//
//                    //while(more_demand)
//					//{
//					memset(&date, 0, sizeof(date));
//					memset(&start_timestamp, 0, sizeof(start_timestamp));
//					memset(&end_timestamp, 0, sizeof(end_timestamp));
//					year=month=day=0;
//					ProcessManager::g_db_reverse_dns_interface.GetLastProcessedDateForReverseDNS((*thread_data.p_projects)[i].m_project_name.c_str(),date);
//					if(strlen(date) == 0) //no previous results in processed database
//					{
//						ProcessManager::g_db_reverse_dns_interface.GetFirstReverseDNSDataInsertionTimestamp(
//							(*thread_data.p_projects)[i].m_project_name.c_str(),date);
//						if(strlen(date) == 0) //no results in raw database
//							continue;
//						else
//						{
//							/*
//							char tmp[32];
//							strncpy(tmp, date, 4);
//							year = atoi(tmp);
//							char* ptr = date;
//							ptr+=4;
//							strncpy(tmp, ptr, 2);
//							tmp[2]='\0';
//							month = atoi(tmp);
//							ptr+=2;
//							strncpy(tmp, ptr, 2);
//							tmp[2]='\0';
//							day = atoi(tmp);							
//							strcpy(start_timestamp, date);
//							*/
//							sscanf(date, "%4d%2d%2d", &year, &month, &day);
//							strcpy(start_timestamp, date);
//						}
//					}
//					else
//					{
//						sscanf(date, "%d-%d-%d", &year, &month, &day);
//						CTime time(year, month, day, 0,0,0);
//						time += CTimeSpan(1,0,0,0);
//						if(time.GetHour() == 23)
//							time += CTimeSpan(0,1,0,0); //winter time
//						else if(time.GetHour() == 1)
//							time -= CTimeSpan(0,1,0,0); // summer time
//						strcpy(start_timestamp, time.Format("%Y%m%d%H%M%S"));
//						year = time.GetYear();
//						month = time.GetMonth();
//						day = time.GetDay();
//					}
//					while(more_demand)
//					{
//						if(!first_round)
//						{
//							CTime time(year, month, day, 0,0,0);
//							time += CTimeSpan(1,0,0,0);
//							if(time.GetHour() == 23)
//								time += CTimeSpan(0,1,0,0); //winter time
//							else if(time.GetHour() == 1)
//								time -= CTimeSpan(0,1,0,0); // summer time
//							strcpy(start_timestamp, time.Format("%Y%m%d%H%M%S"));
//							year = time.GetYear();
//							month = time.GetMonth();
//							day = time.GetDay();
//						}
//						//check to see if we have reached today, then stop demand process for this project
//						if(year == current_time.GetYear() && month == current_time.GetMonth() && day == current_time.GetDay())
//							break;
//
//						//get the end timestamp
//						sprintf(end_timestamp, "%d%.2d%.2d%.2d%.2d%.2d", year,month,day,23,59,59);
//						
//						//get supply query and insert to processed database
//						char on_date[32];
//						sprintf(on_date, "%d-%.2d-%.2d", year, month, day);
//						int ret = ProcessManager::g_db_reverse_dns_interface.ProcessReverseDNS((*thread_data.p_projects)[i].m_project_name.c_str(),
//											start_timestamp, end_timestamp, on_date);
//						if(ret == -1)// lost connection
//						{
//#ifdef DC2
//							while(! (ProcessManager::g_db_reverse_dns_interface.OpenSupplyConnection("dcmaster.mediadefender.com","onsystems",
//										"ebertsux37","DCDATA2")))
//#else
//							while(! (ProcessManager::g_db_reverse_dns_interface.OpenSupplyConnection("dcmaster.mediadefender.com","onsystems",
//										"ebertsux37","DCDATA")))
//#endif
//							{
//								Sleep(1000);
//							}
//							ProcessManager::g_db_reverse_dns_interface.ProcessReverseDNS((*thread_data.p_projects)[i].m_project_name.c_str(),
//											start_timestamp, end_timestamp, on_date);
//						}
//						else if(ret == -2)//table full
//						{
//							table_full=true;
//							break;
//						}
//						first_round = false;
//					}
//
//					thread_data.m_reverse_dns_progress = (int)((float)i/(float)thread_data.p_projects->size()*(float)100);
//					if(table_full)
//						break;
//					//check if the program is exiting
//					new_event = WaitForMultipleObjects(num_events,events,wait_all,0);
//					if(new_event==0)
//					{
//						ResetEvent(events[event]);
//						break;
//					}
//				}
//				thread_data.m_reverse_dns_progress = (int)((float)i/(float)thread_data.p_projects->size()*(float)100);
//				if(new_event==0)
//				{
//					singleLock.Unlock();
//					break;
//				}
//				singleLock.Unlock();
//				break;
//			}
//		}
//	}
//
//	// Close the events
//	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
//	{
//		CloseHandle(events[i]);
//	}
//	thread_data.m_reverse_dns_progress=100;
//	if(event!=0)
//		::PostMessage(hwnd, WM_PROCESS_MANAGER_REVERSE_DNS_DONE,0,0);
//	
//	return 0;	// exit the thread
//}

CString GetDemandTableName(CString project)
{
	if(project.GetLength() > 50)
	{
		project.Truncate(50);
	}

	CString table = "DEMAND_TABLE_";
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

	ProcessManager* manager = (ProcessManager*)pParam;
	HWND hwnd=manager->p_parent->m_dlg.GetSafeHwnd();

	PostMessage(hwnd,WM_INIT_PROCESS_MANAGER_DB_MAINTENANCE_THREAD_DATA,0,0);
	
	UINT demand_records_delete = 0;
	UINT supply_records_delete = 0;

	//delete old raw supply data
	vector<CString> supply_tables;
	while(!ProcessManager::g_processed_db_interface.GetAllProcessedSupplyTables(supply_tables))
		Sleep(5*1000);
	
	for(UINT i=0; i<supply_tables.size(); i++)
	{
		manager->m_maintenance_progress = ((float)(i+1)/(float)supply_tables.size())*(float)100;
		::PostMessage(hwnd, WM_MAINTENANCE_PROCESS_PROJECT, (WPARAM)(const char*)supply_tables[i],0);
		supply_records_delete += ProcessManager::g_processed_db_interface.DeleteProcessedIPs(supply_tables[i]);
	}

	if(supply_records_delete > 0)
	{
		ProcessManager::g_db_supply_interface.OptimizeTables(supply_tables);
	}

	::PostMessage(hwnd, WM_PROCESS_MANAGER_DB_MAINTENANCE_DONE,demand_records_delete,supply_records_delete);

	return 0;	// exit the thread
}

//
//
//
void ProcessManager::StartProcessRawData(DataBaseInfo& db_info)
{
	m_db_info = db_info;
#ifndef NO_DEMAND
	//Demand Process
	if(!m_is_demand_processing)
	{
		m_is_demand_processing = true;
		//db connection to processed database
		if(g_db_demand_interface.OpenDemandConnection(db_info.m_db_host.c_str(),db_info.m_db_user.c_str(),
										db_info.m_db_password.c_str(),"gnutella_raw_demand"))
		{
			//db connection to raw data database
			if(g_db_demand_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","midata"))
				m_demand_thread=AfxBeginThread(ProcessManagerDemandThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_LOWEST);
			else
				m_is_demand_processing = false;
		}
		else
			m_is_demand_processing = false;
	}
#endif
#ifndef NO_SUPPLY
	//Supply Process
	if(!m_is_supply_processing)
	{
		m_is_supply_processing = true;
		//db connection to processed database
		if(g_db_supply_interface.OpenSupplyConnection(db_info.m_db_host.c_str(),db_info.m_db_user.c_str(),
										db_info.m_db_password.c_str(),"midata_ip_gnutella"))
		{
			//db connection to raw data database
			if(g_db_supply_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","gnutella_raw_supply"))
				m_supply_thread=AfxBeginThread(ProcessManagerSupplyThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_LOWEST);
			else
				m_is_supply_processing = false;
		}
		else
			m_is_supply_processing = false;
	}
#endif
//	//Reverse DNS Process
//	if(!m_is_reverse_dns_processing)
//	{
//		v_projects = keywords;
//		m_is_reverse_dns_processing = true;
//		//db connection to processed database
//#ifdef DC2
//		if(g_db_reverse_dns_interface.OpenSupplyConnection(db_info.m_db_host.c_str(),db_info.m_db_user.c_str(),
//										db_info.m_db_password.c_str(),"DCDATA2"))
//#else
//		if(g_db_reverse_dns_interface.OpenSupplyConnection(db_info.m_db_host.c_str(),db_info.m_db_user.c_str(),
//										db_info.m_db_password.c_str(),"DCDATA"))
//#endif
//		{
//#ifdef _DEBUG	
//			if(g_db_reverse_dns_interface.OpenRawDataConnection("63.221.232.35","onsystems","ebertsux37","gnutella_raw_supply"))
//#else
//			//db connection to raw data database
//			if(g_db_reverse_dns_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","gnutella_raw_supply"))
//#endif
//				AfxBeginThread(ProcessManagerReverseDNSThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_LOWEST);
//			else
//				m_is_reverse_dns_processing = false;
//		}
//		else
//			m_is_reverse_dns_processing = false;
//	}
}

//
//
//
void ProcessManager::InitDemandThreadData(WPARAM wparam,LPARAM lparam)
{
	p_demand_critical_section=(CCriticalSection *)wparam;
	p_demand_thread_data=(ProcessManagerThreadData *)lparam;

	if(p_demand_critical_section!=NULL)
	{
		CSingleLock singleLock(p_demand_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			p_demand_thread_data->Clear();
			//p_demand_thread_data->p_projects = &v_projects;
			singleLock.Unlock();
		}
		SetEvent(p_demand_thread_data->m_events[1]);	// start the process
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
			//p_supply_thread_data->p_projects = &v_projects;
			singleLock.Unlock();
		}
		SetEvent(p_supply_thread_data->m_events[1]);	// start the process
	}
}

//
//
//
//void ProcessManager::InitReverseDNSThreadData(WPARAM wparam,LPARAM lparam)
//{
//	p_reverse_dns_critical_section=(CCriticalSection *)wparam;
//	p_reverse_dns_thread_data=(ProcessManagerThreadData *)lparam;
//
//	if(p_reverse_dns_critical_section!=NULL)
//	{
//		CSingleLock singleLock(p_reverse_dns_critical_section);
//		singleLock.Lock();
//		if(singleLock.IsLocked())
//		{
//			p_reverse_dns_thread_data->Clear();
//			//p_reverse_dns_thread_data->p_projects = &v_projects;
//			singleLock.Unlock();
//		}
//		SetEvent(p_reverse_dns_thread_data->m_events[1]);	// start the process
//	}
//}
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
//void ProcessManager::DemandProcessFinished()
//{
//	m_is_demand_processing = false;
//	g_db_demand_interface.CloseDemandConnection();
//	g_db_demand_interface.CloseRawDataConnection();
//	
//	if( !m_is_demand_processing && !m_is_supply_processing /*&& !m_is_reverse_dns_processing*/ )
//		m_is_all_done = true;
//	p_demand_thread_data = NULL;
//
//	//if(m_is_all_done)
//	//	DBMaintenance(); //delete old raw data records
//}
//
////
////
////
//void ProcessManager::SupplyProcessFinished()
//{
//	m_is_supply_processing = false;
//	g_db_supply_interface.CloseSupplyConnection();
//	g_db_supply_interface.CloseRawDataConnection();
//	
//	if( !m_is_demand_processing && !m_is_supply_processing /*&& !m_is_reverse_dns_processing*/ )
//		m_is_all_done = true;
//	p_supply_thread_data = NULL;
//
//	//if(m_is_all_done)
//	//	DBMaintenance(); //delete old raw data records
//}

//
//
//
//void ProcessManager::ReverseDNSProcessFinished()
//{
//	m_is_reverse_dns_processing = false;
//	g_db_reverse_dns_interface.CloseSupplyConnection();
//	g_db_reverse_dns_interface.CloseRawDataConnection();
//	
//	if( !m_is_demand_processing && !m_is_supply_processing && !m_is_reverse_dns_processing )
//		m_is_all_done = true;
//	p_reverse_dns_thread_data = NULL;
//
//	//if(m_is_all_done)
//	//	DBMaintenance(); //delete old raw data records
//}

//
//
//
void ProcessManager::MaintenanceFinished()
{
	g_processed_db_interface.CloseRawDataConnection();

	m_is_maintaining = false;
	//p_parent->DBMaintenanceFinished();
	m_is_all_done=true;
}

//
//
//
void ProcessManager::Reset()
{
	//if(!m_is_demand_processing && !m_is_supply_processing /*&& !m_is_reverse_dns_processing*/)
		m_is_all_done = false;
		m_maintenance_progress=0;
}

//
//
//
void ProcessManager::GetProcessProgress(int& demand_progress, int& supply_progress/*, int& dns*/)
{
	if(p_demand_thread_data != NULL)
	{
		demand_progress = p_demand_thread_data->m_demand_progress;
	}
	if(p_supply_thread_data != NULL)
	{
		supply_progress = p_supply_thread_data->m_supply_progress;
	}
	//if(p_reverse_dns_thread_data != NULL)
	//{
	//	dns = p_reverse_dns_thread_data->m_reverse_dns_progress;
	//}
}

//
//
// delete old raw data from data base
//void ProcessManager::DBMaintenance(vector<ProjectKeywords>& keywords)
//{
//	v_projects = keywords;
//	Reset();
//	if(!m_is_maintaining)
//	{
//		m_is_maintaining = true;
//		bool connected = false;
//		int trial = 0;
//		while(!connected)
//		{
//			//db connection to raw data database
//			connected = g_db_supply_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","gnutella_raw_supply");
//			connected = g_db_demand_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","gnutella_raw_demand");
//			connected = g_db_hash_interface.OpenRawDataConnection("localhost","onsystems","ebertsux37","gnutella_hash");
//			trial++;
//			if(trial > 10)
//				break;
//			Sleep(500);
//		}
//		if(connected)
//		{
//			p_parent->DBMaintenanceReadyToStart();
//			AfxBeginThread(DBMaintenanceThreadProc,(LPVOID)this/*p_parent->m_dlg.GetSafeHwnd()*/,THREAD_PRIORITY_LOWEST);
//		}
//		else
//			m_is_maintaining = false;
//	}
//}
//
////
// delete old raw data from data base
void ProcessManager::DBMaintenance()
{
	Reset();
	if(!m_is_maintaining)
	{
		m_is_maintaining = true;
		bool connected = false;
		int trial = 0;
		while(!connected)
		{
			//db connection to raw data database
			connected = g_processed_db_interface.OpenSupplyConnection(m_db_info.m_db_host.c_str(),m_db_info.m_db_user.c_str(),
										m_db_info.m_db_password.c_str(),"midata_ip_gnutella");
			trial++;
			if(trial > 10)
				break;
			Sleep(500);
		}
		if(connected)
		{
			//p_parent->DBMaintenanceReadyToStart();
			AfxBeginThread(DBMaintenanceThreadProc,(LPVOID)this,THREAD_PRIORITY_LOWEST);
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

//
//
//
void ProcessManager::KeywordUpdated(vector<ProjectKeywords>& projects)
{
	v_projects = projects;
}

//
//
//
void ProcessManager::ProjectIsReadyToBeProcessed(UINT project_id)
{
	if(p_supply_critical_section!=NULL)
	{
		UINT i;
		for(i=0; i<v_projects.size();i++)
			if(v_projects[i].m_id==project_id)
				break;
		if(i<v_projects.size())
		{

			CSingleLock singleLock(p_supply_critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				ProjectKeywords* project = new ProjectKeywords();
				*project = v_projects[i];
				p_supply_thread_data->v_projects.push_back(project);
				singleLock.Unlock();
				SetEvent(p_supply_thread_data->m_events[1]);	// start the process
			}
		}
	}
}
