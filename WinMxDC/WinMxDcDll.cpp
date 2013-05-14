#include "StdAfx.h"
#include "DCHeader.h"
#include "WSocket.h"
#include "ProjectKeywordsVector.h"
#include "StdAfx.h"
#include "winmxdcdll.h"
#include "RegistryKeys.h"

WinMxDcDll::WinMxDcDll(void)
{
	WSocket::Startup();
}

//
//
//
WinMxDcDll::~WinMxDcDll(void)
{
}

//
//
//
void WinMxDcDll::DllInitialize()
{
	m_auto_pause_threshold = 50;
	m_auto_resume_threshold = 30;
}

//
//
//
void WinMxDcDll::DllUnInitialize()
{
	WSocket::Cleanup();
	m_connection_manager.KillModules();	// hopefully avoids crash on closing

	ClosePerformanceCounters();
	m_dlg.DestroyWindow();
#ifdef WINMX_DC
	m_db_manager.TerminateThread();
	m_process_manager.KillThread();
	m_supply_manager.TerminateThread();
#endif
#ifdef WINMX_SWARMER
	m_noise_manager.KillModules();	// hopefully avoids crash on closing
#endif
	m_dlg.OnUnInitialize();

}

//
//
//
void WinMxDcDll::DllStart()
{
	RegistryKeys::GetModuleCounts(&m_min_module_count,&m_max_module_count);
	RegistryKeys::GetSupplyIntervalMultiplier(&m_supply_interval_multi);
	m_dlg.Create(IDD_WINMX_DC_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
#ifdef WINMX_SPOOFER
	m_syncher.Register(this, "WinMXSupplies");
	m_syncher.InitParent(this);
	m_username_syncher.m_id = 1;
	m_username_syncher.Register(this, "Kazaa-Launcher");
	m_username_syncher.InitParent(this);
#endif

	m_connection_manager.SetKeywordManager(&m_keyword_manager);
#ifdef WINMX_DC
	m_connection_manager.SetSupplyManager(&m_supply_manager);
#endif
#ifdef WINMX_SPOOFER
	m_connection_manager.SetFileSharingManager(&m_file_sharing_manager);
#endif

	m_connection_manager.InitParent(this);
	m_keyword_manager.SetConnectionManager(&m_connection_manager);
#ifdef WINMX_DC
	m_keyword_manager.SetSupplyManager(&m_supply_manager);
#endif
	m_keyword_manager.InitParent(this);

#ifdef WINMX_DC
	m_supply_manager.SetConnectionManager(&m_connection_manager);
	m_supply_manager.SetKeywordManager(&m_keyword_manager);
	m_supply_manager.InitParent(this);
	m_supply_manager.AlterSupplyIntervalMultiplier(m_supply_interval_multi);
	m_db_manager.InitParent(this);
	m_process_manager.InitParent(this);
//#ifdef _DEBUG
//	m_processed_db_info.m_db_host="127.0.0.1";
//#else
	m_processed_db_info.m_db_host="dcmaster.mediadefender.com";
//#endif
	m_processed_db_info.m_db_user="onsystems";
	m_processed_db_info.m_db_password="ebertsux37";
#endif

#ifdef WINMX_SPOOFER
	m_file_sharing_manager.SetKeywordManager(&m_keyword_manager);
	m_file_sharing_manager.InitParent(this);
#endif
#ifdef WINMX_SWARMER
	m_noise_manager.InitParent(this);
	m_noise_manager.SetFileSharingManager(&m_file_sharing_manager);
#endif

	//register com service with DCMaster ID 35
	m_com.Register(this, 35);

	m_keyword_manager.BroadcastUpdateAlert();

	InitPerformanceCounters();
	m_dlg.SetTimer(1,1000,0);
	m_dlg.SetTimer(2,60*1000,0);
	m_dlg.SetTimer(3,5*60*1000,0);
	m_dlg.SetTimer(4,10*1000,0);
#ifdef WINMX_SWARMER
	m_dlg.SetTimer(5,30*1000,0);
	m_noise_manager.StartServer();
#endif
}

//
//
//
void WinMxDcDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void WinMxDcDll::Log(const char* log)
{
	m_dlg.Log(log);
}

//
//
//
void WinMxDcDll::OnTimer(UINT nIDEvent)
{
	UINT timer_ret=1;
	m_dlg.KillTimer(nIDEvent);
	switch(nIDEvent)
	{
		case 1:	// 1 second
		{
			CheckPerformanceCounters();
			m_connection_manager.TimerHasFired();
#ifdef WINMX_DC
			if(m_process_manager.IsProcessing())
			{
				int supply=0;
				m_process_manager.GetProcessProgress(supply);
				m_dlg.UpdateDataProcessProgressBar(supply);
			}
#endif
#ifdef WINMX_SPOOFER
			UINT spoof_size,spoof_index,round,hash_size;
			m_file_sharing_manager.GetSpoofEntriesSizes(spoof_size, spoof_index,round,hash_size);
			m_dlg.UpdateSpoofSize(spoof_size,spoof_index,round,hash_size);
#endif
			m_dlg.SetTimer(nIDEvent,1*1000,0);
			break;
		}
		case 2:	// 1 minute
		{
#ifdef WINMX_DC
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
			m_dlg.SetTimer(nIDEvent,60*1000,0);
			break;
		}
		case 3: // 5 minute timer
		{
			m_connection_manager.OnFiveMinuteTimer();	// write out the host cache to file
			CheckModuleConnections();
			m_dlg.SetTimer(nIDEvent,5*60*1000,0);
			break;
		}
		case 4:
		{
#ifdef WINMX_DC
			m_supply_manager.OnTenSecondTimer();
#endif
			m_dlg.SetTimer(nIDEvent,10*1000,0);
			break;
		}
#ifdef WINMX_SWARMER
		case 5: //30 sec
		{
			m_noise_manager.ClearIdleModules();
			m_dlg.SetTimer(nIDEvent,30*1000,0);
		}
#endif
	}
}

//
//
//
void WinMxDcDll::ReportConnectionStatus(ConnectionModuleStatusData &status)
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

#ifdef WINMX_DC
	// Write data to database
	for(UINT i=0; i<status.v_project_status.size(); i++)
	{
		if(status.v_project_status[i].v_query_hits.size()>0)
			m_db_manager.PushQueryHitData(status.v_project_status[i].v_query_hits);
	}
#endif
}

