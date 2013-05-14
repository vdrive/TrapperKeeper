#include "StdAfx.h"
#include "pioletspooferdll.h"
#include "WSocket.h"
#include "DCHeader.h"
#include "ProjectKeywordsVector.h"
#include "Iphlpapi.h"	// for GetAdaptersInfo (also need Iphlpapi.lib)
#include "Psapi.h"	// for GetProcessMemoryInfo()
#include "PioletSpooferHeader.h"
#include "..\TKSyncher\TKSyncherInterface.h"
#define POISONER_TIMEOUT 300 //5 min

PioletSpooferDll::PioletSpooferDll(void)
{
}

PioletSpooferDll::~PioletSpooferDll(void)
{
}


//
//
//
void PioletSpooferDll::DllInitialize()
{
/*
	BOOL ret=AfxInitRichEdit();	// for rich edit in the log window
*/
	WSocket::Startup(); // Start Windows Sockets
	m_dlg.Create(IDD_DIALOG1,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
}

//
//
//
void PioletSpooferDll::DllUnInitialize()
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
void PioletSpooferDll::DllStart()
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

#ifdef NO_PROCESSING
	m_syncher.Register(this, "Piolet Supply Files");
	m_syncher.InitParent(this);
#endif


	m_dlg.SetTimer(1,1*1000,0);	// set 1 second timer
	m_dlg.SetTimer(2,60*1000,0);

	SendInitToAllPoisoners();

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

	m_piolet_raw_db_info.m_db_host="63.221.232.38";
	m_piolet_raw_db_info.m_db_password="ebertsux37";
	m_piolet_raw_db_info.m_db_user="onsystems";
	p_supply_manager->UpdatePioletRawDBInfo(m_piolet_raw_db_info);

	ReadInPoisonerIPs();
}

//
//
//
void PioletSpooferDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void PioletSpooferDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	bool is_dc_header = true;
	char log[1024];
	char* ptr = (char*)data;
	DCHeader* header = (DCHeader*)ptr;
	ptr+= sizeof(DCHeader);
	switch (header->op)
	{
		case DCHeader::Init:
		{
			DCHeader header;
#ifdef SPLITTED
			header.op = DCHeader::Piolet_Spoofer_Splitted_Init_Response;
#else
			header.op = DCHeader::Piolet_Spoofer_Init_Response;
#endif
			header.size=0;
			m_com.SendReliableData(source_name, &header, sizeof(DCHeader));
			// Log this message
			sprintf(log, "Received Init Message from %s\n", source_name);
			m_dlg.Log(log, 0x00FF0000, true); //blue and bold
			break;
		}
		case DCHeader::ProjectChecksums:
		{
			ProjectChecksums current_projects;
			current_projects.ReadFromBuffer(ptr);

			// Log this message
			sprintf(log, "Received ProjectChecksums Message from %s : Received %d checksums\n", source_name, current_projects.v_checksums.size());
			p_keyword_manager->CheckProjectChecksums(source_name, current_projects);
			m_dlg.Log(log, 0x00FF0000, true); //blue and bold
			break;
		}
		case DCHeader::ProjectKeywords:
		{
			ProjectKeywordsVector keywords;
			keywords.ReadFromBuffer(ptr);

			// Log this message
			sprintf(log, "Received ProjectKeywords Message from %s  : %d projects\n", source_name, (int)keywords.v_projects.size());
			m_dlg.Log(log, 0x00FF0000, true); //blue and bold
			p_keyword_manager->NewProjectKeywordsReceived(keywords.v_projects);
			break;
		}
		case DCHeader::DatabaseInfo:
		{
			m_processed_db_info.ReadFromBuffer(ptr);
			break;
		}
		case DCHeader::PioletRawDatabaseInfo:
		{
			m_piolet_raw_db_info.ReadFromBuffer(ptr);
			p_supply_manager->UpdatePioletRawDBInfo(m_piolet_raw_db_info);
			p_connection_manager->AddDC(m_piolet_raw_db_info.m_db_host.c_str());
			break;
		}
		default:
		{
			is_dc_header=false;
			break;
		}
	}
	/*
	if(!is_dc_header)
	{
		ptr = (char*)data;
		PioletSpooferHeader* header = (PioletSpooferHeader*)ptr;
		ptr+= sizeof(PioletSpooferHeader);
		switch (header->op)
		{
			case PioletSpooferHeader::Poisoner_Init_Response:
			{
				ReceivedPoisonerInitResponse(source_name);
				SendDBInfo(source_name);
				break;
			}
		}
	}
	*/
}

