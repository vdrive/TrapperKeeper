#include "StdAfx.h"
#include "fasttrackgiftdll.h"
#include "RegistryKeys.h"
#include "Iphlpapi.h"	// for GetAdaptersInfo (also need Iphlpapi.lib)
#include "Psapi.h"	// for GetProcessMemoryInfo()
#include "DCHeader.h"
#include "WSocket.h"
#include "ProjectKeywordsVector.h"

FastTrackGiftDll::FastTrackGiftDll(void)
{
	WSocket::Startup();
}

//
//
//
FastTrackGiftDll::~FastTrackGiftDll(void)
{
}

//
//
//
void FastTrackGiftDll::DllInitialize()
{
	m_auto_pause_threshold = 50;
	m_auto_resume_threshold = 40;
}

//
//
//
void FastTrackGiftDll::DllUnInitialize()
{
	WSocket::Cleanup();
	m_connection_manager.KillModules();	// hopefully avoids crash on closing
#ifdef FILE_SERVER_ENABLED
	m_noise_manager.KillModules();	// hopefully avoids crash on closing
#endif
	m_db_manager.TerminateThread();
	ClosePerformanceCounters();
	m_dlg.DestroyWindow();
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
	m_process_manager.KillThread();
#endif
#endif
#ifdef SHARING_ENABLED
	m_file_sharing_manager.TerminateThread();
#endif

}