//
//
//
void WinMxDcDll::InitPerformanceCounters()
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
void WinMxDcDll::CheckPerformanceCounters()
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
	if((m_proc_percent_usage<90) & (m_connection_manager.ReturnModCount()<(UINT)m_min_module_count))
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

#ifdef WINMX_DC
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
#endif
}

//
//
//
void WinMxDcDll::ClosePerformanceCounters()
{
	 PdhCloseQuery(m_pdh);
}

//
//
//
void WinMxDcDll::CheckModuleConnections()
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
int WinMxDcDll::AlterModuleCounts(int min,int max)
{
	v_mod_status.clear();
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
void WinMxDcDll::ShowSocketStatus(const char* msg)
{
	m_dlg.GetDlgItem(IDC_Connection_Status_Static)->SetWindowText(msg);
}

//
//
//
void WinMxDcDll::KeywordsUpdated()
{
	m_dlg.KeywordsUpdated(m_keyword_manager.ReturnProjectKeywords());
#ifdef WINMX_SPOOFER
	m_file_sharing_manager.KeywordsUpdated();
#endif
}

//
//
//
void WinMxDcDll::StopGatheringProjectData(string &project)
{
	// Tell the dialog to clear the status of this project
	m_dlg.ClearProjectStatus(project);
}

//
//
//
void WinMxDcDll::RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update)
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
int WinMxDcDll::AlterSupplyIntervalMultiplier(int multi)
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
#ifdef WINMX_DC
	m_supply_manager.AlterSupplyIntervalMultiplier(m_supply_interval_multi);
#endif
	return m_supply_interval_multi;
}

//
//
//
int WinMxDcDll::GetSupplyIntervalMultiplier()
{
	return m_supply_interval_multi;
}

//
//
//
void WinMxDcDll::InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
#ifdef WINMX_DC
	m_db_manager.InitSupplyThreadData(wparam,lparam);
#endif
}

//
//
//
void WinMxDcDll::ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam)
{
#ifdef WINMX_DC
	m_db_manager.ReadyToWriteSupplyDataToDatabase(wparam,lparam);
#endif
}

//
//
//
void WinMxDcDll::DBMaintenanceReadyToStart(void)
{
#ifdef WINMX_DC
	m_db_manager.DBMaintenanceReadyToStart();
#endif
}

//
//
//
void WinMxDcDll::DBMaintenanceFinished(void)
{
#ifdef WINMX_DC
	m_db_manager.DBMaintenanceFinished();
	m_supply_manager.RetreiveNewSpoofEntries();
#endif
}

//
//
//
void WinMxDcDll::RestartSearching()
{
#ifdef WINMX_DC
	m_supply_manager.RestartSearching();
#endif
}

//
//
//
UINT WinMxDcDll::GetBandwidthInBytes()
{
	return m_bandwidth_in_bytes;
}

//
//
//
UINT WinMxDcDll::GetBandwdithInBitsPerSecond()
{
	return m_current_bandwdith_in_bits_per_second;
}