//
//
//
void PioletSpooferDll::OnTimer(UINT nIDEvent)
{
	m_dlg.KillTimer(nIDEvent);
	switch(nIDEvent)
	{
		case 1:	// 1 second timer
		{
			CheckPerformanceCounters();
			p_connection_manager->TimerHasFired();
#ifndef NO_PROCESSING
			if(p_process_manager->IsProcessing())
			{
				int spoof=0;
				p_process_manager->GetProcessProgress(spoof);
				m_dlg.UpdateDataProcessProgressBar(spoof);
			}
#endif
			m_dlg.SetTimer(1,1000,NULL);
			break;
		}
		case 2: //1 min
		{
			SendInitToAllPoisoners();
			//CheckForPoisonersStatus();

			p_supply_manager->OnOneMinuteTimer();

#ifndef NO_PROCESSING
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
#endif
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
void PioletSpooferDll::RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update)
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
void PioletSpooferDll::KeywordsUpdated()
{
	m_dlg.KeywordsUpdated(p_keyword_manager->v_project_keywords);
}

//
//
//
void PioletSpooferDll::SpoofDataReady(ConnectionModuleStatusData& status)
{
	// Write data to database
	for(UINT i=0; i<status.v_project_status.size(); i++)
	{
		p_db_manager->PushSpoofData(status.v_project_status[i].v_queries);
	}
}

//
//
//
void PioletSpooferDll::InitDBManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_db_manager->InitThreadData(wparam,lparam);
}

//
//
//
void PioletSpooferDll::InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_supply_manager->InitThreadData(wparam,lparam);
}

//
//
//
void PioletSpooferDll::UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam)
{
	p_supply_manager->UpdateSupplyManagerSupplyProjects(wparam,lparam);
}

//
//
//
void PioletSpooferDll::ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam)
{
	p_db_manager->ReadyToWriteDataToDatabase(wparam,lparam);
}

//
//
//
void PioletSpooferDll::InitPerformanceCounters()
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
#ifdef _DEBUG
		MessageBox(NULL,"Error opening pdh query","Error",MB_OK);
#endif
		return;
	}

    // ADD A COUNTER TO THE QUERY
	for(i=0;i<3;i++)
	{
		PDH_STATUS error=PdhAddCounter(m_pdh,m_keynames[i].c_str(),NULL,&m_pdh_counters[i]);

		if(error)
		{
#ifdef _DEBUG
			MessageBox(NULL,"Error adding counter to the pdh query","Error",MB_OK);
#endif
		return;
		}
	}
    
//	m_dlg.SetTimer(6,TIMER_LENGTH*1000,0);
}

//
//
//
void PioletSpooferDll::CheckPerformanceCounters()
{
	UINT i;

    // Collect the pdh query data
	if(PdhCollectQueryData(m_pdh))
	{
#ifdef _DEBUG
		MessageBox(NULL,"Error collecting pdh query data","Error",MB_OK);
#endif
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
	UINT queries_size;
	p_db_manager->ReportDBStatus(queries_size);
	sprintf(buf, "%u", queries_size);
	m_dlg.GetDlgItem(IDC_QUERIES_SIZE_STATIC)->SetWindowText(buf);
}

//
//
//
void PioletSpooferDll::ClosePerformanceCounters()
{
	 PdhCloseQuery(m_pdh);
}

//
//
//
void PioletSpooferDll::UpdateHostLimits(UINT max_host, UINT max_host_cache)
{
	p_connection_manager->UpdateHostLimits(max_host,max_host_cache);
}

//
//
//
void PioletSpooferDll::ReportVendorCounts(vector<VendorCount>* vendor_counts)
{
	m_dlg.ReportVendorCounts(vendor_counts);
}

//
//
//
void PioletSpooferDll::InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_process_manager->InitThreadData(wparam, lparam);
}


//
//
//
void PioletSpooferDll::ProcessManagerDone(WPARAM wparam,LPARAM lparam)
{
	p_process_manager->ProcessFinished();
}

//
//
//
void PioletSpooferDll::StartDataProcess()
{
	p_process_manager->StartProcessRawData(m_processed_db_info, p_keyword_manager->v_project_keywords);
}

//
//
//
void PioletSpooferDll::ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam)
{
	p_process_manager->MaintenanceFinished();
}

//
//
//
void PioletSpooferDll::DBMaintenanceReadyToStart(void)
{
	p_db_manager->DBMaintenanceReadyToStart();
}

//
//
//
void PioletSpooferDll::DBMaintenanceFinished(void)
{
	p_db_manager->DBMaintenanceFinished();
}

