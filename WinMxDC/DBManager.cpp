#include "StdAfx.h"
#include "dbmanager.h"
#include <afxmt.h>	// for CCriticalSection
#include "WinMxDcDll.h"

//DBInterface DBManager::g_demand_db_interface;
DBInterface DBManager::g_supply_db_interface;

DBManager::DBManager(void)
{
//	p_demand_critical_section=NULL;
	p_supply_critical_section=NULL;
//	m_demand_thread=NULL;
	m_supply_thread=NULL;
//	m_ready_to_write_to_demand_db=false;
	m_ready_to_write_to_supply_db=false;
	m_db_is_maintaining=false;
	bool connected = false;
	m_last_db_written_at = CTime::GetCurrentTime();

	while(!connected)
	{
#ifdef _DEBUG
		connected = g_supply_db_interface.OpenSupplyConnection("206.161.141.47","onsystems","ebertsux37","winmx_raw_supply");
#else
		connected = g_supply_db_interface.OpenSupplyConnection("127.0.0.1","onsystems","ebertsux37","winmx_raw_supply");
#endif
		Sleep(500);
	}
}

//
//
//
DBManager::~DBManager(void)
{
/*	if(p_demand_critical_section!=NULL)
	{
		SetEvent(m_demand_thread_data.m_events[0]);	// kill thread
	}
	if(p_supply_critical_section!=NULL)
	{
		SetEvent(m_supply_thread_data.m_events[0]);	// kill thread
	}
*/
//	g_demand_db_interface.CloseDemandConnection();
	g_supply_db_interface.CloseSupplyConnection();
/*	
	vector<Query *>::iterator iter = v_project_queries.begin();
	if(iter != v_project_queries.end())
	{
		delete *(iter);
		v_project_queries.erase(iter);
	}
*/	vector<QueryHit *>::iterator hit_iter = v_project_query_hits.begin();
	if(hit_iter != v_project_query_hits.end())
	{
		delete *(hit_iter);
		v_project_query_hits.erase(hit_iter);
	}

}


//
//
//
void WriteProjectSupplyToDatabase(QueryHit* project_query_hit,const char* today_date)
{
	DBManager::g_supply_db_interface.InsertRawSupply(*project_query_hit);
}

//
//
//
void WriteProjectSupplyToDatabase(vector<QueryHit>& project_query_hits,const char* today_date)
{
	DBManager::g_supply_db_interface.InsertRawSupply(project_query_hits);
}

//
//
//
UINT RawDataFromFileToDBThreadProc(LPVOID pParam)
{
	CTime today_date = CTime::GetCurrentTime();
	CString today_date_string = today_date.Format("%Y_%m_%d_");	DBInterface db_interface;
	bool connected = false;

	while(!connected)
	{
#ifdef _DEBUG
		connected = db_interface.OpenSupplyConnection("206.161.141.47","onsystems","ebertsux37","winmx_raw_supply");
#else	
		connected = db_interface.OpenSupplyConnection("127.0.0.1","onsystems","ebertsux37","winmx_raw_supply");
#endif
		Sleep(1000);
	}
	CFile raw_query_hits_file;
	if(raw_query_hits_file.Open("raw_supplies.dat",CFile::modeRead|CFile::typeBinary
		|CFile::shareDenyRead|CFile::shareDenyWrite) != 0)
	{
		UINT data_size = 0;
		vector<QueryHit*> query_hits_cache;
		while(raw_query_hits_file.Read(&data_size, sizeof(UINT)))
		{
			char* buf = new char[data_size];
			raw_query_hits_file.Read(buf, data_size);

			QueryHit* hit = new QueryHit();
			hit->ReadFromBuffer(buf);
			query_hits_cache.push_back(hit);

			if(query_hits_cache.size() >= 100000)
			{
				while(query_hits_cache.size() > 0 )
				{
					// Write out to database for each hit
					//sorting the QH so that the same project groups together
					vector<QueryHit> query_hits;
					vector<QueryHit*>::iterator iter = query_hits_cache.begin();
					//CString project = (*iter)->m_project;
					unsigned int project_id = (*iter)->m_project_id;
					query_hits.push_back(*(query_hits_cache[0]));
					delete *iter;
					query_hits_cache.erase(iter);
					iter = query_hits_cache.begin();
					while(iter != query_hits_cache.end() && query_hits.size() < 10000)
					{
						//if(strcmp(project, (*iter)->m_project)==0)
						if(project_id == (*iter)->m_project_id)
						{
							query_hits.push_back(*(*iter));
							delete *iter;
							query_hits_cache.erase(iter);
						}
						else
							iter++;
					}
					db_interface.InsertRawSupply(query_hits);
				}
			}
			delete [] buf;
		}
		while(query_hits_cache.size() > 0)
		{
			// Write out to database for each hit
			//sorting the QH so that the same project groups together
			vector<QueryHit> query_hits;
			vector<QueryHit*>::iterator iter = query_hits_cache.begin();
			//CString project = (*iter)->m_project;
			unsigned int project_id = (*iter)->m_project_id;
			query_hits.push_back(*(query_hits_cache[0]));
			delete *iter;
			query_hits_cache.erase(iter);
			iter = query_hits_cache.begin();
			while(iter != query_hits_cache.end())
			{
				//if(strcmp(project, (*iter)->m_project)==0)
				if(project_id == (*iter)->m_project_id)
				{
					query_hits.push_back(*(*iter));
					delete *iter;
					query_hits_cache.erase(iter);
				}
				else
					iter++;
			}
			db_interface.InsertRawSupply(query_hits);
		}
		raw_query_hits_file.Close();
		remove("raw_supplies.dat");
	}
	db_interface.CloseSupplyConnection();
	return 0;	// exit the thread
}