//
//
//
void FastTrackGiftDll::DllStart()
{
	DeleteFile("SQL_Error_Log.txt");
	DeleteFile("SQL_Supply_Entry_Error_Log.txt");
	RegistryKeys::GetModuleCounts(&m_min_module_count,&m_max_module_count);
	RegistryKeys::GetSupplyIntervalMultiplier(&m_supply_interval_multi);
	m_dlg.Create(IDD_FASTTRACK_GIFT_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_syncher.Register(this, "Kazaa-Launcher");


	m_connection_manager.InitParent(this);
	m_connection_manager.SetKeywordManager(&m_keyword_manager);
	m_connection_manager.SetSupplyManager(&m_supply_manager);
#ifdef SHARING_ENABLED
	m_connection_manager.SetFileSharingManager(&m_file_sharing_manager);
#endif
	m_keyword_manager.InitParent(this);
	m_keyword_manager.SetConnectionManager(&m_connection_manager);
	m_keyword_manager.SetSupplyManager(&m_supply_manager);
	m_supply_manager.InitParent(this);
	m_supply_manager.SetConnectionManager(&m_connection_manager);
	m_supply_manager.AlterSupplyIntervalMultiplier(m_supply_interval_multi);
	m_db_manager.InitParent(this);
#ifdef FILE_SERVER_ENABLED
	m_noise_manager.InitParent(this);
	m_noise_manager.SetFileSharingManager(&m_file_sharing_manager);
#endif
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
	m_process_manager.InitParent(this);
#endif
#endif

#ifdef SHARING_ENABLED
	m_file_sharing_manager.SetKeywordManager(&m_keyword_manager);
	m_file_sharing_manager.InitParent(this);
#endif
	
#ifdef _DEBUG
	m_processed_db_info.m_db_host="127.0.0.1";
#else
	m_processed_db_info.m_db_host="dcmaster.mediadefender.com";
#endif

#ifdef SPECIAL_SUPPLY_COLLECTING
	m_processed_db_info.m_db_host="127.0.0.1";
#endif

	m_processed_db_info.m_db_user="onsystems";
	m_processed_db_info.m_db_password="ebertsux37";

	//register com service with DCMaster ID 35
	m_com.Register(this, 35);

	m_keyword_manager.BroadcastUpdateAlert();

	InitPerformanceCounters();
	m_dlg.SetTimer(1,1000,0);
	m_dlg.SetTimer(2,60*1000,0);
	m_dlg.SetTimer(3,5*60*1000,0);
	m_dlg.SetTimer(6,5*1000,0);
	m_dlg.SetTimer(5,30*1000,0); //checking idle modules
#ifdef FILE_SERVER_ENABLED
	m_noise_manager.StartServer();
#endif
}

//
//
//
void FastTrackGiftDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void FastTrackGiftDll::ConnectToSupernode(const char* ip, int port)
{
	//m_socket.Connect(ip,port);
}

//
//
//
void FastTrackGiftDll::Log(const char* log)
{
	m_dlg.Log(log);
}

//
//
//
/*
void FastTrackGiftDll::Search(const char* search_string)
{
	m_connection_manager.Search(search_string);
}
*/

//
//
//
void FastTrackGiftDll::OnTimer(UINT nIDEvent)
{
	UINT timer_ret=1;
	m_dlg.KillTimer(nIDEvent);
	switch(nIDEvent)
	{
		case 1:	// 1 second
		{
			CheckPerformanceCounters();
			m_connection_manager.TimerHasFired();
			//m_supply_manager.OnOneSecondTimer();
			
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
			if(m_process_manager.IsProcessing())
			{
				int supply=0;
				m_process_manager.GetProcessProgress(supply);
				m_dlg.UpdateDataProcessProgressBar(supply);
			}
#endif
#endif
#ifdef SHARING_ENABLED
			UINT spoof_size,temp_size,spoof_index,round,hash_size;
			m_file_sharing_manager.GetSpoofEntriesSizes(spoof_size, temp_size,spoof_index,round,hash_size);
			m_dlg.UpdateSpoofSize(spoof_size,temp_size,spoof_index,round,hash_size);
#endif
			m_dlg.SetTimer(nIDEvent,1*1000,0);
			break;
		}
		case 2:	// 1 minute heartbeat
		{
			m_connection_manager.OnHeartbeat();	// write out the host cache to file
			

#ifndef HASH_GROWTH
//#ifndef NO_DCMASTER_RESPONDING
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
			//reset process manager everyday at 1am
			CTime current_time = CTime::GetCurrentTime();
			int hour = current_time.GetHour();
			int min = current_time.GetMinute();
			if(hour == 1 && min == 0)
			{
				m_process_manager.Reset();
			}

			//check to see if we need to process raw data
			if(m_process_manager.IsProcessing() == false &&
				m_process_manager.IsAllDone() == false)
			{
				m_process_manager.StartProcessRawData(m_processed_db_info, m_keyword_manager.ReturnProjectKeywords());
			}
#endif
#endif
//#endif
#endif
#ifdef SHARING_ENABLED
			m_file_sharing_manager.OnOneMinuteTimer();
#endif
#ifdef HASH_GROWTH
			m_db_manager.OnOneMinuteTimer();
#endif
			CheckModuleConnections();
			m_dlg.SetTimer(nIDEvent,60*1000,0);
			break;
		}
		case 3: // 5 minute timer
		{
			m_dlg.SetTimer(nIDEvent,5*60*1000,0);
			break;
		}
		case 4:
		{
#ifdef FILE_SERVER_ENABLED
			m_noise_manager.StartServer();
#endif
			break;
		}
		case 5:
		{
#ifdef FILE_SERVER_ENABLED
			m_noise_manager.ClearIdleModules();
			m_dlg.SetTimer(nIDEvent,30*1000,0);
#endif
			break;
		}
		case 6:
		{
			m_supply_manager.OnFiveSecondTimer();
			m_dlg.SetTimer(nIDEvent,5*1000,0);
			break;
		}
	}
}

//
//
//
void FastTrackGiftDll::ReportConnectionStatus(ConnectionModuleStatusData &status)
{
	vector<ConnectionModuleStatusData>::iterator data_iter=v_mod_status.begin();
	while(data_iter!=v_mod_status.end())
	{
		if(data_iter->m_mod==status.m_mod)
		{
			v_mod_status.erase(data_iter);
			break;
		}

		data_iter++;
	}

	v_mod_status.push_back(status);

	// Display the status on the dialog
	m_dlg.ReportConnectionStatus(status);//,&v_mod_status);


	// Write data to database
	for(UINT i=0; i<status.v_project_status.size(); i++)
	{
		if(status.v_project_status[i].v_query_hits.size()>0)
			m_db_manager.PushQueryHitData(status.v_project_status[i].v_query_hits);
	}

}

//
//
//
void FastTrackGiftDll::VendorCountsReady(vector<VendorCount>& vendor_counts)
{
	m_dlg.UpdateVendorCounts(vendor_counts);
}

//
//
//
void FastTrackGiftDll::InitPerformanceCounters()
{
	UINT timer_ret=1;

	int i;

	char iai_buf[2048];
	DWORD iai_buf_len=2048;
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
	while(strchr(iai->Description,'#')!=NULL)
	{
		*strchr(iai->Description,'#')='_';
	}

	CString network_adaptor = iai->Description;
	network_adaptor.Trim();
	m_keynames[0]="\\Processor(0)\\% Processor Time";
	m_keynames[1]="\\Network Interface(";
	m_keynames[1]+=network_adaptor;
	m_keynames[1]+=")\\Bytes Total/sec";
	m_keynames[2]="\\Network Interface(";
	m_keynames[2]+=network_adaptor;
	m_keynames[2]+=")\\Current Bandwidth";

    m_pdh=0;

    // Create the pdh query
    if(PdhOpenQuery(NULL,0,&m_pdh))
	{
		//MessageBox(NULL,"Error opening pdh query","Error",MB_OK);
		return;
	}

    // ADD A COUNTER TO THE QUERY
	for(i=0;i<3;i++)
	{
		PDH_STATUS error=PdhAddCounter(m_pdh,m_keynames[i].c_str(),NULL,&m_pdh_counters[i]);

		if(error)
		{
			//MessageBox(NULL,"Error adding counter to the pdh query","Error",MB_OK);
			return;
		}
	}
}

//
//
//
void FastTrackGiftDll::CheckPerformanceCounters()
{
	UINT i;

    // Collect the pdh query data
	if(PdhCollectQueryData(m_pdh))
	{
//		MessageBox(NULL,"Error collecting pdh query data","Error",MB_OK);
//		MessageBox(NULL,m_keynames[1].c_str(),"On this network adaptor",MB_OK);
		//return;
	}

	// Check the processor usage counter
	PDH_FMT_COUNTERVALUE value;
    PDH_STATUS error;

	error=PdhGetFormattedCounterValue(m_pdh_counters[0],PDH_FMT_LONG,0,&value);
	if(error)
	{
//		MessageBox(NULL,"Error formatting pdh counter data","Error",MB_OK);
//		MessageBox(NULL,m_keynames[1].c_str(),"On this network adaptor",MB_OK);
//		m_dlg.SetWindowText("Error formatting pdh coutner data");
		return;
	}

	m_proc_percent_usage=value.longValue;

	// If less than 90 % kick off another mod if there is less than min count of them
	if(/*(m_proc_percent_usage<90) & */(m_connection_manager.ReturnModCount()<(UINT)m_min_module_count))
	{
		m_connection_manager.AddModule();

		char log[1024];
		sprintf(log,"Module Added : (Current Count : %u) : (Proc % Usage : %u%%) : (Min Mod Count : %u)",
			m_connection_manager.ReturnModCount(),m_proc_percent_usage,m_min_module_count);
		m_dlg.Log(log);
	}
		

	// Check the bandwidth counter
	error=PdhGetFormattedCounterValue(m_pdh_counters[1],PDH_FMT_LONG,0,&value);
	if(error)
	{
//		MessageBox(NULL,"Error formatting bandwidth counter data","Error",MB_OK);
//		MessageBox(NULL,m_keynames[1].c_str(),"On this network adaptor",MB_OK);
//		m_dlg.SetWindowText("Error formatting pdh coutner data");
		return;
	}

	m_bandwidth_in_bytes=value.longValue;

	// Check the current bandwidth in bits per second
	error=PdhGetFormattedCounterValue(m_pdh_counters[2],PDH_FMT_LONG,0,&value);
	if(error)
	{
//		MessageBox(NULL,"Error formatting bandwidth in bits per second data","Error",MB_OK);
//		MessageBox(NULL,m_keynames[1].c_str(),"On this network adaptor",MB_OK);
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
	char mem_buf[1024];
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
	UINT qh_size;
	m_db_manager.ReportDBStatus(qh_size);
	sprintf(buf, "%u", qh_size);
	m_dlg.GetDlgItem(IDC_QH_SIZE_STATIC)->SetWindowText(buf);

	//if less than m_auto_resume_threshold cpu, resuming searching, if more than m_auto_pause_threshold cpu, pause searching
	if((m_proc_percent_usage <= m_auto_resume_threshold) && (qh_size < 10000))
	{
		m_dlg.AutoPause(false);
		m_supply_manager.AutoPauseSearching(false);
	}
	else if((m_proc_percent_usage >= m_auto_pause_threshold) || (qh_size >= 10000) )
	{
		m_dlg.AutoPause(true);
		m_supply_manager.AutoPauseSearching(true);
	}
}

//
//
//
void FastTrackGiftDll::ClosePerformanceCounters()
{
	 PdhCloseQuery(m_pdh);
}

//
//
//
void FastTrackGiftDll::CheckModuleConnections()
{
	UINT i;

	int mod_count=m_connection_manager.ReturnModCount();

	// Check to see that the mod count is at least as big as the min.  If not, let it get to that point
	if(mod_count<m_min_module_count)
	{
		return;
	}

	// If all of modules are all below 30 connected sockets, remove a module if we are above the min module count
	if(mod_count>m_min_module_count)
	{
		bool remove=true;
		for(i=0;i<v_mod_status.size();i++)
		{
			if(v_mod_status[i].m_connected_socket_count>=30)
			{
				remove=false;
				break;
			}
		}

		if(remove)
		{
			char log[1024];
			sprintf(log,"Module Removed : (Current Count : %u) : (Connected Socket Counts :",mod_count-1);
			
			char val[32];
			for(i=0;i<v_mod_status.size();i++)
			{
				strcat(log," ");
				_itoa(v_mod_status[i].m_connected_socket_count,val,10);
				strcat(log,val);
			}
			strcat(log,")\n");
			
			m_dlg.Log(log);
			
			// Remove the module
			m_connection_manager.LimitModuleCount(mod_count-1);
			m_dlg.ModuleCountHasChanged(mod_count-1);

			// Clear the status vector so that only the remaining modules will have status members
			v_mod_status.clear();

			return;	// if we just removed a mod, we ain't gonna add one
		}
	}

	// If all of the modules are all greater than 40 connected sockets, add a module if we are below the max mod count
	if(mod_count<m_max_module_count)
	{
		bool add=true;
		int not_enough_count=0;
		for(i=0;i<v_mod_status.size();i++)
		{
			if(v_mod_status[i].m_connected_socket_count<40)
			{
				add=false;
				not_enough_count++;
			}
		}

		if(add)
		{
			m_connection_manager.AddModule();

			char log[1024];
			sprintf(log,"Module Added : (Current Count : %u) : (Proc % Usage : %u%%) : (Min Mod Count : %u)",
				m_connection_manager.ReturnModCount(),m_proc_percent_usage,m_min_module_count);
			m_dlg.Log(log);
		}
		else
		{
			// Log why we didn't add a module (not all of the modules were connected to at least 40 modules)
			char log[1024];
			sprintf(log,"Module NOT Added : %u / %u modules were not connected to at least 40 Supernodes",not_enough_count,v_mod_status.size());
			m_dlg.Log(log);
		}
	}
}

//
//
//
//
//
//
int FastTrackGiftDll::AlterModuleCounts(int min,int max)
{
	// Check for special data queying calls
	if(min==max)
	{
		if(min==-1)	// if min and max are -1, return the min value
		{
			return m_min_module_count;
		}
		if(min==1)	// if min and max are 1, return the max value
		{
			return m_max_module_count;
		}
	}
	
	m_min_module_count+=min;
	m_max_module_count+=max;

	// If changing the max, it can't go below the min
	if(max!=0)
	{
		if(m_max_module_count<m_min_module_count)
		{
			m_max_module_count=m_min_module_count;
		}
	}
	else	// If changing the min, it will increase the max if the min is greater than the max
	{
		if(m_min_module_count>m_max_module_count)
		{
			m_max_module_count=m_min_module_count;
		}
	}

	// Limit both the min and max module counts to fall within 0 and 100
	if(m_min_module_count<0)
	{
		m_min_module_count=0;
	}
	
	else if(m_min_module_count>200)
	{
		m_min_module_count=200;
	}
	
	if(m_max_module_count<0)
	{
		m_max_module_count=0;
	}
	
	else if(m_max_module_count>200)
	{
		m_max_module_count=200;
	}
	

	// Check to see if we need to delete some mods due to a decrease in the max
	m_connection_manager.LimitModuleCount(m_max_module_count);

	// Store in the registry
	RegistryKeys::SetModuleCounts(m_min_module_count,m_max_module_count);

	// If the max was changed, return the new max, else return the new min
	if(max!=0)
	{
		return m_max_module_count;
	}
	else
	{
		return m_min_module_count;
	}
}

//
//
//
void FastTrackGiftDll::ShowSocketStatus(const char* msg)
{
	m_dlg.GetDlgItem(IDC_Connection_Status_Static)->SetWindowText(msg);
}

//
//
//
void FastTrackGiftDll::KeywordsUpdated()
{
#ifdef SHARING_ENABLED
	m_file_sharing_manager.KeywordsUpdated();
#endif
	m_dlg.KeywordsUpdated(m_keyword_manager.ReturnProjectKeywords());
}

//
//
//
void FastTrackGiftDll::StopGatheringProjectData(string &project)
{
	// Tell the dialog to clear the status of this project
	m_dlg.ClearProjectStatus(project);
}

//
//
//
void FastTrackGiftDll::RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update)
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
//
//
//
int FastTrackGiftDll::AlterSupplyIntervalMultiplier(int multi)
{
	m_supply_interval_multi+=multi;

	// Limit the multi to fall within 1 and 24
	if(m_supply_interval_multi<1)
	{
		m_supply_interval_multi=1;
	}
	else if(m_supply_interval_multi>24)
	{
		m_supply_interval_multi=24;
	}

	// Store in the registry
	RegistryKeys::SetSupplyIntervalMultiplier(m_supply_interval_multi);

	m_supply_manager.AlterSupplyIntervalMultiplier(m_supply_interval_multi);

	return m_supply_interval_multi;
}

//
//
//
int FastTrackGiftDll::GetSupplyIntervalMultiplier()
{
	return m_supply_interval_multi;
}

//
//
//
void FastTrackGiftDll::InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
	m_db_manager.InitSupplyThreadData(wparam,lparam);
}

