#include "StdAfx.h"
#include "gnutellasupplydll.h"
#include "RegistryKeys.h"
#include "Iphlpapi.h"	// for GetAdaptersInfo (also need Iphlpapi.lib)
#include "Psapi.h"	// for GetProcessMemoryInfo()
#include "../DCMaster/ProjectKeywordsVector.h"
#include "..\TKSyncher\TKSyncherInterface.h"

#define TIMER_LENGTH							1		// seconds
#define HEARTBEAT_INTERVAL						60		// seconds
#define BEASTIE_STATUS_INTERVAL					30		// seconds
#define MODULE_CONNECTION_CHECK_INTERVAL		5*60	// seconds

GnutellaSupplyDll::GnutellaSupplyDll(void)
{
}

//
//
//
GnutellaSupplyDll::~GnutellaSupplyDll(void)
{
}

//
//
//
void GnutellaSupplyDll::DllInitialize()
{
	BOOL ret=AfxInitRichEdit();	// for rich edit in the log window
	WSocket::Startup(); // Start Windows Sockets
	RegistryKeys::GetModuleCounts(&m_min_module_count,&m_max_module_count);
	RegistryKeys::GetSupplyIntervalMultiplier(&m_supply_interval_multi);
	m_supply_manager.AlterSupplyIntervalMultiplier(m_supply_interval_multi);

	m_dlg.Create(IDD_GnutellaSupply_Dialog,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_log_window_manager.InitParent(this);
	m_connection_manager.InitParent(this);
	m_keyword_manager.InitParent(this);
	m_supply_manager.InitParent(this);
//	m_logfile_manager.InitParent(this);
	m_db_manager.InitParent(this);
	m_process_manager.InitParent(this);
	m_connection_manager.InitKeywordManager(&m_keyword_manager);
	m_connection_manager.InitSupplyManager(&m_supply_manager);
	m_connection_manager.InitDBManager(&m_db_manager);
	m_supply_manager.InitConnectionManager(&m_connection_manager);
	m_keyword_manager.SetConnectionManager(&m_connection_manager);
	m_keyword_manager.SetSupplyManager(&m_supply_manager);
}

//
//
//
void GnutellaSupplyDll::DllUnInitialize()
{
	m_db_manager.TerminateThread();
	m_process_manager.KillThread();
	m_connection_manager.KillModules();	// hopefully avoids crash on closing
	ClosePerformanceCounters();
	m_dlg.DestroyWindow();
}

//
//
//
void GnutellaSupplyDll::DllStart()
{

	UINT timer_ret=1;
	m_com.Register(this, 35);

	//disabling data processing
	m_dlg.SetTimer(1, HEARTBEAT_INTERVAL*1000, NULL);	// 1 minute heartbeat *&*
	
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

#ifdef _DEBUG
	m_processed_db_info.m_db_host = "localhost";
#else
	m_processed_db_info.m_db_host = "216.152.250.181";
#endif
	m_processed_db_info.m_db_user = "onsystems";
	m_processed_db_info.m_db_password = "ebertsux37";
	
	//m_process_manager.DBMaintenance();
	m_dlg.SetTimer(7,60*60*1000,NULL);	// 60 mins to write out host caches and sync the cache file out
	m_process_manager.StartProcessRawData(m_processed_db_info);
}

//
//
//
void GnutellaSupplyDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void GnutellaSupplyDll::DataReceived(char *source_name, void *data, int data_length)
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
			header.op = DCHeader::Gnutella_DC_Init_Response;
			header.size=0;
			m_com.SendReliableData(source_name, &header, sizeof(DCHeader));
			// Log this message
			m_log_window_manager.Log("Received ");
			m_log_window_manager.Log("Init",0x00008800,true);	// bold green text
			m_log_window_manager.Log(" Message from ");
			m_log_window_manager.Log(source_name,0,true);	// bold
			m_log_window_manager.Log("\n");
			break;
		}
		case DCHeader::ProjectChecksums:
		{
			ProjectChecksums current_projects;
			current_projects.ReadFromBuffer(ptr);

			// Log this message
			m_log_window_manager.Log("Received ");
			m_log_window_manager.Log("ProjectChecksums",0x00008800,true);	// bold green text
			m_log_window_manager.Log(" Message from ");
			m_log_window_manager.Log(source_name,0,true);	// bold
			m_log_window_manager.Log(" : Received ");
			sprintf(log,"%u",current_projects.v_checksums.size());
			m_log_window_manager.Log(log,0x000000FF,true);	// bold red text
			m_log_window_manager.Log(" Checksums\n");

			m_keyword_manager.CheckProjectChecksums(source_name, current_projects);
			break;
		}
		/*
		case DCHeader::ProjectKeywordsStart:
		{
			// Log this message
			m_log_window_manager.Log("Received ");
			m_log_window_manager.Log("ProjectKeywordsStart",0x00008800,true);	// bold green text
			m_log_window_manager.Log(" Message from ");
			m_log_window_manager.Log(source_name,0,true);	// bold
			m_log_window_manager.Log("\n");

			v_new_project_keywords.clear();
			break;
		}
		*/
		case DCHeader::ProjectKeywords:
		{
			//v_new_project_keywords.clear();

			//ProjectKeywords keywords;
			//keywords.ReadFromBuffer(ptr);
			ProjectKeywordsVector keywords;
			keywords.ReadFromBuffer(ptr);

			// Log this message
			m_log_window_manager.Log("Received ");
			m_log_window_manager.Log("ProjectKeywords",0x00008800,true);	// bold green text
			m_log_window_manager.Log(" Message from ");
			m_log_window_manager.Log(source_name,0,true);	// bold
			m_log_window_manager.Log(" : ");
			//sprintf(log,"%s\n",keywords.m_project_name.c_str());
			sprintf(log,"%d projects\n", (int)keywords.v_projects.size());
			m_log_window_manager.Log(log,0x00FF0000,true);	// bold blue text

			//v_new_project_keywords.push_back(keywords);

			m_keyword_manager.NewProjectKeywordsReceived(keywords.v_projects);
			break;
		}
		/*
		case DCHeader::ProjectKeywordsEnd:
		{
			// Log this message
			m_log_window_manager.Log("Received ");
			m_log_window_manager.Log("ProjectKeywordsEnd",0x00008800,true);	// bold green text
			m_log_window_manager.Log(" Message from ");
			m_log_window_manager.Log(source_name,0,true);	// bold
			m_log_window_manager.Log("\n");
			
			m_keyword_manager.NewProjectKeywordsReceived(v_new_project_keywords);

			break;
		}
		*/
		/*
		case DCHeader::DatabaseInfo:
		{
			m_processed_db_info.ReadFromBuffer(ptr);
			break;
		}
		*/
	}
}

