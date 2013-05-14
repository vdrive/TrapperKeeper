#include "StdAfx.h"
#include "dbmanager.h"
#include <afxmt.h>	// for CCriticalSection
#include "GnutellaProtectorDll.h"

DBInterface DBManager::g_db_interface;
DBManager::DBManager(void)
{
	p_critical_section=NULL;
	m_thread = NULL;
	m_ready_to_write_to_db=false;
	m_db_is_maintaining=false;
	bool connected = false;
#ifndef NO_PROCESSING
	while(!connected)
	{
		connected = g_db_interface.OpenConnection("127.0.0.1","onsystems","ebertsux37",GNUTELLA_DATABASE);
		Sleep(500);
	}
#endif
}

//
//
//
DBManager::~DBManager(void)
{
	g_db_interface.CloseConnection();
	
	vector<Query *>::iterator iter = v_project_queries.begin();
	if(iter != v_project_queries.end())
	{
		delete *(iter);
		v_project_queries.erase(iter);
	}
}

//
//
//
void WriteProjectDemandToDatabase(Query *project_queries)
{
	CString timestamp = project_queries->m_timestamp.Format("%Y%m%d%H%M%S");
	
	DBManager::g_db_interface.InsertGnutellaSPD(GNUTELLA_SPD_TABLE,
		project_queries->m_project.c_str(), project_queries->m_track,
		project_queries->m_spoofs_sent, project_queries->m_poisons_sent, project_queries->m_dists_sent, timestamp);
}

//
//
//
UINT RawDataFromFileToDBThreadProc(LPVOID pParam)
{
	DBInterface db_interface;
	bool connected = false;
	while(!connected)
	{
		connected = db_interface.OpenConnection("127.0.0.1","onsystems","ebertsux37",GNUTELLA_DATABASE);
		Sleep(1000);
	}

	CFile raw_queries_file;
	if(raw_queries_file.Open("raw_demands.dat",CFile::modeRead|CFile::typeBinary
		|CFile::shareDenyRead|CFile::shareDenyWrite) != 0)
	{
		UINT data_size=0;
		while(raw_queries_file.Read(&data_size, sizeof(UINT)))
		{
			char* buf = new char[data_size];
			raw_queries_file.Read(buf, data_size);

			Query query;
			query.ReadFromBuffer(buf);

			CString timestamp = query.m_timestamp.Format("%Y%m%d%H%M%S");	
			db_interface.InsertGnutellaSPD(GNUTELLA_SPD_TABLE,
					query.m_project.c_str(), query.m_track,
					query.m_spoofs_sent, query.m_poisons_sent, query.m_dists_sent, timestamp);
			delete [] buf;
		}
		raw_queries_file.Close();
		remove("raw_demands.dat");
	}

	db_interface.CloseConnection();
	return 0;	// exit the thread
}

//
//
//
UINT DBManagerThreadProc(LPVOID pParam)
{
	UINT i;

	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	DBManagerThreadData thread_data;
	vector<Query *> project_queries;

	// Create the events
	HANDLE events[DBManagerThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data.m_events[i]=events[i];
	}

	thread_data.p_project_queries=&project_queries;
	
	PostMessage(hwnd,WM_INIT_DB_MANAGER_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	PostMessage(hwnd,WM_READY_TO_WRITE_DATA_TO_DB,1,1);	// the thread is now ready to write into the database

	vector<Query *> project_queries_cache;

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

		// There is new data to be written to database
		if(event==1)
		{
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				ResetEvent(events[event]);	// reset event

				// Cache the queries
				for(i=0;i<project_queries.size();i++)
				{
					project_queries_cache.push_back(project_queries[i]);
				}

				project_queries.clear();	// don't free the memory yet

				singleLock.Unlock();
			}
		}

		// If there are cached queries to process, then process them
		if(project_queries_cache.size()>0)
		{
			// Write out to database for each quries
			for(i=0;i<project_queries_cache.size();i++)
			{
				WriteProjectDemandToDatabase(project_queries_cache[i]);
				delete project_queries_cache[i];	// now free the memory
			}
			
			project_queries_cache.clear();
		}

		// We've written all of the cached data to file...tell the logfile manager that we are ready to write more logfile data to file
		PostMessage(hwnd,WM_READY_TO_WRITE_DATA_TO_DB,0,0);
	}

	// Close the events
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		CloseHandle(events[i]);
	}

	TRACE("Terminating DBManagerThreadProc\n");
	return 0;	// exit the thread
}


//
//
//
void DBManager::InitParent(GnutellaProtectorDll *parent)
{
	Manager::InitParent(parent);

	m_thread = AfxBeginThread(DBManagerThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_BELOW_NORMAL);
}

//
//
//
void DBManager::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	m_thread_data=*((DBManagerThreadData *)lparam);
}