//
//
//
void FastTrackGiftDll::ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam)
{
	m_db_manager.ReadyToWriteSupplyDataToDatabase(wparam,lparam);
}

//
//
//
void FastTrackGiftDll::DBMaintenanceReadyToStart(void)
{
	m_db_manager.DBMaintenanceReadyToStart();
}

//
//
//
void FastTrackGiftDll::DBMaintenanceFinished(void)
{
	m_db_manager.DBMaintenanceFinished();
}

//
//
//
void FastTrackGiftDll::RestartSearching()
{
	m_supply_manager.RestartSearching();
}

//
//
//
int FastTrackGiftDll::GetModCount()
{
	int count=0;
#ifdef FILE_SERVER_ENABLED
	count=m_noise_manager.GetModCount();
#endif
	return count;
}

//
//
//
UINT FastTrackGiftDll::GetBandwidthInBytes()
{
	return m_bandwidth_in_bytes;
}

//
//
//
UINT FastTrackGiftDll::GetBandwdithInBitsPerSecond()
{
	return m_current_bandwdith_in_bits_per_second;
}

//
//
//
void FastTrackGiftDll::ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status)
{
	m_dlg.ReportNoiseManagerStatus(status);
}

//
//
//
void FastTrackGiftDll::DataReceived(char *source_name, void *data, int data_length)
{
#ifndef NO_DCMASTER_RESPONDING
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
#ifdef DATA_COLLECTION_ENABLED
			header.op = DCHeader::FastTrack_DC_Init_Response;
#endif
#ifdef SHARING_ENABLED
#ifdef THROTTLED_DECOYING
			header.op = DCHeader::FastTrack_Decoyer_Init_Response;
#else
#ifdef FILE_SERVER_ENABLED
			header.op = DCHeader::FastTrack_Swarmer_Init_Response;
#else
			header.op = DCHeader::FastTrack_Spoofer_Init_Response;
#endif
#endif
#endif
			header.size=0;
			m_com.SendReliableData(source_name, &header, sizeof(DCHeader));
			// Log this message
			m_dlg.Log("Received Init Message from DCMaster");
			if((strlen(source_name)+1) < sizeof(m_dc_master))
				strcpy(m_dc_master,source_name);
			break;
		}
		case DCHeader::ProjectChecksums:
		{
			ProjectChecksums current_projects;
			current_projects.ReadFromBuffer(ptr);

			// Log this message
			char log[256];
			sprintf(log,"Received ProjectChecksums Message from DCMaster : Received %u Checksums",current_projects.v_checksums.size());
			m_dlg.Log(log);
			m_keyword_manager.CheckProjectChecksums(source_name, current_projects);
			break;
		}
		case DCHeader::ProjectKeywords:
		{
			ProjectKeywordsVector keywords;
			keywords.ReadFromBuffer(ptr);

			// Log this message
			char log[256];
			sprintf(log,"Received ProjectKeywords from DCMaster : %d projects",(int)keywords.v_projects.size());
			m_dlg.Log(log);
			m_keyword_manager.NewProjectKeywordsReceived(keywords.v_projects);
			break;
		}
		case DCHeader::DatabaseInfo:
		{
			m_processed_db_info.ReadFromBuffer(ptr);
			break;
		}
	}