//
//
//
void GnutellaSupplyDll::ReportConnectionStatus(ConnectionModuleStatusData &status)
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

	// Tell the search manager that there is status that he might want to look at
//	m_search_manager.ReportConnectionStatus(status);

	// Extract status for the reporting
//	ExtractProjectStatusReportData(status);

	// Display the status on the dialog
	m_dlg.ReportConnectionStatus(status,&v_mod_status);

	// Write out the logfiles
//	m_logfile_manager.Log(status);

	// Write data to database
	for(UINT i=0; i<status.v_project_status.size(); i++)
	{
		m_db_manager.PushQueryHitData(status.v_project_status[i].v_query_hits);
		m_db_manager.PushQueryData(status.v_project_status[i].v_queries);
	}
}

//
//
//
void GnutellaSupplyDll::VendorCountsReady(vector<VendorCount>& vendor_counts)
{
	m_dlg.UpdateVendorCounts(vendor_counts);
}

//
//
//
void GnutellaSupplyDll::Log(char* msg)
{
	m_dlg.GetDlgItem(IDC_Connection_Status_Static)->SetWindowText(msg);
}

//
//
//
void GnutellaSupplyDll::RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update)
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
void GnutellaSupplyDll::KeywordsUpdated()
{
	m_dlg.KeywordsUpdated(m_keyword_manager.v_project_keywords);
	m_process_manager.KeywordUpdated(m_keyword_manager.v_project_keywords);
}