//
//
//
void DBManager::PushQueryData(vector<Query> &querie)
{
#ifndef NO_PROCESSING
	if(!m_db_is_maintaining)
	{
		for(UINT i=0;i<querie.size();i++)
		{
			Query* q = new Query(querie[i]);
			v_project_queries.push_back(q);
		}
		// Check to see if the thread is ready to write data to file
		if(m_ready_to_write_to_db)
		{
			WriteDataToDatabase();
		}
	}
	else
	{
		CFile raw_queries_file;
		if(raw_queries_file.Open("raw_demands.dat",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary
			|CFile::shareDenyWrite|CFile::shareDenyRead) != 0)
		{
			raw_queries_file.SeekToEnd();
			for(UINT i=0;i<querie.size();i++)
			{
				UINT data_size = querie[i].GetBufferLength();
				raw_queries_file.Write(&data_size, sizeof(UINT));
				char* buf = new char[data_size];
				querie[i].WriteToBuffer(buf);
				raw_queries_file.Write(buf, data_size);
				delete [] buf;
			}
			raw_queries_file.Close();
		}
	}
#endif
}

//
//
//
void DBManager::ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam)
{
	m_ready_to_write_to_db=true;

	WriteDataToDatabase();	// will write out data to db(if any)
}

//
//
//
void DBManager::WriteDataToDatabase()
{
	UINT i;

	if(p_critical_section==NULL)
	{
		return;
	}

	// If there are no project query hits to send to the thread to be written to database, then return
	if(v_project_queries.size()==0)
	{
		return;
	}

	if(m_ready_to_write_to_db==false)
	{
		return;
	}

	m_ready_to_write_to_db=false;

	// Tell the thread to connect to these hosts
//	p_parent->m_log_window_manager.Log("LogfileManager::TimerHasFired() - Before LOCK\n",0,false,true);	// black italic
	CSingleLock singleLock(p_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		// Demand
		if(v_project_queries.size()>0)
		{
			// If the vector is empty, then just copy. Else, add the correct items to the appropriate project
			if(m_thread_data.p_project_queries->size()==0)
			{
				*m_thread_data.p_project_queries=v_project_queries;
			}
			else
			{
				// For each of the project queries, find the project in the thread, and if it can't be found, then create it
				for(i=0;i<v_project_queries.size();i++)
				{
					m_thread_data.p_project_queries->push_back(v_project_queries[i]);
				}
			}

			v_project_queries.clear();	// clear the vector
		}

		SetEvent(m_thread_data.m_events[1]);	// vector data

		singleLock.Unlock();
	}
//	p_parent->m_log_window_manager.Log("LogfileManager::TimerHasFired() - After LOCK\n",0,false,true);	// black italic

}

//
//
//
void DBManager::ReportDBStatus(UINT& query_size)
{
	query_size = (UINT)v_project_queries.size();
}

//
//
//
void DBManager::DBMaintenanceReadyToStart(void)
{
	m_db_is_maintaining = true;
	WriteAllCacheRawDataToDisk();
}

//
//
//
void DBManager::DBMaintenanceFinished(void)
{
	m_db_is_maintaining = false;
/*	
	CFile raw_queries_file;
	if(raw_queries_file.Open("raw_demands.dat",CFile::modeRead|CFile::typeBinary
		|CFile::shareDenyRead|CFile::shareDenyWrite) != 0)
	{
		UINT data_size=0;
		while(raw_queries_file.Read(&data_size, sizeof(UINT)))
		{
			char* buf = new char[data_size];
			raw_queries_file.Read(buf, data_size);

			Query* query = new Query();
			query->ReadFromBuffer(buf);
			v_project_queries.push_back(query);
			delete [] buf;
		}
		raw_queries_file.Close();
		remove("raw_demands.dat");
	}

	CFile raw_query_hits_file;
	if(raw_query_hits_file.Open("raw_supplies.dat",CFile::modeRead|CFile::typeBinary
		|CFile::shareDenyRead|CFile::shareDenyWrite) != 0)
	{
		UINT data_size = 0;
		while(raw_query_hits_file.Read(&data_size, sizeof(UINT)))
		{
			char* buf = new char[data_size];
			raw_query_hits_file.Read(buf, data_size);

			QueryHit* hit = new QueryHit();
			hit->ReadFromBuffer(buf);
			v_project_query_hits.push_back(hit);
			delete [] buf;
		}
		raw_query_hits_file.Close();
		remove("raw_supplies.dat");
	}
*/
	//Create a new thread to write all the raw data which saved on HDD to the database
	AfxBeginThread(RawDataFromFileToDBThreadProc,NULL,THREAD_PRIORITY_LOWEST);
}

//
//
//
void DBManager::WriteAllCacheRawDataToDisk(void)
{
	CFile raw_queries_file;
	if(raw_queries_file.Open("raw_demands.dat",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary
		|CFile::shareDenyWrite|CFile::shareDenyRead) != 0)
	{
		raw_queries_file.SeekToEnd();
		for(UINT i=0;i<v_project_queries.size();i++)
		{
			UINT data_size = v_project_queries[i]->GetBufferLength();
			raw_queries_file.Write(&data_size, sizeof(UINT));
			char* buf = new char[data_size];
			v_project_queries[i]->WriteToBuffer(buf);
			raw_queries_file.Write(buf, data_size);
			delete [] buf;
			delete v_project_queries[i];
		}
		raw_queries_file.Close();
		v_project_queries.clear();
	}
}

//
//
//
void DBManager::TerminateThread()
{
	if(p_critical_section!=NULL)
	{
		SetEvent(m_thread_data.m_events[0]);	// kill thread
	}
	if(m_thread!=NULL)
	{
		if(WaitForSingleObject(m_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
			//ASSERT(0);
		}
	}
}