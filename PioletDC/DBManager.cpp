#include "StdAfx.h"
#include "dbmanager.h"
#include <afxmt.h>	// for CCriticalSection
#include "PioletDCDll.h"

DBInterface DBManager::g_db_interface;
DBManager::DBManager(void)
{
	p_critical_section=NULL;
	m_ready_to_write_to_db=false;
	m_db_is_maintaining=false;
	bool connected = false;
	while(!connected)
	{
		connected = g_db_interface.OpenConnection("127.0.0.1","onsystems","ebertsux37",PIOLET_DATABASE);
		Sleep(500);
	}
}

//
//
//
DBManager::~DBManager(void)
{
	if(p_critical_section!=NULL)
	{
		SetEvent(m_thread_data.m_events[0]);	// kill thread
	}
	g_db_interface.CloseConnection();
	
	vector<Query *>::iterator iter = v_project_queries.begin();
	if(iter != v_project_queries.end())
	{
		delete *(iter);
		v_project_queries.erase(iter);
	}
	vector<QueryHit *>::iterator hit_iter = v_project_query_hits.begin();
	if(hit_iter != v_project_query_hits.end())
	{
		delete *(hit_iter);
		v_project_query_hits.erase(hit_iter);
	}

}


//
//
//

void WriteProjectSupplyToDatabase(QueryHit *project_query_hits)
{
	CString timestamp = project_query_hits->m_timestamp.Format("%Y%m%d%H%M%S");
	CString filename = project_query_hits->Filename();
	
	DBManager::g_db_interface.InsertRawPioletSupply(PIOLET_SUPPLY_TABLE,
		project_query_hits->m_project,
		project_query_hits->m_ip,
		project_query_hits->m_nickname,
		project_query_hits->m_file_size,
		project_query_hits->m_track,
		project_query_hits->m_bitrate,
		project_query_hits->m_sampling_rate,
		project_query_hits->m_stereo,
		project_query_hits->m_song_length,
		filename,
		project_query_hits->MD5(),
		timestamp,
		project_query_hits->m_header_counter);
}

//
//
//
void WriteProjectDemandToDatabase(Query *project_queries)
{
	CString timestamp = project_queries->m_timestamp.Format("%Y%m%d%H%M%S");
	
	DBManager::g_db_interface.InsertRawPioletDemand(PIOLET_DEMAND_TABLE,
		project_queries->m_project.c_str(), project_queries->m_track,
		project_queries->m_query.c_str(), timestamp, project_queries->m_ip);
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
		connected = db_interface.OpenConnection("127.0.0.1","onsystems","ebertsux37",PIOLET_DATABASE);
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
			db_interface.InsertRawPioletDemand(PIOLET_DEMAND_TABLE,query.m_project.c_str(), query.m_track,
				query.m_query.c_str(), timestamp, query.m_ip);
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

			QueryHit hit;
			hit.ReadFromBuffer(buf);
			
			CString timestamp = hit.m_timestamp.Format("%Y%m%d%H%M%S");
			CString filename = hit.Filename();
			
			db_interface.InsertRawPioletSupply(PIOLET_SUPPLY_TABLE,
				hit.m_project,
				hit.m_ip,
				hit.m_nickname,
				hit.m_file_size,
				hit.m_track,
				hit.m_bitrate,
				hit.m_sampling_rate,
				hit.m_stereo,
				hit.m_song_length,
				filename,
				hit.MD5(),
				timestamp,
				hit.m_header_counter);	

			delete [] buf;
		}
		raw_query_hits_file.Close();
		remove("raw_supplies.dat");
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
	vector<QueryHit *> project_query_hits;
	vector<Query *> project_queries;

	// Create the events
	HANDLE events[DBManagerThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data.m_events[i]=events[i];
	}

	thread_data.p_project_query_hits=&project_query_hits;
	thread_data.p_project_queries=&project_queries;
	
	PostMessage(hwnd,WM_INIT_DB_MANAGER_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	PostMessage(hwnd,WM_READY_TO_WRITE_DATA_TO_DB,1,1);	// the thread is now ready to write into the database

	vector<QueryHit *> project_query_hits_cache;
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

				// Cache the hits
				for(i=0;i<project_query_hits.size();i++)
				{
					project_query_hits_cache.push_back(project_query_hits[i]);
				}

				project_query_hits.clear();	// don't free the memory yet
				
				// Cache the queries
				for(i=0;i<project_queries.size();i++)
				{
					project_queries_cache.push_back(project_queries[i]);
				}

				project_queries.clear();	// don't free the memory yet

				singleLock.Unlock();
			}
		}

		// If there are cached hits to process, then process them
		if(project_query_hits_cache.size()>0)
		{
			// Write out to database for each hit
			for(i=0;i<project_query_hits_cache.size();i++)
			{
				WriteProjectSupplyToDatabase(project_query_hits_cache[i]);
				delete project_query_hits_cache[i];	// now free the memory
			}
			
			project_query_hits_cache.clear();
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

	return 0;	// exit the thread
}