//
//
//
/*
void GnutellaSupplyDll::UpdateSupplyProjects(vector<SupplyProject> &supply_projects)
{
	m_dlg.SupplyUpdated(supply_projects);
}
*/
//
//
//
void GnutellaSupplyDll::StopGatheringProjectData(string &project)
{
	// Tell the dialog to clear the status of this project
	m_dlg.ClearProjectStatus(project);

/* =>_<=
	// Find the correct project reporting status object, send it, and delete it
	vector<ProjectStatusReportData>::iterator data_iter=v_project_status_report_data.begin();
	while(data_iter!=v_project_status_report_data.end())
	{
		if(strcmp(data_iter->m_project_name.c_str(),project.c_str())==0)
		{
			data_iter->m_end_time=CTime::GetCurrentTime();
			break;
		}

		data_iter++;
	}

	// Make sure we found the correct data item
	if(data_iter==v_project_status_report_data.end())
	{
		return;
	}

	unsigned int buf_len=sizeof(BeastieHeader)+data_iter->GetBufferLength();
	char *buf=new char[buf_len];
	BeastieHeader *hdr=(BeastieHeader *)buf;
	hdr->op=BeastieHeader::ProjectStatus;
	hdr->size=data_iter->WriteToBuffer(&buf[sizeof(BeastieHeader)]);

	// If the beast master is online, send him the data, else write it out to the data to be sent file
	if(DllInterface::IsUserOnline(m_master))
	{
		// First check to see if we need to send stuff to the beast master
		CheckProjectStatusReportDataFile();

		DllInterface::SendAppData(m_master,buf_len,buf);
	}
	else
	{
		SaveProjectStatusReportDataToFile(buf,buf_len);
	}

	delete [] buf;

	// Remove the item from the vector
	v_project_status_report_data.erase(data_iter);
=>_<= */
}

//
//
//
void GnutellaSupplyDll::ShowLogWindow()
{
	m_log_window_manager.ShowWindow();
}

//
//
//
void GnutellaSupplyDll::OnTimer(UINT nIDEvent)
{
	UINT timer_ret=1;

	m_dlg.KillTimer(nIDEvent);

	switch(nIDEvent)
	{
		case 6:	// 1 second
		{
			CheckPerformanceCounters();

//			m_search_manager.TimerHasFired();
			m_connection_manager.TimerHasFired();
			m_supply_manager.OnOneSecondTimer();

			if(m_process_manager.IsProcessing())
			{
				int demand=0;
				int supply=0;
				int dns=0;
				m_process_manager.GetProcessProgress(demand, supply/*, dns*/);
				m_supply_manager.m_supply_progress = supply;
				m_dlg.UpdateDataProcessProgressBar(demand, supply, dns);
			}
			if(m_process_manager.IsMaintaining())
			{
				int maintain=0;
				m_process_manager.GetMaintainProgress(maintain);
				m_dlg.UpdateDataMaintenanceProgressBar(maintain);
			}

			m_dlg.SetTimer(nIDEvent,TIMER_LENGTH*1000,0);
			break;
		}
		case 1:	// 1 minute heartbeat
		{
			//CTime current_time = CTime::GetCurrentTime();
			//int hour = current_time.GetHour();
			//int min = current_time.GetMinute();
			//if(hour == 1 && min == 0)
			//{
			//	m_process_manager.Reset();
			//}
			//if(!m_process_manager.IsAllDone())
			//	m_process_manager.DBMaintenance();
			m_dlg.SetTimer(nIDEvent,HEARTBEAT_INTERVAL*1000,0);
			break;
		}
		case 2:	// 30 second timer
		{
			//SendStatus();

			m_dlg.SetTimer(nIDEvent,BEASTIE_STATUS_INTERVAL*1000,0);
			break;
		}
/*		case 3:
		{
			m_search_manager.SearchTimerHasFired();
			
			// Make sure that there are any projects
			if(m_search_manager.TimerInterval()>0)
			{
				timer_ret=m_dlg.SetTimer(nIDEvent,m_search_manager.TimerInterval(),0);	// reset timer to 6 hours / # projects
			}

			break;
		}

		case 4:	// 1 minute timer
		{
			m_dist_manager.CheckBearshareDB();
			timer_ret=m_dlg.SetTimer(nIDEvent,BEARSHARE_DIST_DB_FILE_CHECK_INTERVAL*1000,0);
			break;
		}
*/		case 5: // 5 minute timer
		{
			CheckModuleConnections();

			m_dlg.SetTimer(nIDEvent,MODULE_CONNECTION_CHECK_INTERVAL*1000,0);
			break;
		}
		case 7: // 60 minute
		{
			m_connection_manager.OnHeartbeat();	// write out the host cache to file
			TKSyncherInterface tki;
			tki.RescanSource("GnutellaHostCache");
			m_dlg.SetTimer(nIDEvent,60*60*1000,0);
			break;
		}
	}
}