//
//
//
void WinMxDcDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	//char log[1024];
	char* ptr = (char*)data;
	DCHeader* header = (DCHeader*)ptr;
	ptr+= sizeof(DCHeader);
	switch (header->op)
	{
		case DCHeader::Init:
		{
			DCHeader header;
#ifdef WINMX_DC
			header.op = DCHeader::WinMx_DC_Init_Response;
#endif
#ifdef WINMX_SPOOFER
			header.op = DCHeader::WinMx_Spoofer_Init_Response;
#endif
#ifdef WINMX_SWARMER
			header.op = DCHeader::WinMx_Swarmer_Init_Response;
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
#ifdef WINMX_DC
		case DCHeader::DatabaseInfo:
		{
			m_processed_db_info.ReadFromBuffer(ptr);
			break;
		}
#endif
	}
}

//
//
//
int WinMxDcDll::GetMinModuleCount()
{
	return m_min_module_count;
}


//
//
//
void WinMxDcDll::InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
#ifdef WINMX_DC
	m_process_manager.InitSupplyThreadData(wparam, lparam);
#endif
}


//
//
//
void WinMxDcDll::ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam)
{
#ifdef WINMX_DC
	m_process_manager.SupplyProcessFinished();
#endif
}


//
//
//
void WinMxDcDll::StartDataProcess()
{
#ifdef WINMX_DC
	m_process_manager.StartProcessRawData(m_processed_db_info, m_keyword_manager.ReturnProjectKeywords());
#endif
}

//
//
//
void WinMxDcDll::ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam)
{
#ifdef WINMX_DC
	m_process_manager.MaintenanceFinished();
#endif
}

//
//
//
bool WinMxDcDll::IsDBMaintaining()
{
#ifdef WINMX_DC
	return m_process_manager.IsMaintaining();
#else
	return false;
#endif
}

//
//
//
void WinMxDcDll::ReConnectToAllSupernodes()
{
	m_connection_manager.ReConnectAll();
#ifdef WINMX_SPOOFER
	m_file_sharing_manager.ResetSpoofsCount();
#endif
}

//
//
//
void WinMxDcDll::RemoveAllModules()
{
	m_dlg.ModuleCountHasChanged(0);
	// Clear the status vector so that only the remaining modules will have status members
	v_mod_status.clear();
}

//
//
//
void WinMxDcDll::ManualPauseSearching(bool pause)
{
#ifdef WINMX_DC
	m_supply_manager.ManualPauseSearching(pause);
#endif
}

//
//
//
void WinMxDcDll::ApplyAutoSearchingThreshold(int pause_threshold, int resume_threshold)
{
	m_auto_pause_threshold = pause_threshold;
	m_auto_resume_threshold = resume_threshold;
}

//
//
//
bool WinMxDcDll::IsDBReady()
{
	UINT queue_size=0;
#ifdef WINMX_DC
	m_db_manager.ReportDBStatus(queue_size);
#endif
	if(queue_size >= 1000)
		return false;
	else
		return true;
}

//
//
//
void WinMxDcDll::SpoofEntriesRetrievalStarted()
{
	m_dlg.SpoofEntriesRetrievalStarted();
}

//
//
//
void WinMxDcDll::DoneRetreivingSpoofEntries()
{
#ifdef WINMX_DC
	m_supply_manager.DoneRetreivingSpoofEntries();
#endif
}

//
//
//
void WinMxDcDll::SupplyManagerSupplyRetrievalFailed()
{
#ifdef WINMX_DC
	m_supply_manager.SupplyManagerSupplyRetrievalFailed();
#endif
}

//
//
//
void WinMxDcDll::InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam)
{
#ifdef WINMX_DC
	m_supply_manager.InitThreadData(wparam,lparam);
#endif
}

//
//
//
void WinMxDcDll::SupplySynched(const char* source_ip)
{
#ifdef WINMX_SPOOFER
 //TODO: Recevied a new set of fake supplies, notify the FileSharingManager about that here
	m_dlg.Log("Supplies are synched, reloading supplies from files");
	m_file_sharing_manager.DoneRetreivingSpoofEntries();
#endif
}

//
//
//
void WinMxDcDll::StopSearchingProject(const char* project_name)
{
	CString msg;
	msg.Format("Stop searching for project \"%s\" due to over 100K results have returned",project_name);
	m_dlg.Log(msg);
	m_connection_manager.StopSearchingProject(project_name);
}

//
//
//
int WinMxDcDll::GetModCount()
{
	int count=0;
#ifdef WINMX_SWARMER
	count=m_noise_manager.GetModCount();
#endif
	return count;
}

//
//
//
void WinMxDcDll::ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status)
{
#ifdef WINMX_SWARMER
	m_dlg.ReportNoiseManagerStatus(status);
#endif
}