#endif
}

//
//
//
int FastTrackGiftDll::GetMinModuleCount()
{
	return m_min_module_count;
}


//
//
//
void FastTrackGiftDll::InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
	m_process_manager.InitSupplyThreadData(wparam, lparam);
#endif
#endif
}


//
//
//
void FastTrackGiftDll::ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam)
{
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
	m_process_manager.SupplyProcessFinished();
#endif
#endif
}


//
//
//
void FastTrackGiftDll::StartDataProcess()
{
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
	m_process_manager.StartProcessRawData(m_processed_db_info, m_keyword_manager.ReturnProjectKeywords());
#endif
#endif
}

//
//
//
void FastTrackGiftDll::ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam)
{
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
	m_process_manager.MaintenanceFinished();
#endif
#endif
}

//
//
//
bool FastTrackGiftDll::IsDBMaintaining()
{
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
	return m_process_manager.IsMaintaining();
#endif
#endif
	return false;
}

//
//
//
void FastTrackGiftDll::SpoofEntriesRetrievalStarted()
{
	SendSupplyRetrievalStartingTime();
	m_dlg.SpoofEntriesRetrievalStarted();
}

//
//
//
void FastTrackGiftDll::InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam)
{
#ifdef SHARING_ENABLED
	m_file_sharing_manager.InitThreadData(wparam,lparam);
#endif
}