//
//
//
/*
void GnutellaSupplyDll::SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam)
{
	m_supply_manager.SupplyManagerThreadDataReady(wparam,lparam);
}

//
//
//
void GnutellaSupplyDll::InitLogfileManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	m_logfile_manager.InitThreadData(wparam,lparam);
}
*/

//
//
//
void GnutellaSupplyDll::InitDBManagerDemandThreadData(WPARAM wparam,LPARAM lparam)
{
	m_db_manager.InitDemandThreadData(wparam,lparam);
}

//
//
//
void GnutellaSupplyDll::InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
	m_db_manager.InitSupplyThreadData(wparam,lparam);
}

//
//
//
int GnutellaSupplyDll::AlterModuleCounts(int min,int max)
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

	// Limit both the min and max module counts to fall within 0 and 60
	if(m_min_module_count<0)
	{
		m_min_module_count=0;
	}
	else if(m_min_module_count>60)
	{
		m_min_module_count=60;
	}
	if(m_max_module_count<0)
	{
		m_max_module_count=0;
	}
	else if(m_max_module_count>60)
	{
		m_max_module_count=60;
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
int GnutellaSupplyDll::AlterSupplyIntervalMultiplier(int multi)
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
int GnutellaSupplyDll::GetSupplyIntervalMultiplier()
{
	return m_supply_interval_multi;
}

//
//
//
void GnutellaSupplyDll::ReadyToWriteDataToDemandDatabase(WPARAM wparam,LPARAM lparam)
{
	m_db_manager.ReadyToWriteDemandDataToDatabase(wparam,lparam);
}

//
//
//
void GnutellaSupplyDll::ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam)
{
	m_db_manager.ReadyToWriteSupplyDataToDatabase(wparam,lparam);
}