//
//
//
UINT DBManagerSupplyThreadProc(LPVOID pParam)
{
	UINT i;

	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	DBManagerThreadData thread_data;
	vector<QueryHit *> project_query_hits;

	// Create the events
	HANDLE events[DBManagerThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data.m_events[i]=events[i];
	}

	thread_data.p_project_query_hits=&project_query_hits;
	
	PostMessage(hwnd,WM_INIT_DB_MANAGER_SUPPLY_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	PostMessage(hwnd,WM_READY_TO_WRITE_DATA_TO_SUPPLY_DB,1,1);	// the thread is now ready to write into the database

	vector<QueryHit *> project_query_hits_cache;

	// Start the thread
	DWORD num_events=2;
	BOOL wait_all=FALSE;
	DWORD timeout=INFINITE;
	DWORD event;	// which event fired

	CTime today_date = CTime::GetCurrentTime();
	CString today_date_string = today_date.Format("%Y_%m_%d_");	DBInterface db_interface;

	while(1)
	{
		today_date = CTime::GetCurrentTime();
		today_date_string = today_date.Format("%Y_%m_%d_");	DBInterface db_interface;
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
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				ResetEvent(events[event]);	// reset event

				// Cache the hits
				for(i=0;i<project_query_hits.size();i++)
				{
					project_query_hits_cache.push_back(project_query_hits[i]);
				}

				project_query_hits.clear();	// don't free the memory yet
				singleLock.Unlock();
			}
		}

		// If there are cached hits to process, then process them
		while(project_query_hits_cache.size() > 0)
		{
			//in each loop, it goes throught the vector and finds all entries belongs to the same project
			//and put them into query_hits vector and passes them the DB insert function

			vector<QueryHit> query_hits;
			vector<QueryHit*>::iterator iter = project_query_hits_cache.begin();
			//CString project = (*iter)->m_project;
			unsigned int project_id = (*iter)->m_project_id;
			query_hits.push_back(*(project_query_hits_cache[0]));
			delete *iter;
			project_query_hits_cache.erase(iter);
			iter = project_query_hits_cache.begin();
			while(iter != project_query_hits_cache.end() && query_hits.size() < 50000)
			{
				//if(strcmp(project, (*iter)->m_project)==0)
				if(project_id == (*iter)->m_project_id)
				{
					query_hits.push_back(*(*iter));
					delete *iter;
					project_query_hits_cache.erase(iter);
				}
				else
					iter++;
			}
			WriteProjectSupplyToDatabase(query_hits,today_date_string);
		}

		// We've written all of the cached data to file...tell the logfile manager that we are ready to write more logfile data to file
		PostMessage(hwnd,WM_READY_TO_WRITE_DATA_TO_SUPPLY_DB,0,0);
	}

	// Close the events
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		CloseHandle(events[i]);
	}
	TRACE("Terminating DBManagerSupplyThreadProc\n");

	return 0;	// exit the thread
}

//
//
//
void DBManager::InitParent(WinMxDcDll *parent)
{
	Manager::InitParent(parent);
	m_supply_thread = AfxBeginThread(DBManagerSupplyThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_NORMAL);
}

//
//
//
void DBManager::InitSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
	p_supply_critical_section=(CCriticalSection *)wparam;
	m_supply_thread_data=*((DBManagerThreadData *)lparam);
}

