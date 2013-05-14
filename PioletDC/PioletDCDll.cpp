#include "StdAfx.h"
#include "pioletdcdll.h"
#include "WSocket.h"
#include "DCHeader.h"
#include "ProjectKeywordsVector.h"
#include "Iphlpapi.h"	// for GetAdaptersInfo (also need Iphlpapi.lib)
#include "Psapi.h"	// for GetProcessMemoryInfo()

PioletDCDll::PioletDCDll(void)
{
}

PioletDCDll::~PioletDCDll(void)
{
}

//
//
//
void PioletDCDll::DllInitialize()
{
/*
	BOOL ret=AfxInitRichEdit();	// for rich edit in the log window
*/
	WSocket::Startup(); // Start Windows Sockets
	m_dlg.Create(IDD_PIOLET_DC_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
}

//
//
//
void PioletDCDll::DllUnInitialize()
{
	delete p_connection_manager;
	delete p_keyword_manager;
	delete p_db_manager;
	delete p_process_manager;
	delete p_supply_manager;
	ClosePerformanceCounters();
	m_dlg.DestroyWindow();
}

//
//
//
void PioletDCDll::DllStart()
{
	InitPerformanceCounters();

	m_proc_percent_usage=0;
	m_bandwidth_in_bytes=0;
	m_current_bandwdith_in_bits_per_second=0;
	m_mem_usage=0;

	m_com.Register(this, 35);

	p_keyword_manager = new KeywordManager();
	p_keyword_manager->InitParent(this);

	p_connection_manager = new ConnectionManager();
	p_connection_manager->InitParent(this);
	p_connection_manager->InitKeywordManager(p_keyword_manager);

	p_supply_manager = new SupplyManager();
	p_supply_manager->InitParent(this);
	p_supply_manager->InitConnectionManager(p_connection_manager);

	p_connection_manager->InitSupplyManager(p_supply_manager);

	p_keyword_manager->SetConnectionManager(p_connection_manager);
	p_keyword_manager->SetSupplyManager(p_supply_manager);

	p_db_manager = new DBManager();
	p_db_manager->InitParent(this);

	p_process_manager = new ProcessManager();
	p_process_manager->InitParent(this);

	m_dlg.SetTimer(1,1*1000,0);	// set 1 second timer
	m_dlg.SetTimer(2,60*1000,0);

/*
	m_dlg.SetTimer(2, BEASTIE_STATUS_INTERVAL*1000, NULL);	// 30 second status timer
	m_dlg.SetTimer(5,MODULE_CONNECTION_CHECK_INTERVAL*1000,NULL);	// timer to check to see if we need to add or remove modules

	KeywordsUpdated();	// tell the gui what the project keywords are
	m_keyword_manager.BroadcastUpdateAlert();
	InitPerformanceCounters();

	m_running_since_time=CTime::GetCurrentTime();
	m_proc_percent_usage=0;
	m_bandwidth_in_bytes=0;
	m_current_bandwdith_in_bits_per_second=0;
	m_mem_usage=0;

	m_log_window_manager.Log("GnutellaSupply Started\n",0x00000000,true);

	// Log the keywords we read in from file
	m_log_window_manager.Log("Keyword Manager : ",0x00FF40FF);	// purple
	m_log_window_manager.Log("Loading Keyword Files\n",0,true);		// bold
	char log[1024];
	for(UINT i=0;i<m_keyword_manager.v_project_keywords.size();i++)
	{
		m_log_window_manager.Log("Keyword Manager : ",0x00FF40FF);	// purple
		m_log_window_manager.Log("Keyword File Loaded : ");
		sprintf(log,"(%u/%u)",i+1,m_keyword_manager.v_project_keywords.size());
		m_log_window_manager.Log(log,0x000080FF);
		sprintf(log," : %s\n",m_keyword_manager.v_project_keywords[i].m_project_name.c_str());
		m_log_window_manager.Log(log,0x00FF0000,true);	// bold blue text
	}
	m_log_window_manager.Log("Keyword Manager : ",0x00FF40FF);	// purple
	m_log_window_manager.Log("Loading Keyword Files Complete\n",0,true);	// bold

	//m_process_manager.DBMaintenance();
*/
	p_keyword_manager->BroadcastUpdateAlert();

	m_processed_db_info.m_db_host = "127.0.0.1";
	m_processed_db_info.m_db_password = "ebertsux37";
	m_processed_db_info.m_db_user = "onsystems";
}

//
//
//
void PioletDCDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void PioletDCDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	char log[1024];
	char* ptr = (char*)data;
	DCHeader* header = (DCHeader*)ptr;
	ptr+= sizeof(DCHeader);
	switch (header->op)
	{
		case DCHeader::Init:
		{
			DCHeader header;
			header.op = DCHeader::Piolet_DC_Init_Response;
			header.size=0;
			m_com.SendReliableData(source_name, &header, sizeof(DCHeader));
			// Log this message
			sprintf(log, "Received Init Message from %s\n", source_name);
			m_dlg.Log(log);
			break;
		}
		case DCHeader::ProjectChecksums:
		{
			ProjectChecksums current_projects;
			current_projects.ReadFromBuffer(ptr);

			// Log this message
			sprintf(log, "Received ProjectChecksums Message from %s : Received %d checksums\n", source_name, current_projects.v_checksums.size());
			p_keyword_manager->CheckProjectChecksums(source_name, current_projects);
			m_dlg.Log(log);
			break;
		}
		case DCHeader::ProjectKeywords:
		{
			ProjectKeywordsVector keywords;
			keywords.ReadFromBuffer(ptr);

			// Log this message
			sprintf(log, "Received ProjectKeywords Message from %s  : %d projects\n", source_name, (int)keywords.v_projects.size());
			m_dlg.Log(log);
			p_keyword_manager->NewProjectKeywordsReceived(keywords.v_projects);
			break;
		}
		case DCHeader::DatabaseInfo:
		{
			m_processed_db_info.ReadFromBuffer(ptr);
			break;
		}
	}
}

