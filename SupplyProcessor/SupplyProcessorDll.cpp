#include "StdAfx.h"
#include "SupplyProcessorDLL.h"
#include "SupplyData.h"
#include "Winsock2.h"
#include "SupplyProcessorHeader.h"



//
//
//
SupplyProcessorDLL::SupplyProcessorDLL(void)
{
	m_source = "";
	m_clear_interval = 3;
}

//
//
//
SupplyProcessorDLL::~SupplyProcessorDLL(void)
{
}

//
//
//
void SupplyProcessorDLL::DllInitialize()
{
#ifdef DC2
	m_database_helper.Init("127.0.0.1","onsystems","ebertsux37","supply_test");
#else
	m_database_helper.Init("206.161.141.35","onsystems","ebertsux37","supply_test");
#endif
	m_dlg.Create(IDD_SUPPLYPROCESSORTESTDLG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_dlg_hwnd = m_dlg.GetSafeHwnd();
}

//
//
//
void SupplyProcessorDLL::DllUnInitialize()
{
}

//
//
//
void SupplyProcessorDLL::DllStart()
{
	Log("SupplyProcessorDLL::DllStart");
	CTime now=CTime::GetCurrentTime();
	CTimeSpan day(1, 0, 0, 0);
	now = now + day;
	CTime next(now.GetYear(), now.GetMonth(), now.GetDay(), 2, 0, 0);
	CTimeSpan span = next-CTime::GetCurrentTime();
	m_dlg.SetTimer(2, 5*60*1000,NULL);  // 5 mins between db inserts
	m_dlg.SetTimer(1, (long)span.GetTotalSeconds()*1000, NULL); // 24 hour timer for clearing teh db.
}


//
//
//
void SupplyProcessorDLL::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
	Log("SupplyProcessorDLL::DllShowGUI");
}


bool SupplyProcessorDLL::ReceivedDllData(AppID from_app_id,void* input_data,void* output_data)
{
	//Log("SupplyProcessorDLL::ReceivedDllData");
	char *ptr = (char*)input_data;
	SupplyProcessorHeader header;
	memcpy(&header, input_data, sizeof(SupplyProcessorHeader));
	header.size = sizeof(SupplyProcessorHeader);

	ptr+= header.size;
	switch(header.op)
	{
	case SupplyProcessorHeader::Submit:
		return Write(ptr, output_data);
	case SupplyProcessorHeader::Request:
		return Read(ptr, output_data);
	case SupplyProcessorHeader::Clear:
		return Clear(ptr, output_data);
	}
	return false;

}

bool SupplyProcessorDLL::Send()
{
	bool retVal = false;
	CSingleLock locked(&m_lock);
	vector<SupplyData> v_supply_queue_copy;
	locked.Lock();
	if(locked.IsLocked()) {
		v_supply_queue_copy = v_supply_queue;
		v_supply_queue.clear();
	}
	locked.Unlock();
	retVal = Write(&v_supply_queue_copy);
	return retVal;
}

bool SupplyProcessorDLL::Write(void * buf, void * supply_data) 
{
	vector<SupplyData>* v_supply = (vector<SupplyData>*)supply_data;
	CSingleLock lock(&m_lock);
	lock.Lock();
	if(lock.IsLocked()) {
		for(unsigned int i = 0; i < v_supply->size(); i++) {
#ifdef _DEBUG
			CString query;
			(*v_supply)[i].GetValueQuery(query);
			m_dlg.Queue(query);
#endif
			v_supply_queue.push_back((*v_supply)[i]);
		}
	}
	lock.Unlock();
	return true;
}