//
//
//
void PioletSpooferDll::SpoofEntriesRetrievalStarted()
{
	m_dlg.SpoofEntriesRetrievalStarted();
}

//
//
//
void PioletSpooferDll::SendPoisonEntry(char* poisoner, PoisonEntries& pe)
{
	/*
	char* buf = new char[sizeof(PioletSpooferHeader)+pe.GetBufferLength()];
	PioletSpooferHeader* header = (PioletSpooferHeader*)buf;
	header->op = PioletSpooferHeader::PoisonEntry;
	header->size = pe.GetBufferLength();
	
	pe.WriteToBuffer(&buf[sizeof(PioletSpooferHeader)]);
	m_com.SendReliableData(poisoner, buf, sizeof(PioletSpooferHeader)+pe.GetBufferLength());
	delete [] buf;
	*/
}

//
//
//
void PioletSpooferDll::SendInitToAllPoisoners()
{
	vector<string> IPs;
	m_nameserver.RequestIP("PIOLET-POISONER",IPs);
	for(UINT i=0; i<IPs.size();i++)
	{
		PioletSpooferHeader header;
		header.op = PioletSpooferHeader::Poisoner_Init;
		header.size = 0;
		char IP[16];
		strcpy(IP, IPs[i].c_str());
		m_com.SendReliableData(IP, &header, sizeof(header));
	}
}

//
//
//
/*
void PioletSpooferDll::ReceivedPoisonerInitResponse(char* from)
{
	UINT ip=0;
	bool found = false;
	for(UINT i=0;i<v_poisoners.size();i++)
	{
		int ip1,ip2,ip3,ip4;
		sscanf(from,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
		ip = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);

		if(v_poisoners[i] == ip)
		{
			found = true;
			v_poisoners[i].m_last_init_response_time = CTime::GetCurrentTime();
			break;
		}
	}
	if(!found)
	{
		PoisonerStatus status;
		status.m_ip = ip;
		strcpy(status.m_ip_str, from);
		v_poisoners.push_back(status);
	}
}
*/
//
//
//
/*
void PioletSpooferDll::CheckForPoisonersStatus()
{
	UINT i=0;
	//check for emule network
	vector<PoisonerStatus>::iterator iter = v_poisoners.begin();
	while(iter!= v_poisoners.end())
	{
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= POISONER_TIMEOUT) //hasn't response for more than 10 mins
		{
			v_poisoners.erase(iter);
		}
		else
			iter++;
	}
}
*/

//
//
//
vector<PoisonerStatus>* PioletSpooferDll::GetPoisoners()
{
	return &v_poisoners;
}

//
//
//
void PioletSpooferDll::SendDBInfo(char* dest)
{
	int len = sizeof(PioletSpooferHeader) + m_processed_db_info.GetBufferLength();
	char* buf = new char[len];
	PioletSpooferHeader* header = (PioletSpooferHeader*)buf;
	header->op = PioletSpooferHeader::Poisoner_DatabaseInfo;
	header->size = len - sizeof(PioletSpooferHeader);

	m_processed_db_info.WriteToBuffer(&buf[sizeof(PioletSpooferHeader)]);

	m_com.SendReliableData(dest, buf, len);
	delete [] buf;
}

//
//
//
void PioletSpooferDll::ReadInPoisonerIPs()
{
	v_poisoners.clear();
	CStdioFile file;
#ifdef SPLITTED
	if(file.Open("c:\\syncher\\rcv\\executables\\poisoners.txt",CFile::modeRead|CFile::typeText)!=0)
#else
	if(file.Open("poisoners.txt",CFile::modeRead|CFile::typeText)!=0)
#endif
	{
		CString ip_str;
		while(file.ReadString(ip_str))
		{
			if(ip_str.GetLength()>0)
			{
				int ip1,ip2,ip3,ip4;
				sscanf(ip_str,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
				UINT ip = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
				PoisonerStatus status;
				status.m_ip = ip;
				strcpy(status.m_ip_str, ip_str);
				v_poisoners.push_back(status);
			}
		}
		file.Close();
	}
}

int PioletSpooferDll::GetNumPoisoners()
{
 return v_poisoners.size();
}

//
//
//
void PioletSpooferDll::SyncherRefresh()
{
	//notify the syncher to re-scan the source
	TKSyncherInterface tki;
	tki.RescanSource("Piolet Supply Files");
}

//
//
//
void PioletSpooferDll::SupplySynched(const char* source_ip)
{
 	m_dlg.Log("Supplies are synched, reloading supplies from files");
	p_supply_manager->UpdateSupplyManagerSupplyProjects(0,0);
}