//
//
//
/*
void GnutellaSupplyDll::ReadyToWriteLogfileDataToFile(WPARAM wparam,LPARAM lparam)
{
	m_logfile_manager.ReadyToWriteLogfileDataToFile(wparam,lparam);
}
*/
//
//
//
/*
void GnutellaSupplyDll::ExtractProjectStatusReportData(ConnectionModuleStatusData &status)
{
	UINT i,j;

	for(i=0;i<status.v_project_status.size();i++)
	{
		ProjectStatusReportData *report_data=NULL;

		// Find the status object for this project. If I can't find it, then create a new one
		bool found=false;
		for(j=0;j<v_project_status_report_data.size();j++)
		{
			if(strcmp(v_project_status_report_data[j].m_project_name.c_str(),status.v_project_status[i].ProjectName())==0)
			{
				found=true;
				report_data=&v_project_status_report_data[j];
				break;
			}
		}

		// If not found, then add it, and set the pointer to it
		if(!found)
		{
			ProjectStatusReportData psrd;
			
			psrd.m_project_name=status.v_project_status[i].ProjectName();
			psrd.m_start_time=CTime::GetCurrentTime();

			v_project_status_report_data.push_back(psrd);
			
			report_data=&v_project_status_report_data[v_project_status_report_data.size()-1];
		}

		// Increment counters
		report_data->m_spoof_count+=status.v_project_status[i].m_spoof_count;
		report_data->m_dist_count+=status.v_project_status[i].m_dist_count;
		report_data->m_poison_count+=status.v_project_status[i].m_poison_count;
		report_data->IncrementTrackCounters(status.v_project_status[i]);
	}
}
*/

//
//
//
void GnutellaSupplyDll::InitPerformanceCounters()
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
    
	m_dlg.SetTimer(6,TIMER_LENGTH*1000,0);
}

//
//
//
void GnutellaSupplyDll::CheckPerformanceCounters()
{
	UINT i;

    // Collect the pdh query data
	if(PdhCollectQueryData(m_pdh))
	{
//		MessageBox(NULL,"Error collecting pdh query data","Error",MB_OK);
//		MessageBox(NULL,m_keynames[1].c_str(),"On this network adaptor",MB_OK);
		return;
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

	// If less than 50 % kick off another mod if there is less than min count of them
	if((m_proc_percent_usage<50) & (m_connection_manager.ReturnModCount()<(UINT)m_min_module_count))	// was 60
	{
		m_connection_manager.AddModule();

		char log[1024];
		sprintf(log,"Module Added : (Current Count : %u) : (Proc % Usage : %u%%) : (Min Mod Count : %u)\n",
			m_connection_manager.ReturnModCount(),m_proc_percent_usage,m_min_module_count);
		m_log_window_manager.Log(log,0x00888888);
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
	UINT queries_size, qh_size;
	m_db_manager.ReportDBStatus(queries_size, qh_size);
	sprintf(buf, "%u", queries_size);
	m_dlg.GetDlgItem(IDC_QUERIES_SIZE_STATIC)->SetWindowText(buf);
	sprintf(buf, "%u", qh_size);
	m_dlg.GetDlgItem(IDC_QH_SIZE_STATIC)->SetWindowText(buf);
}

//
//
//
void GnutellaSupplyDll::ClosePerformanceCounters()
{
	 PdhCloseQuery(m_pdh);
}

//
//
//
void GnutellaSupplyDll::SendStatus()
{
/*	int i;

	BeastieStatusData data;

	data.m_bandwidth=m_bandwidth_in_bytes;
	data.m_cpu_usage=m_proc_percent_usage;
	data.m_num_mods=m_connection_manager.ReturnModCount();
	data.m_mem_usage=m_mem_usage;

	data.m_run_time=m_running_since_time;

	for(i=0;i<m_keyword_manager.v_project_keywords.size();i++)
	{
		data.v_project_names.push_back(m_keyword_manager.v_project_keywords[i].m_project_name);
	}

	// Send the beastie status data
	unsigned int buf_len=sizeof(BeastieHeader)+data.GetBufferLength();
	char *buf=new char[buf_len];
	
	BeastieHeader *hdr=(BeastieHeader *)buf;
	hdr->op=BeastieHeader::BeastieStatus;
	hdr->size=data.WriteToBuffer(&buf[sizeof(BeastieHeader)]);

	if(DllInterface::IsUserOnline(m_master))
	{
		DllInterface::SendAppData(m_master,buf_len,buf);
	}

	delete [] buf;
*/
}

//
//
//
void GnutellaSupplyDll::CheckModuleConnections()
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
			
			m_log_window_manager.Log(log,0x008080FF);	// faded red
			
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
			sprintf(log,"Module Added : (Current Count : %u) : (Proc % Usage : %u%%) : (Min Mod Count : %u)\n",
				m_connection_manager.ReturnModCount(),m_proc_percent_usage,m_min_module_count);
			m_log_window_manager.Log(log,0x00888888);
		}
		else
		{
			// Log why we didn't add a module (not all of the modules were connected to at least 40 modules)
			char log[1024];
			m_log_window_manager.Log("Module ",0x00888888);		// grey
			m_log_window_manager.Log("NOT",0x000000FF);			// red
			m_log_window_manager.Log(" Added : ",0x00888888);	// grey
			sprintf(log,"%u / %u",not_enough_count,v_mod_status.size());
			m_log_window_manager.Log(log,0x008080FF,true);	// bold faded red
			m_log_window_manager.Log(" modules were not connected to at least ",0x00888888);	// grey
			m_log_window_manager.Log("40",0x00FF0000);	// blue
			m_log_window_manager.Log(" Ultrapeers\n",0x00888888);	// grey
		}
	}
}