//
//
//
void DBManager::InitParent(PioletDCDll *parent)
{
	Manager::InitParent(parent);

	AfxBeginThread(DBManagerThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_BELOW_NORMAL);
	//AfxBeginThread(DBManagerThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_LOWEST);
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
void DBManager::PushQueryHitData(vector<QueryHit> &query_hits)
{
	if(!m_db_is_maintaining)
	{
		for(UINT i=0;i<query_hits.size();i++)
		{
			QueryHit* hit = new QueryHit(query_hits[i]);
			v_project_query_hits.push_back(hit);
		}
		
		// Check to see if the thread is ready to write data to file
		if(m_ready_to_write_to_db)
		{
			WriteDataToDatabase();
		}
	}
	else
	{
		CFile raw_query_hits_file;
		if(raw_query_hits_file.Open("raw_supplies.dat",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary
			|CFile::shareDenyWrite|CFile::shareDenyRead) != 0)
		{
			raw_query_hits_file.SeekToEnd();
			for(UINT i=0;i<query_hits.size();i++)
			{
				UINT data_size = query_hits[i].GetBufferLength();
				raw_query_hits_file.Write(&data_size, sizeof(UINT));
				char* buf = new char[data_size];
				query_hits[i].WriteToBuffer(buf);
				raw_query_hits_file.Write(buf, data_size);
				delete [] buf;
			}
			raw_query_hits_file.Close();
		}
	}
}

//
//
//
void DBManager::PushQueryData(vector<Query> &querie)
{
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
	if((v_project_query_hits.size()==0) && (v_project_queries.size()==0))
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
		// Supply
		if(v_project_query_hits.size()>0)
		{
			// If the vector is empty, then just copy. Else, add the correct items to the appropriate project
			if(m_thread_data.p_project_query_hits->size()==0)
			{
				*m_thread_data.p_project_query_hits=v_project_query_hits;
			}
			else
			{
				// For each of the project query hits, find the project in the thread, and if it can't be found, then create it
				for(i=0;i<v_project_query_hits.size();i++)
				{
					m_thread_data.p_project_query_hits->push_back(v_project_query_hits[i]);
				}
			}

			v_project_query_hits.clear();	// clear the vector
		}

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
/*
void DBManager::InsertGUID(GUID& guid, CString project, CTime timestamp)
{
	CString ts = timestamp.Format("%Y%m%d%H%M%S");
	g_db_interface.InsertGUID(GNUTELLA_GUID_INDEX_TALBE, guid, project, ts);
}
*/
//
//
//
void DBManager::ReportDBStatus(UINT& query_size, UINT& query_hit_size)
{
	query_size = (UINT)v_project_queries.size();
	query_hit_size = (UINT)v_project_query_hits.size();
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