//
//
//
void PioletDCDll::OnTimer(UINT nIDEvent)
{
	m_dlg.KillTimer(nIDEvent);
	switch(nIDEvent)
	{
		case 1:	// 1 second timer
		{
			CheckPerformanceCounters();
			p_connection_manager->TimerHasFired();
			if(p_process_manager->IsProcessing())
			{
				int demand=0;
				int supply=0;
				p_process_manager->GetProcessProgress(demand, supply);
				m_dlg.UpdateDataProcessProgressBar(demand, supply);
			}
			m_dlg.SetTimer(1,1000,NULL);
			break;
		}
		case 2: //1 min
		{
			p_supply_manager->OnOneMinuteTimer();

			//reset process manager everyday at 1am
			CTime current_time = CTime::GetCurrentTime();
			int hour = current_time.GetHour();
			int min = current_time.GetMinute();
			if(hour == 1 && min == 0)
			{
				p_process_manager->Reset();
			}

			//check to see if we need to process raw data
			if(p_process_manager->IsProcessing() == false &&
				p_process_manager->IsAllDone() == false)
			{
				p_process_manager->StartProcessRawData(m_processed_db_info, p_keyword_manager->v_project_keywords);
			}

			m_dlg.SetTimer(2,60*1000,NULL);
			break;
		}
	}
}

//
//
//
//
//
//
void PioletDCDll::RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update)
{
	int buf_len = projects_requiring_update.GetBufferLength();
	unsigned char* buf = new unsigned char[sizeof(DCHeader)+buf_len];

	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::UpdateProjectsRequest;
	header->size =buf_len;

	projects_requiring_update.WriteToBuffer((char*)(&buf[sizeof(DCHeader)]));

	m_com.SendReliableData(dc_master, buf, sizeof(DCHeader)+buf_len);

	delete [] buf;
}