//
//
//
void GnutellaSupplyDll::InitProcessManagerDemandThreadData(WPARAM wparam,LPARAM lparam)
{
	m_process_manager.InitDemandThreadData(wparam, lparam);

}


//
//
//
void GnutellaSupplyDll::ProcessManagerDemandDone(WPARAM wparam,LPARAM lparam)
{
//	m_process_manager.DemandProcessFinished();
}

//
//
//
void GnutellaSupplyDll::InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam)
{
	m_process_manager.InitSupplyThreadData(wparam, lparam);
}


//
//
//
void GnutellaSupplyDll::ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam)
{
//	m_process_manager.SupplyProcessFinished();
}

//
//
//
void GnutellaSupplyDll::InitProcessManagerReverseDNSThreadData(WPARAM wparam,LPARAM lparam)
{
	//m_process_manager.InitReverseDNSThreadData(wparam, lparam);
}


//
//
//
void GnutellaSupplyDll::ProcessManagerReverseDNSDone(WPARAM wparam,LPARAM lparam)
{
	//m_process_manager.ReverseDNSProcessFinished();
}

//
//
//
void GnutellaSupplyDll::StartDataProcess()
{
	//m_process_manager.StartProcessRawData(m_processed_db_info, m_keyword_manager.v_project_keywords);
}

//
//
//
/*
void GnutellaSupplyDll::InitDBManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam)
{
	m_process_manager.InitMaintenanceThreadData(wparam, lparam);
}
*/
//
//
//
void GnutellaSupplyDll::ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam)
{
	m_process_manager.MaintenanceFinished();
}

//
//
//
void GnutellaSupplyDll::DBMaintenanceReadyToStart(void)
{
	m_db_manager.DBMaintenanceReadyToStart();
}

//
//
//
void GnutellaSupplyDll::DBMaintenanceFinished(void)
{
	m_db_manager.DBMaintenanceFinished();
}

//
//
//
bool GnutellaSupplyDll::IsDBMaintaining()
{
	return m_process_manager.IsMaintaining();
}

//
//
//
void GnutellaSupplyDll::EnableVendorCounts(BOOL enabled)
{
	m_connection_manager.m_vendor_counts_enabled=(bool)enabled;
}

//
//
//
bool GnutellaSupplyDll::ReadyToSearch()
{
	UINT queries_size,qh_size;
	m_db_manager.ReportDBStatus(queries_size, qh_size);
	if( (queries_size >= 1000) || (qh_size >= 1000) )
		return false;
	else
		return true;
}

//
//
//
void GnutellaSupplyDll::StartDBMaintenance()
{
//#ifndef NO_PROCESSING
//	m_process_manager.DBMaintenance(m_keyword_manager.v_project_keywords);
//#endif
}

//
//
//
void GnutellaSupplyDll::ProjectIsReadyToBeProcessed(UINT project_id)
{
	m_process_manager.ProjectIsReadyToBeProcessed(project_id);
}