bool SupplyProcessorDLL::Write(vector<SupplyData>* v_supply_data) 
{
	if(v_supply_data->size() == 0) return true;
	CString query = "insert ignore into raw_supply values";
	for(unsigned int i = 0; i < v_supply_data->size(); i++) {
		if(i != 0) query += ", ";
		(*v_supply_data)[i].GetValueQuery(query);
	}
	m_dlg.ClearQueue();
#ifdef _DEBUG
	TRACE("SupplyProcessorDLL::Write query: %s\n", query);
#endif
	if(!m_database_helper.Execute(query)) {
		TRACE("SupplyProcessorDLL::Write ERROR: %s\n%s\n", m_database_helper.GetLastErrorMessage(), query);
		CString buf;
		buf.Format("SupplyProcessorDLL::Write ERROR: %s", m_database_helper.GetLastErrorMessage());
		Log(buf);
		CSingleLock locked(&m_lock);
		locked.Lock();
		if(locked.IsLocked()) {
			for(UINT i=0;i < v_supply_data->size(); i++)
			{
				v_supply_queue.push_back((*v_supply_data)[i]);
			}
			v_supply_data->clear();
		}
		locked.Unlock();
		return false;
	}
	else {
		//TRACE("Insert successful\n");
		v_supply_data->clear();
		Log("Insert successful");
		return true;
	}

}

bool SupplyProcessorDLL::Clear(char * buf, void*output_data)
{
	int days = *((unsigned int *)buf);
	buf+= sizeof(int);
	string source = buf;

	m_source = source;
	m_clear_interval = days;
	return true;
	//return ClearDB();
}

bool SupplyProcessorDLL::ClearDB()
{
	Log("SupplyProcessorDLL::ClearDB");
	CTime now=CTime::GetCurrentTime();
	TRACE("now: %04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());

	if(m_source.length() == 0) return false;
	char query[1024];
	sprintf(query, "delete from raw_supply where date < curdate() - interval %d day and network = '%s'",
		m_clear_interval, m_source.c_str());

	if(!m_database_helper.Execute(query)) {
		TRACE("SupplyProcessorDLL::ClearDB  ERROR: %s\n", m_database_helper.GetLastErrorMessage());
		CString buf;
		buf.Format("SupplyProcessorDLL::ClearDB ERROR: %s", m_database_helper.GetLastErrorMessage());
		Log(buf);
		return false;
	}
	else {
		Log("ClearDB() thread done");
		return true;
	}
}

bool SupplyProcessorDLL::Read(char*buf, void* output_data)
{
	char * ptr = buf;
	int project_id = *((unsigned int*)ptr);
	ptr += sizeof(unsigned int);
	int track_num = *((unsigned int *)ptr);
	ptr += sizeof(unsigned int);
	int num_want = *((unsigned int *)ptr);
	ptr += sizeof(unsigned int);

	vector<SupplyData>* vec = (vector<SupplyData>*)output_data;
	CString query;
	query.Format("select *,sum(count) as total_count from raw_supply where project_id = '%d' and track = %d and network = '%s' group by hash order by total_count desc limit %d",
		project_id, track_num, ptr, num_want);
	if(m_database_helper.Query(query)) {
		int num_rows = m_database_helper.GetNumRows();
		for(int i = 0; i < num_rows; i++) {
			MYSQL_ROW row = m_database_helper.GetNextRow();
			SupplyData sd;
			sd.m_filename = row[0];
			sscanf(row[1],"%u",&sd.m_filesize);
			sscanf(row[2],"%u",&sd.m_project_id);
			sd.m_track = atoi(row[3]);
			sd.m_bitrate = atoi(row[4]);
			sd.m_media_len = atoi(row[5]);
			sd.m_hash = row[6];
			sd.m_ip = row[7];
			sd.m_port = atoi(row[8]);
			sd.m_count = atoi(row[9]);
			sd.m_file_type = row[10];
			sd.m_network_name = row[12];
			sd.m_artist = row[13];
			sd.m_title = row[14];
			sd.m_album = row[15];
			sd.m_codec = row[16];
			vec->push_back(sd);
		}
	}
	else {
		TRACE("SupplyProcessorDLL::Read %s\n", m_database_helper.GetLastErrorMessage());
		TRACE("SupplyProcessorDLL::Read QUERY: %s\n", query);
		Log("SupplyProcessorDLL::Read Error:");
		Log(m_database_helper.GetLastErrorMessage());
		return false;
	}
	//vector<SupplyData> * ips = (vector<SupplyData>*)output_data;
	//*ips = vec;
	return true;
}

//
//
//
void SupplyProcessorDLL::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface


}

void SupplyProcessorDLL::Log(string message)
{
	Log(message.c_str());
}
void SupplyProcessorDLL::Log(const char * message)
{
	m_dlg.Log(message);
	TRACE("SupplyProcessorDLL: %s\n", message);
}