//
//
//
void FastTrackGiftDll::DoneRetreivingSpoofEntries()
{
#ifdef SHARING_ENABLED
	m_file_sharing_manager.DoneRetreivingSpoofEntries();
	SendSupplyRetrievalEndingTime();
#endif
}

//
//
//
void FastTrackGiftDll::UpdateFileSharingManagerSupplyProjects(WPARAM wparam,LPARAM lparam)
{
#ifdef SHARING_ENABLED
	m_file_sharing_manager.UpdateFileSharingManagerSupplyProjects(wparam,lparam);
#endif
}

//
//
//
void FastTrackGiftDll::ReConnectToAllSupernodes()
{
	m_connection_manager.ReConnectAll();
#ifdef SHARING_ENABLED
	m_file_sharing_manager.ResetSpoofsCount();
#endif
}

//
//
//
void FastTrackGiftDll::RemoveAllModules()
{
	m_dlg.ModuleCountHasChanged(0);
	// Clear the status vector so that only the remaining modules will have status members
	v_mod_status.clear();
}

//
//
//
void FastTrackGiftDll::ManualPauseSearching(bool pause)
{
	m_supply_manager.ManualPauseSearching(pause);
}

//
//
//
void FastTrackGiftDll::ApplyAutoSearchingThreshold(int pause_threshold, int resume_threshold)
{
	m_auto_pause_threshold = pause_threshold;
	m_auto_resume_threshold = resume_threshold;
}