//
//
//
void PioletDCDll::KeywordsUpdated()
{
	m_dlg.KeywordsUpdated(p_keyword_manager->v_project_keywords);
}

//
//
//
void PioletDCDll::DemandSupplyDataReady(ConnectionModuleStatusData& status)
{
	// Write data to database
	for(UINT i=0; i<status.v_project_status.size(); i++)
	{
		p_db_manager->PushQueryHitData(status.v_project_status[i].v_query_hits);
		p_db_manager->PushQueryData(status.v_project_status[i].v_queries);
	}
}

//
//
//
void PioletDCDll::InitDBManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_db_manager->InitThreadData(wparam,lparam);
}

//
//
//
void PioletDCDll::ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam)
{
	p_db_manager->ReadyToWriteDataToDatabase(wparam,lparam);
}

//
//
//
void PioletDCDll::InitPerformanceCounters()
{
	UINT timer_ret=1;

	int i;

	char iai_buf[1024];
	DWORD iai_buf_len=1024;
	IP_ADAPTER_INFO *iai=(IP_ADAPTER_INFO *)iai_buf;

	GetAdaptersInfo(iai,&iai_buf_len);

	// Remove (,) and / from the description of the interface
	while(strchr(iai->Description,'(')!=NULL)
	{
		*strchr(iai->Description,'(')='[';
	}
	while(strchr(iai->Description,')')!=NULL)
	{
		*strchr(iai->Description,')')=']';
	}
	while(strchr(iai->Description,'/')!=NULL)
	{
		*strchr(iai->Description,'/')='_';
	}

	m_keynames[0]="\\Processor(0)\\% Processor Time";
	m_keynames[1]="\\Network Interface(";
	m_keynames[1]+=iai->Description;
	m_keynames[1]+=")\\Bytes Total/sec";
	m_keynames[2]="\\Network Interface(";
	m_keynames[2]+=iai->Description;
	m_keynames[2]+=")\\Current Bandwidth";

    m_pdh=0;

    // Create the pdh query
    if(PdhOpenQuery(NULL,0,&m_pdh))
	{
		MessageBox(NULL,"Error opening pdh query","Error",MB_OK);
		return;
	}

    // ADD A COUNTER TO THE QUERY
	for(i=0;i<3;i++)
	{
		PDH_STATUS error=PdhAddCounter(m_pdh,m_keynames[i].c_str(),NULL,&m_pdh_counters[i]);

		if(error)
		{
			MessageBox(NULL,"Error adding counter to the pdh query","Error",MB_OK);
			return;
		}
	}
    
//	m_dlg.SetTimer(6,TIMER_LENGTH*1000,0);
}