//
//
//
void DBManager::PushQueryHitData(vector<QueryHit> &query_hits)
{
	//this timestamp is used for ReadyToReconnect() function
	//if we are not receiving heavy supplies from supernodes, then we are ready to reconnect to all supernodes
	if(query_hits.size() > 100)
		m_last_db_written_at = CTime::GetCurrentTime();
	if(!m_db_is_maintaining)
	{
		for(UINT i=0;i<query_hits.size();i++)
		{
			QueryHit* hit = new QueryHit(query_hits[i]);
			v_project_query_hits.push_back(hit);
		}
		
		// Check to see if the thread is ready to write data to file
		if(m_ready_to_write_to_supply_db)
		{
			WriteDataToSupplyDatabase();
		}
	}
	else
	{
		for(UINT i=0;i<query_hits.size();i++)
		{
			UINT data_size = query_hits[i].GetBufferLength();
			m_raw_supply_file.Write(&data_size, sizeof(UINT));
			char* buf = new char[data_size];
			query_hits[i].WriteToBuffer(buf);
			m_raw_supply_file.Write(buf, data_size);
			delete [] buf;
		}
	}

}

//
//
//
void DBManager::ReadyToWriteSupplyDataToDatabase(WPARAM wparam,LPARAM lparam)
{
	m_ready_to_write_to_supply_db=true;

	WriteDataToSupplyDatabase();	// will write out data to db(if any)
}

//
//
//
void DBManager::WriteDataToSupplyDatabase()
{
	UINT i;

	if(p_supply_critical_section==NULL)
	{
		return;
	}

	// If there are no project query hits to send to the thread to be written to database, then return
	if(v_project_query_hits.size()==0)
	{
		return;
	}

	if(m_ready_to_write_to_supply_db==false)
	{
		return;
	}

	m_ready_to_write_to_supply_db=false;

	// Tell the thread to connect to these hosts
//	p_parent->m_log_window_manager.Log("LogfileManager::TimerHasFired() - Before LOCK\n",0,false,true);	// black italic
	CSingleLock singleLock(p_supply_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{

		// Supply
		if(v_project_query_hits.size()>0)
		{
			// If the vector is empty, then just copy. Else, add the correct items to the appropriate project
			if(m_supply_thread_data.p_project_query_hits->size()==0)
			{
				*m_supply_thread_data.p_project_query_hits=v_project_query_hits;
			}
			else
			{
				// For each of the project query hits, find the project in the thread, and if it can't be found, then create it
				for(i=0;i<v_project_query_hits.size();i++)
				{
					m_supply_thread_data.p_project_query_hits->push_back(v_project_query_hits[i]);
				}
			}

			v_project_query_hits.clear();	// clear the vector
		}
		SetEvent(m_supply_thread_data.m_events[1]);	// vector data
		singleLock.Unlock();
	}
//	p_parent->m_log_window_manager.Log("LogfileManager::TimerHasFired() - After LOCK\n",0,false,true);	// black italic
}

//
//
//
void DBManager::ReportDBStatus(UINT& query_hit_size)
{
	query_hit_size = (UINT)v_project_query_hits.size();
}

//
//
//
void DBManager::DBMaintenanceReadyToStart(void)
{
	m_db_is_maintaining = true;
	WriteAllCacheRawDataToDisk();
	m_raw_supply_file.Open("raw_supplies.dat",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary
			|CFile::shareDenyWrite|CFile::shareDenyRead);
	m_raw_supply_file.SeekToEnd();
}

//
//
//
void DBManager::DBMaintenanceFinished(void)
{
	m_db_is_maintaining = false;
	m_raw_supply_file.Close();
	//Create a new thread to write all the raw data which saved on HDD to the database
	AfxBeginThread(RawDataFromFileToDBThreadProc,NULL,THREAD_PRIORITY_BELOW_NORMAL);
}

//
//
//
void DBManager::WriteAllCacheRawDataToDisk(void)
{
	CFile raw_query_hit_file;
	if(raw_query_hit_file.Open("raw_supplies.dat",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary
		|CFile::shareDenyWrite|CFile::shareDenyRead) != 0)
	{
		raw_query_hit_file.SeekToEnd();

		for(UINT i=0;i<v_project_query_hits.size();i++)
		{
			UINT data_size = v_project_query_hits[i]->GetBufferLength();
			raw_query_hit_file.Write(&data_size, sizeof(UINT));
			char* buf = new char[data_size];
			v_project_query_hits[i]->WriteToBuffer(buf);
			raw_query_hit_file.Write(buf, data_size);
			delete [] buf;
			delete v_project_query_hits[i];
		}
		raw_query_hit_file.Close();
		v_project_query_hits.clear();
	}
}

//
//
//
void DBManager::TerminateThread()
{
	if(p_supply_critical_section!=NULL)
	{
		SetEvent(m_supply_thread_data.m_events[0]);	// kill thread
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
void DBManager::OnOneMinuteTimer()
{
}

//
//
//
bool DBManager::ReadyToReconnect()
{
	CTimeSpan ts = CTime::GetCurrentTime() - m_last_db_written_at;
	if(ts.GetTotalSeconds() >= 60)
		return true;
	else
		return false;
}