//
//
//
void FastTrackGiftDll::FileSharingManagerSupplyRetrievalFailed()
{
#ifdef SHARING_ENABLED
	m_file_sharing_manager.FileSharingManagerSupplyRetrievalFailed();
#endif
}

//
//
//
void FastTrackGiftDll::SendSupplyRetrievalStartingTime()
{
	int buf_len = sizeof(DCHeader);
	unsigned char* buf = new unsigned char[buf_len];

	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::GiftSupplyRetrievalStarted;
	header->size =0;
	m_com.SendReliableData(m_dc_master, buf, buf_len);
	delete [] buf;
}

//
//
void FastTrackGiftDll::SendSupplyRetrievalEndingTime()
{
	int buf_len = sizeof(DCHeader);
	unsigned char* buf = new unsigned char[buf_len];

	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::GiftSupplyRetrievalEnded;
	header->size =0;
	m_com.SendReliableData(m_dc_master, buf, buf_len);
	delete [] buf;
}

//
//
//
void FastTrackGiftDll::SendSupplyRetrievalProgress(int progress)
{
	int buf_len = sizeof(DCHeader)+sizeof(int);
	unsigned char* buf = new unsigned char[buf_len];

	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::GiftSupplyRetrievalProgress;
	header->size =sizeof(int);
	int* ptr_progress = (int*)&buf[sizeof(DCHeader)];
	*ptr_progress = progress;
	m_com.SendReliableData(m_dc_master, buf, buf_len);
	delete [] buf;
}

//
//
//
void FastTrackGiftDll::EnableVendorCounts(BOOL enable)
{
	m_connection_manager.m_vendor_counts_enabled = (bool)enable;
}

//
//
//
bool FastTrackGiftDll::ReadyToReconnect()
{
	return m_db_manager.ReadyToReconnect();
}