//
//
//
void PioletDCDll::CheckPerformanceCounters()
{
	UINT i;

    // Collect the pdh query data
	if(PdhCollectQueryData(m_pdh))
	{
		MessageBox(NULL,"Error collecting pdh query data","Error",MB_OK);
		return;
	}

	// Check the processor usage counter
	PDH_FMT_COUNTERVALUE value;
    PDH_STATUS error;

	error=PdhGetFormattedCounterValue(m_pdh_counters[0],PDH_FMT_LONG,0,&value);
	if(error)
	{
//		MessageBox("Error formatting pdh counter data");
//		m_dlg.SetWindowText("Error formatting pdh coutner data");
		return;
	}

	m_proc_percent_usage=value.longValue;

/*
	// If less than 50 % kick off another mod if there is less than min count of them
	if((m_proc_percent_usage<50) & (m_connection_manager.ReturnModCount()<(UINT)m_min_module_count))	// was 60
	{
		m_connection_manager.AddModule();

		char log[1024];
		sprintf(log,"Module Added : (Current Count : %u) : (Proc % Usage : %u%%) : (Min Mod Count : %u)\n",
			m_connection_manager.ReturnModCount(),m_proc_percent_usage,m_min_module_count);
		m_log_window_manager.Log(log,0x00888888);
	}
*/
	// Check the bandwidth counter
	error=PdhGetFormattedCounterValue(m_pdh_counters[1],PDH_FMT_LONG,0,&value);
	if(error)
	{
//		MessageBox("Error formatting pdh counter data");
//		m_dlg.SetWindowText("Error formatting pdh coutner data");
		return;
	}

	m_bandwidth_in_bytes=value.longValue;

	// Check the current bandwidth in bits per second
	error=PdhGetFormattedCounterValue(m_pdh_counters[2],PDH_FMT_LONG,0,&value);
	if(error)
	{
		return;
	}

	m_current_bandwdith_in_bits_per_second=value.longValue;	// this should be 100 Mbps
	
	double bps=m_bandwidth_in_bytes*8;

	// Measure the processes mem usage
	HANDLE hHandle;
	DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&hHandle,0,FALSE,DUPLICATE_SAME_ACCESS);
	
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.cb=sizeof(PROCESS_MEMORY_COUNTERS);
	GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS));

	CloseHandle(hHandle);

	m_mem_usage=(UINT)pmc.WorkingSetSize;	// in bytes

	// Put commas in the mem measurement value
	char mem_buf[100];
	string mem;
	_itoa(m_mem_usage/1024,mem_buf,10);
	strrev(mem_buf);
	for(i=0;i<strlen(mem_buf);i++)
	{
		if((i>0) && (i%3==0))
		{
			mem+=",";
		}
		
		mem+=mem_buf[i];
	}
	strcpy(mem_buf,mem.c_str());
	strrev(mem_buf);
	
	// Display the status
	char buf[1024];
	sprintf(buf,"[ %u %% ] - [ %.2f KB/sec - %.2f %% ] - [ %s KB ]",m_proc_percent_usage,((double)m_bandwidth_in_bytes)/1024,
		(100*bps)/m_current_bandwdith_in_bits_per_second,mem_buf);
	m_dlg.GetDlgItem(IDC_Computer_Status_Static)->SetWindowText(buf);

	//Get DB queues sizes
	UINT queries_size, qh_size;
	p_db_manager->ReportDBStatus(queries_size, qh_size);
	sprintf(buf, "%u", queries_size);
	m_dlg.GetDlgItem(IDC_QUERIES_SIZE_STATIC)->SetWindowText(buf);
	sprintf(buf, "%u", qh_size);
	m_dlg.GetDlgItem(IDC_QH_SIZE_STATIC)->SetWindowText(buf);
}

//
//
//
void PioletDCDll::ClosePerformanceCounters()
{
	 PdhCloseQuery(m_pdh);
}

//
//
//
void PioletDCDll::UpdateHostLimits(UINT max_host, UINT max_host_cache)
{
	p_connection_manager->UpdateHostLimits(max_host,max_host_cache);
}

//
//
//
void PioletDCDll::ReportVendorCounts(vector<VendorCount>* vendor_counts)
{
	m_dlg.ReportVendorCounts(vendor_counts);
}

//
//
//
void PioletDCDll::InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_process_manager->InitThreadData(wparam, lparam);
}


//
//
//
void PioletDCDll::ProcessManagerDone(WPARAM wparam,LPARAM lparam)
{
	p_process_manager->ProcessFinished();
}

//
//
//
void PioletDCDll::StartDataProcess()
{
	p_process_manager->StartProcessRawData(m_processed_db_info, p_keyword_manager->v_project_keywords);
}

//
//
//
void PioletDCDll::ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam)
{
	p_process_manager->MaintenanceFinished();
}

//
//
//
void PioletDCDll::DBMaintenanceReadyToStart(void)
{
	p_db_manager->DBMaintenanceReadyToStart();
}

//
//
//
void PioletDCDll::DBMaintenanceFinished(void)
{
	p_db_manager->DBMaintenanceFinished();
}