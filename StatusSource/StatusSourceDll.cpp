#include "StdAfx.h"
#include "statussourcedll.h"
#include "Iphlpapi.h"	// for GetAdaptersInfo (also need Iphlpapi.lib)
#include "Psapi.h"	// for GetProcessMemoryInfo()
#include <io.h>	// for _finddata_t stuff
#include "PDHMsg.h"	// for Pdh function error messages
#include <shldisp.h> //for system info
#include "StatusHeader.h"
#include "DllInterface.h"
#include "FileInfos.h"
#include "..\KazaaLauncher\KazaaLauncherInterface.h"
#include <atlbase.h>
#include <atlwin.h>

StatusSourceDll::StatusSourceDll(void)
{
}

//
//
//
StatusSourceDll::~StatusSourceDll(void)
{
}

//
//
//
void StatusSourceDll::DllInitialize()
{
	m_log_file.Open("StatusSourceLog.txt",CFile::modeCreate|CFile::modeWrite|CFile::typeText|CFile::shareDenyNone);

	m_log_file.WriteString("DllInitialize() begins\n");
	m_log_file.Flush();
	m_dlg.Create(IDD_STATUS_SOURCE_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_log_file.WriteString("DllInitialize() ends\n");
	m_log_file.Flush();
}

//
//
//
void StatusSourceDll::DllUnInitialize()
{
	ClosePerformanceCounters();
	m_log_file.Close();
}

//
//
//
void StatusSourceDll::DllStart()
{
	m_log_file.WriteString("DllStart() starts\n");
	m_log_file.Flush();
	//register com service
	m_com.Register(this,DllInterface::m_app_id.m_app_id);

	m_performance_counters_initialized=false;

	char text[1024];
	StatusData status;
	sprintf(text,"Status Source : Version %u",status.m_version);
	m_dlg.SetWindowText(text);

	SharedFileCount();

	m_dlg.SetTimer(1,60*1000,0);	// check # of shared files every minute

	if(InitPerformanceCounters())	// else there was no error, so initialize performance counters
	{
		m_performance_counters_initialized=true;
	}
	m_log_file.WriteString("DllStart() ends\n");
	m_log_file.Flush();

	StatusData s;
	CheckPerformanceCounters(s);

}

//
//
//
void StatusSourceDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void StatusSourceDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	StatusHeader* header = (StatusHeader*)data;
	byte* pData = (byte*)data;
	pData += sizeof(StatusHeader);
	switch(header->op)
	{
		case (StatusHeader::Status_Request):
		{
			byte* buf = (byte*)data;
			ReceivedStatusRequest(source_name, (UINT)*(&buf[sizeof(StatusHeader)]));
			break;
		}
		case (StatusHeader::Kill_All_Kazaa):
		{
			KazaaLauncherInterface kl_interface;
			kl_interface.KillAllKazaa();
			break;
		}
		case (StatusHeader::Launch_All_Kazaa):
		{
			KazaaLauncherInterface kl_interface;
			kl_interface.ResumeLaunchingKazaa();
			break;
		}
		case (StatusHeader::Restart_All_Kazaa):
		{
			KazaaLauncherInterface kl_interface;
			kl_interface.KillAllKazaa();
			kl_interface.ResumeLaunchingKazaa();
			break;
		}
		case (StatusHeader::Current_Number_Of_Kazaa_To_Run):
		{
			KazaaLauncherInterface kl_interface;
			UINT max_kazaa = *((UINT*)pData);
			kl_interface.SetMaxKazaaNumber(max_kazaa);
			break;
		}
		case (StatusHeader::Restart_Computer):
		{
			RestartComputer();
			break;
		}
		case (StatusHeader::System_Info_Request):
		{
			SystemInfoData sid;
			GetSystemInformation(sid);
			SendSystemInfoData(source_name, sid);
			break;
		}
		case (StatusHeader::Num_Kazaa_Uploads_Request):
		{
			ReceivedNumKazaaUploadsRequest(source_name);
			break;
		}
		case (StatusHeader::Request_Shared_Files):
		{
			ReceivedRequestSharedFiles(source_name);
			break;
		}

	}
}

//
//
//
bool StatusSourceDll::InitPerformanceCounters()
{
	m_log_file.WriteString("InitPerformanceCounters() begins\n");
	m_log_file.Flush();
	unsigned int i;

	char iai_buf[2048];
	DWORD iai_buf_len=2048;
	IP_ADAPTER_INFO *iai=(IP_ADAPTER_INFO *)iai_buf;

	//m_log_file.WriteString("calling GetAdaptersInfo(iai,&iai_buf_len)\n");
	//m_log_file.Flush();
	DWORD ret = GetAdaptersInfo(iai,&iai_buf_len);
	//m_log_file.WriteString("done calling GetAdaptersInfo(iai,&iai_buf_len)\n");
	//m_log_file.Flush();
	if(ret != ERROR_SUCCESS)
	{
		CString msg= "Error getting adapters info. Reason: ";
		if(ret == ERROR_BUFFER_OVERFLOW)
		{
			msg += "ERROR_BUFFER_OVERFLOW\t";
			msg.AppendFormat("%u bytes required\n",iai_buf_len);
		}
		else if(ret == ERROR_INVALID_PARAMETER)
		{
			msg += "ERROR_INVALID_PARAMETER\n";
		}
		else if(ret == ERROR_NO_DATA)
		{
			msg += "ERROR_NO_DATA\n";
		}
		else if(ret == ERROR_NOT_SUPPORTED)
		{
			msg += "ERROR_NOT_SUPPORTED\n";
		}
		else
		{
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			msg += (const char*)lpMsgBuf;
			msg += "\n";
			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
		m_log_file.WriteString(msg);
		m_log_file.Flush();
		return false;
	}
	CString log;
	log.AppendFormat("AddressLength: %d\n",iai->AddressLength);
	m_log_file.WriteString(log);
	m_log_file.Flush();

	//fill in the mac address
	for(i=0; i< iai->AddressLength;i++)
	{
		char temp[4];
		memset(&temp, 0, sizeof(temp));
		sprintf(temp, "%02X", iai->Address[i]);
		m_mac_address += temp;
		if(i!= iai->AddressLength-1)
			m_mac_address += ":";
	}
	m_log_file.WriteString("MAC address: ");
	m_log_file.WriteString(m_mac_address);
	m_log_file.WriteString("\n");
	m_log_file.Flush();

	
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
	m_log_file.WriteString("Network adaptor: ");
	m_log_file.WriteString(network_adaptor);
	m_log_file.WriteString("\n");
	m_log_file.Flush();


	m_keynames[0]="\\Processor(0)\\% Processor Time";
	
	m_keynames[1]="\\Network Interface(";
//	m_keynames[1]+=iai->Description;
	m_keynames[1]+=network_adaptor;
	m_keynames[1]+=")\\Bytes Total/sec";
	
	m_keynames[2]="\\Network Interface(";
//	m_keynames[2]+=iai->Description;
	m_keynames[2]+=network_adaptor;
	m_keynames[2]+=")\\Bytes Received/sec";

	m_keynames[3]="\\Network Interface(";
//	m_keynames[3]+=iai->Description;
	m_keynames[3]+=network_adaptor;
	m_keynames[3]+=")\\Bytes Sent/sec";

	m_keynames[4]="\\Memory\\Committed Bytes";


    m_pdh=0;
	//m_log_file.WriteString("calling PdhOpenQuery(NULL,0,&m_pdh)\n");
	//m_log_file.Flush();

	// Create the pdh query
    if(PdhOpenQuery(NULL,0,&m_pdh)!=ERROR_SUCCESS)
	{
		//::MessageBox(NULL,"Error opening pdh query","Error",MB_OK);
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText("Error opening pdh query");
		m_log_file.WriteString("Error opening pdh query\n");
		m_log_file.Flush();
		return false;
	}
	//m_log_file.WriteString("done calling PdhOpenQuery(NULL,0,&m_pdh)\n");
	//m_log_file.Flush();

	// ADD A COUNTER TO THE QUERY
	for(i=0;i<5;i++)
	{
		PDH_STATUS error=PdhAddCounter(m_pdh,m_keynames[i].c_str(),NULL,&m_pdh_counters[i]);

		if(error!=ERROR_SUCCESS)
		{
			char msg[1024];
			if(error==PDH_CSTATUS_BAD_COUNTERNAME)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_CSTATUS_BAD_COUNTERNAME",m_keynames[i].c_str());
			}
			else if(error==PDH_CSTATUS_NO_COUNTER)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_CSTATUS_NO_COUNTER",m_keynames[i].c_str());
			}
			else if(error==PDH_CSTATUS_NO_COUNTERNAME)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_CSTATUS_NO_COUNTERNAME",m_keynames[i].c_str());
			}
			else if(error==PDH_CSTATUS_NO_MACHINE)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_CSTATUS_NO_MACHINE",m_keynames[i].c_str());
			}
			else if(error==PDH_CSTATUS_NO_OBJECT)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_CSTATUS_NO_OBJECT",m_keynames[i].c_str());
			}
			else if(error==PDH_FUNCTION_NOT_FOUND)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_FUNCTION_NOT_FOUND",m_keynames[i].c_str());
			}
			else if(error==PDH_INVALID_ARGUMENT)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_INVALID_ARGUMENT",m_keynames[i].c_str());
			}
			else if(error==PDH_INVALID_HANDLE)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_INVALID_HANDLE",m_keynames[i].c_str());
			}
			else if(error==PDH_MEMORY_ALLOCATION_FAILURE)
			{
				sprintf(msg,"Error adding counter %s to the pdh query - PDH_MEMORY_ALLOCATION_FAILURE",m_keynames[i].c_str());
			}
			else
			{
				sprintf(msg,"Error adding counter %s to the pdh query - unknown reason",m_keynames[i].c_str());
			}

			//::MessageBox(NULL,msg,"Error",MB_OK);
			m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
			m_log_file.WriteString(msg);
			m_log_file.WriteString("\n");
			m_log_file.Flush();

//			return false;
		}
	}

	m_log_file.WriteString("InitPerformanceCounters() ends\n");
	m_log_file.Flush();
	return true;
}

//
//
//
bool StatusSourceDll::CheckPerformanceCounters(StatusData &status)
{
//	unsigned int i;
	//m_log_file.WriteString("CheckPerformanceCounters() begins\n");
	//m_log_file.Flush();

    // Collect the pdh query data
	//m_log_file.WriteString("calling PdhCollectQueryData(m_pdh)\n");
	//m_log_file.Flush();
	PDH_STATUS pdh_status=PdhCollectQueryData(m_pdh);
	//m_log_file.WriteString("done calling PdhCollectQueryData(m_pdh)\n");
	//m_log_file.Flush();

	if(pdh_status!=ERROR_SUCCESS)
	{
		if(pdh_status==PDH_INVALID_HANDLE)
		{
			m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText("Error collecting pdh query data - PDH_INVALID_HANDLE");
			//::MessageBox(NULL,"Error collecting pdh query data - PDH_INVALID_HANDLE","Error",MB_OK);
			m_log_file.WriteString("Error collecting pdh query data - PDH_INVALID_HANDLE\n");
			CString reason;
			reason.AppendFormat("Error code: %X\n",(DWORD)pdh_status);
			m_log_file.WriteString(reason);
			m_log_file.Flush();
			return false;
		}
		else if(pdh_status==PDH_NO_DATA)
		{
			m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText("Error collecting pdh query data - PDH_NO_DATA");
			//::MessageBox(NULL,"Error collecting pdh query data - PDH_NO_DATA","Error",MB_OK);
			m_log_file.WriteString("Error collecting pdh query data - PDH_NO_DATA\n");
			CString reason;
			reason.AppendFormat("Error code: %X\n",(DWORD)pdh_status);
			m_log_file.WriteString(reason);
			m_log_file.Flush();
			return false;
		}
		else
		{
			m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText("Error collecting pdh query data - unknown reason");
//			::MessageBox(NULL,"Error collecting pdh query data - unknown reason","Error",MB_OK);
			m_log_file.WriteString("Error collecting pdh query data - reason\n");
			CString reason;
			reason.AppendFormat("Error code: %X\n",(DWORD)pdh_status);
			m_log_file.WriteString(reason);
			m_log_file.Flush();
			return false;
		}

//		return false;
	}

	// Check the processor usage counter
	PDH_FMT_COUNTERVALUE value;
	PDH_STATUS error;

	//m_log_file.WriteString("calling PdhGetFormattedCounterValue(m_pdh_counters[0],PDH_FMT_LONG,0,&valu)\n");
	//m_log_file.Flush();
	error=PdhGetFormattedCounterValue(m_pdh_counters[0],PDH_FMT_LONG,0,&value);
	//m_log_file.WriteString("done calling PdhGetFormattedCounterValue(m_pdh_counters[0],PDH_FMT_LONG,0,&valu)\n");
	//m_log_file.Flush();
	if(error)
	{
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText("Error formatting pdh proc counter data");
		//MessageBox(NULL,"Error formatting pdh proc counter data","Error",MB_OK);
//		m_dlg.SetWindowText("Error formatting pdh coutner data");
		m_log_file.WriteString("Error formatting pdh proc counter data. ");
		CString reason;
		reason.AppendFormat("Error code: %X\n",(DWORD)error);
		m_log_file.WriteString(reason);
		m_log_file.Flush();
		return false;
		// Set this to the unknown value of 101%
		//status.m_percent_processor_usage=101;
	}
	else
	{
		status.m_percent_processor_usage=value.longValue;
	}

	// Check the total bandwidth counter
	//m_log_file.WriteString("calling PdhGetFormattedCounterValue(m_pdh_counters[1],PDH_FMT_LONG,0,&value)\n");
	//m_log_file.Flush();
	error=PdhGetFormattedCounterValue(m_pdh_counters[1],PDH_FMT_LONG,0,&value);
	//m_log_file.WriteString("done calling PdhGetFormattedCounterValue(m_pdh_counters[1],PDH_FMT_LONG,0,&value)\n");
	//m_log_file.Flush();
	if(error)
	{
		char msg[256];
		sprintf(msg, "Error formatting pdh counter total bw data. Error Code: %X\n", error);		
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
//		MessageBox(NULL,"Error formatting pdh counter bw data","Error",MB_OK);
//		m_dlg.SetWindowText("Error formatting pdh coutner data");
		m_log_file.WriteString(msg);
		m_log_file.Flush();
		return false;
	}
	else
	{
//	m_bandwidth_in_bytes=value.longValue;
		status.m_total_used_bandwidth=value.longValue*8;
	}

	// Check the download bandwidth in bits per second
	//m_log_file.WriteString("calling PdhGetFormattedCounterValue(m_pdh_counters[2],PDH_FMT_LONG,0,&value)\n");
	//m_log_file.Flush();
	error=PdhGetFormattedCounterValue(m_pdh_counters[2],PDH_FMT_LONG,0,&value);
	//m_log_file.WriteString("done calling PdhGetFormattedCounterValue(m_pdh_counters[2],PDH_FMT_LONG,0,&value)\n");
	//m_log_file.Flush();
	if(error)
	{
		char msg[256];
		sprintf(msg, "Error checking download bandwidth. Error Code: %X\n", error);		
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
		//MessageBox(NULL,"Error checking current bandwidth","Error",MB_OK);
		m_log_file.WriteString(msg);
		m_log_file.Flush();
		return false;
	}
	else
	{
		status.m_received_per_sec = value.longValue*8;
	}

	// Check the upload bandwidth in bits per second
	//m_log_file.WriteString("calling PdhGetFormattedCounterValue(m_pdh_counters[3],PDH_FMT_LONG,0,&value)\n");
	//m_log_file.Flush();
	error=PdhGetFormattedCounterValue(m_pdh_counters[3],PDH_FMT_LONG,0,&value);
	//m_log_file.WriteString("done calling PdhGetFormattedCounterValue(m_pdh_counters[3],PDH_FMT_LONG,0,&value)\n");
	//m_log_file.Flush();
	if(error)
	{
		char msg[256];
		sprintf(msg, "Error checking upload bandwidth. Error Code: %X\n", error);		
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
		//MessageBox(NULL,"Error checking current bandwidth","Error",MB_OK);
		m_log_file.WriteString(msg);
		m_log_file.Flush();
		return false;
	}
	else
	{
		status.m_sent_per_sec = value.longValue*8;
	}

	// Check the system memory usage
	//m_log_file.WriteString("calling PdhGetFormattedCounterValue(m_pdh_counters[4],PDH_FMT_LONG,0,&value)\n");
	//m_log_file.Flush();
	error=PdhGetFormattedCounterValue(m_pdh_counters[4],PDH_FMT_LONG,0,&value);
	//m_log_file.WriteString("done calling PdhGetFormattedCounterValue(m_pdh_counters[4],PDH_FMT_LONG,0,&value)\n");
	//m_log_file.Flush();
	if(error)
	{
		char msg[256];
		sprintf(msg, "Error checking system memory usage. Error Code: %X\n", error);		
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
		//MessageBox(NULL,"Error checking current bandwidth","Error",MB_OK);
		m_log_file.WriteString(msg);
		m_log_file.Flush();
		return false;
	}
	else
	{
		status.m_committed_memory = value.longValue;
	}

	// Measure the processes mem usage
	HANDLE hHandle;
	//m_log_file.WriteString("calling DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&hHandle,0,FALSE,DUPLICATE_SAME_ACCESS)\n");
	//m_log_file.Flush();
	DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&hHandle,0,FALSE,DUPLICATE_SAME_ACCESS);
	//m_log_file.WriteString("done calling DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&hHandle,0,FALSE,DUPLICATE_SAME_ACCESS)\n");
	//m_log_file.Flush();
	
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.WorkingSetSize=0;
	pmc.cb=sizeof(PROCESS_MEMORY_COUNTERS);
	//m_log_file.WriteString("calling GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS))\n");
	//m_log_file.Flush();
	GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS));
	//m_log_file.WriteString("done calling GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS))\n");
	//m_log_file.Flush();

	CloseHandle(hHandle);

	status.m_trapper_keeper_mem_usage = (UINT)pmc.WorkingSetSize;	// in bytes

	//m_log_file.WriteString("CheckPerformanceCounters() ends\n");
	//m_log_file.Flush();
	return true;
}

//
//
//
void StatusSourceDll::ClosePerformanceCounters()
{
	PDH_STATUS error=0;
	unsigned int i;

	for(i=0;i<3;i++)
	{
		error=PdhRemoveCounter(m_pdh_counters[i]);
	}

	 error=PdhCloseQuery(m_pdh);
}

//
//
//
void StatusSourceDll::ReceivedStatusRequest(char* source, UINT version)
{
	// If the performance counters have not been inited, try again
	if(m_performance_counters_initialized==false)
	{
		if(InitPerformanceCounters()==false)
		{
			return;
		}
	}

	unsigned int i;

	// Update the status on the GUI
	char msg[4096];
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"Received Status Request from %s on %04u-%02u-%02u at %02u:%02u:%02u\r\n",
		source,now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
	
	StatusData sd;
	if(sd.m_version>version)
	{	
		sprintf(&msg[strlen(msg)],"Remote Requester : OLDER Version (%u)\r\n",version);
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);	
	}
	else if(sd.m_version<version)
	{
		sprintf(&msg[strlen(msg)],"Remote Requester : NEWER Version (%u)\r\n",version);
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
	}
	else
	{
		sprintf(&msg[strlen(msg)],"Remote Requester : Current Version (%u)\r\n",version);
		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
	}

	// Gather the status
	StatusData reply_status;

	// Check the bandwidth and processor usage of the computer
	if(CheckPerformanceCounters(reply_status)==false)
	{
		return;		
	}

	// Get all of the memory usages for the KaZaA's running and figure out how many are running
//	unsigned int kazaa_count;
	//vector<unsigned int> kazaa_mem_usages;
	//FindKaZaAMemUsages(&kazaa_mem_usages);
	FindKaZaAMemUsages(&reply_status.v_kazaa_mems);

	//kazaa_count=(unsigned int)kazaa_mem_usages.size();
	reply_status.m_kazaa_count=(UINT)reply_status.v_kazaa_mems.size();

/*
	// Check to see if the required programs are running
	if(FindWindow(NULL,"TijitLauncher")!=NULL)
	{
		reply_status.m_is_tijit_launcher_running=true;
	}
	if(FindWindow(NULL,"KazaaSuperNodeRemover")!=NULL)
	{
		reply_status.m_is_supernode_remover_running=true;
	}
*/
	// Update the status on the GUI
	sprintf(&msg[strlen(msg)],"CPU Usage : %u %%\r\nBandwidth : %.2f kB/s\r\n",
		reply_status.m_percent_processor_usage,(((double)reply_status.m_total_used_bandwidth)/((double)1024))/((double)8));
	strcat(msg,"KaZaA Mem Usages : ");
	for(i=0;i<(int)reply_status.v_kazaa_mems.size();i++)
	{
		_itoa(reply_status.v_kazaa_mems[i]/(1024*1024),&msg[strlen(msg)],10);
		strcat(msg," MB");
		if(i<reply_status.v_kazaa_mems.size()-1)
		{
			strcat(msg,", ");
		}
	}
	strcat(msg,"\r\n");
/*	
	if(reply_status.m_is_tijit_launcher_running)
	{
		strcat(msg,"TijitLauncher : Running\r\n");
	}
	else
	{
		strcat(msg,"TijitLauncher : NOT Running\r\n");
	}
	if(reply_status.m_is_supernode_remover_running)
	{
		strcat(msg,"SuperNode Remover : Running\r\n");
	}
	else
	{
		strcat(msg,"SuperNode Remover : NOT Running\r\n");
	}
*/	
	reply_status.m_shared_file_count=m_shared_file_count;
	reply_status.m_movies_shared=m_movies_shared;
	reply_status.m_music_shared=m_music_shared;
	reply_status.m_swarmer_shared=m_swarmer_shared;

	sprintf(&msg[strlen(msg)],"Shared Files : %u\r\n",m_shared_file_count);

	m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);

	//Get the max kazaa number from the interface
	KazaaLauncherInterface kl_interface;
	reply_status.m_max_kazaa = (UINT)kl_interface.RequestKazaaNumber();

	ULARGE_INTEGER i64FreeBytesToCaller,i64TotalBytes,i64FreeBytes;
	//get harddrive info
	GetDiskFreeSpaceEx ("C:",
		(PULARGE_INTEGER)&i64FreeBytesToCaller,
        (PULARGE_INTEGER)&i64TotalBytes,
        (PULARGE_INTEGER)&i64FreeBytes);
	reply_status.m_free_bytes = (float)(i64FreeBytes.QuadPart / (float)(1024*1024*1024)); //free space in GB

	// Send back the reply to the source
	//unsigned int buf_len=sizeof(StatusHeader)+sizeof(StatusData)+sizeof(unsigned int)*reply_status.m_kazaa_count;
	unsigned int buf_len=sizeof(StatusHeader)+reply_status.GetBufferLength();
	char *buf=new char[buf_len];
	StatusHeader* header = (StatusHeader*)buf;
	header->op = StatusHeader::Status_Reply;
	header->size = buf_len - sizeof(StatusHeader);

	reply_status.WriteToBuffer(&buf[sizeof(StatusHeader)]);
	//memcpy(&buf[sizeof(StatusHeader)],&reply_status,sizeof(StatusData));
	/*
	unsigned int *ptr=(unsigned int *)&buf[sizeof(StatusHeader)+sizeof(StatusData)];
	for(i=0;i<reply_status.m_kazaa_count;i++)
	{
		*ptr=kazaa_mem_usages[i];
		ptr++;
	}
	*/
	m_com.SendUnreliableData(source,buf,buf_len);
	delete buf;
}

//
//
//
/*
BOOL CALLBACK EnumWindowProc(HWND hwnd,LPARAM lParam)
{
	vector<unsigned int> *kmu=(vector<unsigned int> *)lParam;

	char buf[1024+1];
	GetClassName(hwnd,buf,sizeof(buf)-1);

	// See if this is window's class name is KaZaA
	if(strcmp(buf,"KaZaA")==0)
	{
		// Get the process id for this window	
		DWORD process_id,thread_id;
		thread_id=GetWindowThreadProcessId(hwnd,&process_id);

		BOOL ret;
		HANDLE token_handle;
		ret=OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token_handle);

		LUID luid;
		ret=LookupPrivilegeValue(NULL,"SeDebugPrivilege",&luid);

		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount=1;
		tp.Privileges[0].Luid=luid;
		tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		ret=AdjustTokenPrivileges(token_handle,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);

		// Measure the processes mem usage
		HANDLE hHandle;
		hHandle=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,process_id);
		
		PROCESS_MEMORY_COUNTERS pmc;
		pmc.cb=sizeof(PROCESS_MEMORY_COUNTERS);
		GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS));
		
		CloseHandle(hHandle);
		CloseHandle(token_handle);

		kmu->push_back((unsigned int)pmc.WorkingSetSize);		// in bytes
	}
	
	return TRUE;	// continue the enumeration
}
*/
//
//
//
void StatusSourceDll::FindKaZaAMemUsages(vector<unsigned int> *kmu)
{
/*
	//default desktop
	EnumWindows((WNDENUMPROC)EnumWindowProc,(LPARAM)kmu);

	//Switch between the other 3 desktops and enumerate all Kazaa Windows
	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	SwitchDesktop(desk1);
	EnumDesktopWindows(desk1,(WNDENUMPROC)EnumWindowProc,(LPARAM)kmu);	
	
	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	SwitchDesktop(desk2);
	EnumDesktopWindows(desk2,(WNDENUMPROC)EnumWindowProc,(LPARAM)kmu);
	
	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	SwitchDesktop(desk3);
	EnumDesktopWindows(desk3,(WNDENUMPROC)EnumWindowProc,(LPARAM)kmu);

	//Switch back to the Default Desktop
	HDESK desk = OpenDesktop("Default",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	SwitchDesktop(desk);

	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);
	CloseDesktop(desk);
*/

	DWORD ids[1000];
	HMODULE modules[1000];
	DWORD count;
	EnumProcesses(ids,sizeof(DWORD)*1000,&count);
	count/=(sizeof(DWORD));
	for(int i=0;i<(int)count;i++)
	{ //for each of the processes
		DWORD nmod;
		HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
		EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
		nmod/=(sizeof(HMODULE));
		if(nmod>0)
		{
			char name[100];
			GetModuleBaseName(handle,modules[0],name,99);
			if( (stricmp("Kazaa.exe",name)==0) || (stricmp("Kazaa.kpp",name)==0) )//if process is named kazaa, add it to the kazaa process list
			{
				BOOL ret;
				HANDLE token_handle;
				ret=OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token_handle);

				LUID luid;
				ret=LookupPrivilegeValue(NULL,"SeDebugPrivilege",&luid);

				TOKEN_PRIVILEGES tp;
				tp.PrivilegeCount=1;
				tp.Privileges[0].Luid=luid;
				tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
				ret=AdjustTokenPrivileges(token_handle,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);

				// Measure the processes mem usage
				HANDLE hHandle;
				hHandle=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,ids[i]);
				
				PROCESS_MEMORY_COUNTERS pmc;
				pmc.cb=sizeof(PROCESS_MEMORY_COUNTERS);
				GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS));
				
				CloseHandle(hHandle);
				CloseHandle(token_handle);

				kmu->push_back((unsigned int)pmc.WorkingSetSize);		// in bytes
			}
		}
		CloseHandle(handle);
	}
}

//
//
//
void ExpandFolder(const char *folder,unsigned int &count)
{
	_finddata_t data;

	string str=folder;
	str+="\\*.*";

	intptr_t hFind=_findfirst(str.c_str(),&data);
	if(hFind==-1)
	{
		return;
	}

	// Check to see if it is a folder
	if(data.attrib & _A_SUBDIR)
	{
		// If it is not '.' or '..', expand it...recursion!
		if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
		{
			string new_str=folder;
			new_str+="\\";
			new_str+=data.name;
			ExpandFolder((char *)new_str.c_str(),count);
		}
	}
	else	// it is a file
	{
		count++;
	}

	while(_findnext(hFind,&data)!=-1)
	{
		// Check to see if it is a folder
		if(data.attrib & _A_SUBDIR)
		{
			// If it is not '.' or '..', expand it...recursion!
			if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
			{
				string new_str=folder;
				new_str+="\\";
				new_str+=data.name;
				ExpandFolder((char *)new_str.c_str(),count);
			}
		}
		else	// it is a file
		{
			count++;
		}
	}

	_findclose(hFind);
}

//
//
//
void StatusSourceDll::SharedFileCount()
{
	m_shared_file_count=m_movies_shared=m_music_shared=m_swarmer_shared=0;
	
	ExpandFolder("C:\\FastTrack Shared",m_shared_file_count);
	
	ExpandFolder("C:\\FastTrack Shared\\Movies", m_movies_shared);
	ExpandFolder("C:\\syncher\\rcv\\Media.Fucked", m_movies_shared);
	ExpandFolder("C:\\FastTrack Shared\\Music", m_music_shared);

	
	//find all swarmer's folders
	CString path;
	WIN32_FIND_DATA file_data;
	path+="C:\\FastTrack Shared\\*.*";
	HANDLE search_handle = ::FindFirstFile(path, &file_data);
	BOOL found = FALSE;
	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}
	vector<CString> swarmers;
	while(found == TRUE)
	{
		if(strcmp(file_data.cFileName, ".")!=0)
		{
			char filename[256+1];
			strcpy(filename, file_data.cFileName);

			if( (file_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
			{
				if(strstr(strlwr(file_data.cFileName), "swarmer")!=NULL)
				{
					CString folder = "C:\\FastTrack Shared\\";
					folder+=file_data.cFileName;
					swarmers.push_back(folder);
				}
			}
		}
		found = ::FindNextFile(search_handle, &file_data);
	}
	::FindClose(search_handle);

	for(UINT i=0; i<swarmers.size();i++)
		ExpandFolder(swarmers[i], m_swarmer_shared);	
}

//
//
//
void StatusSourceDll::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
		case 1:	// re-measure the number of shared files
		{
			SharedFileCount();
			break;
		}
	}
}

//
//
//
void StatusSourceDll::RestartComputer(void)
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 
	 
	// Get a token for this process. 
	 
	if (!OpenProcessToken(GetCurrentProcess(), 
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
		OutputDebugString("OpenProcessToken"); 
	 
	// Get the LUID for the shutdown privilege. 
	 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
			&tkp.Privileges[0].Luid); 
	 
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	 
	// Get the shutdown privilege for this process. 
	 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
			(PTOKEN_PRIVILEGES)NULL, 0); 
	 
	// Cannot test the return value of AdjustTokenPrivileges. 
	 
	if (GetLastError() != ERROR_SUCCESS) 
		OutputDebugString("AdjustTokenPrivileges"); 
	 
	// Shut down the system and force all applications to close. 
	 
	if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0)) 
		OutputDebugString("ExitWindowsEx"); 
	CloseHandle(hToken);
}

//
//
//
void StatusSourceDll::GetSystemInformation(SystemInfoData& sys_info)
{
	//get the memory info
	MEMORYSTATUS mem_info;
	::GlobalMemoryStatus(&mem_info);
	sys_info.m_total_physical_memory = (UINT)mem_info.dwTotalPhys;

	//get windows version
	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	//
	// If that fails, try using the OSVERSIONINFO structure.
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	}
	sys_info.b_OsVersionInfoEx = bOsVersionInfoEx;

	sys_info.m_os_info = osvi;

	ULARGE_INTEGER i64FreeBytesToCaller,i64TotalBytes,i64FreeBytes;
	//get harddrive info
	GetDiskFreeSpaceEx ("C:",
		(PULARGE_INTEGER)&i64FreeBytesToCaller,
        (PULARGE_INTEGER)&i64TotalBytes,
        (PULARGE_INTEGER)&i64FreeBytes);
	sys_info.m_total_harddisk_space = (float)(i64TotalBytes.QuadPart / (float)(1024*1024*1024)); //free space in GB

	//get CPU info
	CString cpu_info;
	GetCPUInfo(cpu_info);
	cpu_info.TrimLeft();
	cpu_info.TrimRight();
	if(cpu_info.GetLength() < sizeof(sys_info.m_cpu_info))
		strcpy(sys_info.m_cpu_info, cpu_info);

	//Get MAC address
	if(m_mac_address.GetLength() < sizeof(sys_info.m_mac_address))
		strcpy(sys_info.m_mac_address, m_mac_address);

	//Get computer name
	DWORD size = sizeof(sys_info.m_computer_name);
	::GetComputerName(sys_info.m_computer_name, &size);
}

//
//
//
void StatusSourceDll::GetCPUInfo(CString& cpu_info)
{
	TCHAR reg_sub_key[] = TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0");
	HKEY hkey;
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_sub_key, 0, KEY_QUERY_VALUE, &hkey);
	if( l == ERROR_SUCCESS )
	{
		int i= 0;

		while( true)
		{
			TCHAR szName[MAX_PATH];
			BYTE temp[MAX_PATH];
			::ZeroMemory(&szName, MAX_PATH);
			DWORD szName_size = sizeof(szName);
			DWORD data_size = MAX_PATH;

			l = RegEnumValue(hkey, i, szName, &szName_size, NULL, NULL,
				temp, &data_size);

			if(l == ERROR_NO_MORE_ITEMS)
				break;
		
			if(stricmp(szName, "ProcessorNameString") == 0)
			{
				cpu_info = temp;
				break;
			}
			i++;
		}
	}
	RegCloseKey(hkey);
}

//
//
//
void StatusSourceDll::SendSystemInfoData(char* dest, SystemInfoData& sys_info)
{
	// Send back the reply to the source
	unsigned int buf_len=sizeof(StatusHeader)+sizeof(SystemInfoData);
	char *buf=new char[buf_len];
	StatusHeader* header = (StatusHeader*)buf;
	header->op = StatusHeader::System_Info_Reply;
	header->size = buf_len - sizeof(StatusHeader);

	memcpy(&buf[sizeof(StatusHeader)],&sys_info,sizeof(SystemInfoData));
	m_com.SendReliableData(dest,buf,buf_len);
	delete buf;
}

//
//
//
unsigned int ReadMemory(void *ptr,char *buf,unsigned buf_len, HANDLE process_handle)
{
	DWORD num_read=0;
	unsigned int num_to_read=buf_len;
	while(ReadProcessMemory(process_handle,ptr,buf,num_to_read,&num_read)==FALSE)
	{
		if(GetLastError()!=ERROR_PARTIAL_COPY)	// error 299
		{
			break;
		}

		num_to_read--;
		if(num_to_read==0)
		{
			break;
		}
	}

	return (unsigned int)num_read;
}

//
//
// callback function for getting all kazaas' hwnds
BOOL CALLBACK EnumWindowProc(HWND hwnd,LPARAM lParam)
{
	EnumWindowData *enum_data =(EnumWindowData *)lParam;

	char buf[1024+1];
	GetClassName(hwnd,buf,sizeof(buf)-1);

	// See if this is window's class name is KaZaA
	if(strcmp(buf,"KaZaA")==0)
	{
		CWindow myWindow;
		myWindow.Attach(hwnd);
		DWORD process_id = myWindow.GetWindowProcessID();
		myWindow.Detach();

		HWND child_hwnd=::GetWindow(hwnd,GW_CHILD);
		while(child_hwnd!=NULL)
		{
			char buf[1024];
			memset(buf,0,sizeof(buf));
			GetClassName(child_hwnd,buf,sizeof(buf));
			if(strcmp(buf,"msctls_statusbar32")==0)
			{
				memset(buf,0,sizeof(buf));
				// Get the process handle of KaZaA
				HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS,TRUE,process_id);
				int err = GetLastError();
				// Allocate memory in the remote process's address space, and read the window item to see if it is connected
				void *ptr = VirtualAllocEx(process_handle,NULL,sizeof(buf),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

				// Call the sendmessagetimeout function so that if the kazaa is frozen, this won't hang.  Will wait 5 secs before timing out and returning 0.
				LRESULT res;
				if(::SendMessageTimeout(child_hwnd,SB_GETTEXT,(WPARAM)0,(LPARAM)ptr,SMTO_ABORTIFHUNG|SMTO_BLOCK,5000,(DWORD *)&res)==0)
				{
					// If the kazaa is hung or it has been 5 seconds, then free the memory...this kazaa is not connected.
					//Log("SENDMESSAGETIMEOUT - KaZaA::IsConnected() - KAZAA IS HUNG");
					// Free memory
					VirtualFreeEx(process_handle,ptr,0,MEM_RELEASE);
					// Close handle
					CloseHandle(process_handle);
					return true;
				}
				unsigned int num_read=ReadMemory(ptr,buf,256,process_handle);
				
				// Free memory
				VirtualFreeEx(process_handle,ptr,0,MEM_RELEASE);
				// Close handle
				CloseHandle(process_handle);
				
				// Make lowercase
				strlwr(buf);

				char* upload_ptr = strstr(buf,"uploads:");
				int uploads = 0;
				if( upload_ptr !=NULL )
				{
					upload_ptr+=9; // length of "Uploads: "
					sscanf(upload_ptr, "%d", &uploads);
					
					EnumWindowData* new_data = new EnumWindowData();
					strcpy(new_data->m_source, enum_data->m_source);
					new_data->m_uploads = uploads;
					::PostMessage(enum_data->m_hwnd, WM_KAZAA_HWND, (WPARAM)new_data, 0);
					return TRUE;
				}
				else
					child_hwnd=GetWindow(child_hwnd,GW_HWNDNEXT);
			}
			else
			{
				child_hwnd=GetWindow(child_hwnd,GW_HWNDNEXT);
			}
		}
	}	
	return TRUE;	// continue the enumeration
}

//
//
//
void StatusSourceDll::ReceivedNumKazaaUploadsRequest(char* from)
{
	EnumWindowData* dest_data = new EnumWindowData();
	strcpy(dest_data->m_source, from);
	dest_data->m_hwnd = m_dlg.GetSafeHwnd();

	EnumWindowData* dest1_data = new EnumWindowData();
	strcpy(dest1_data->m_source, from);
	dest1_data->m_hwnd = m_dlg.GetSafeHwnd();
	
	EnumWindowData* dest2_data = new EnumWindowData();
	strcpy(dest2_data->m_source, from);
	dest2_data->m_hwnd = m_dlg.GetSafeHwnd();
	
	EnumWindowData* dest3_data = new EnumWindowData();
	strcpy(dest3_data->m_source, from);
	dest3_data->m_hwnd = m_dlg.GetSafeHwnd();

	//Enum the default Desktop
	EnumWindows((WNDENUMPROC)EnumWindowProc,(LPARAM)dest_data);
	
	//Switch between the other 3 desktops and enumerate all Kazaa Windows
	HDESK desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk1,(WNDENUMPROC)EnumWindowProc,(LPARAM)dest1_data);	
	HDESK desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk2,(WNDENUMPROC)EnumWindowProc,(LPARAM)dest2_data);
	HDESK desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	EnumDesktopWindows(desk3,(WNDENUMPROC)EnumWindowProc,(LPARAM)dest3_data);

	//Close all Desktop Handles
	CloseDesktop(desk1);
	CloseDesktop(desk2);
	CloseDesktop(desk3);

	delete dest_data;
	delete dest1_data;
	delete dest2_data;
	delete dest3_data;
}

//
//
//
void StatusSourceDll::OnKazaaHwnd(WPARAM wparam,LPARAM lparam)
{
	EnumWindowData *enum_data =(EnumWindowData *)wparam;
	// Send back the reply to the source
	unsigned int buf_len=sizeof(StatusHeader)+sizeof(int);
	char *buf=new char[buf_len];
	StatusHeader* header = (StatusHeader*)buf;
	header->op = StatusHeader::Num_Kazaa_Uploads;
	header->size = buf_len - sizeof(StatusHeader);
				
	int* num_upload = (int*)&buf[sizeof(StatusHeader)];
	*num_upload = enum_data->m_uploads;
	m_com.SendReliableData(enum_data->m_source,buf,buf_len);
	delete buf;
	delete enum_data;
}

//
//
//
void StatusSourceDll::ReceivedRequestSharedFiles(char* source_name)
{
	vector<FileInfo> files;
	ExploreFolder("C:\\Fasttrack Shared\\", files);
	ExploreFolder("C:\\syncher\\rcv\\Media.Fucked\\", files);

	FileInfos file_infos;
	file_infos.v_files = files;
	
	//Send the results to the source
	unsigned int buf_len = sizeof(StatusHeader) + file_infos.GetBufferLength();
	char *buf=new char[buf_len];
	StatusHeader* header = (StatusHeader*)buf;
	header->op = StatusHeader::Shared_Files_Reply;
	header->size = buf_len - sizeof(StatusHeader);
	file_infos.WriteToBuffer(&buf[sizeof(StatusHeader)]);
	m_com.SendReliableData(source_name, buf, buf_len);
	delete [] buf;
}

//
//
//
void StatusSourceDll::ExploreFolder(char *folder, vector<FileInfo> &files)
{
	char buf[4096];
	strcpy(buf,folder);
	strcat(buf,"*.*");
	
	_finddata_t data;
	long handle=(long)_findfirst(buf,&data);

	if(handle==-1)
	{
		return;
	}

	if(data.attrib & _A_SUBDIR)
	{
		if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
		{
			strcpy(buf,folder);
			strcat(buf,data.name);
			strcat(buf,"\\");
			ExploreFolder(buf,files);
		}
	}
	else
	{
		FileInfo file_data;
		file_data.m_path = folder;
		file_data.m_filename = data.name;
		file_data.m_file_size = data.size;
		file_data.m_creation_date = CTime(data.time_create);
		files.push_back(file_data);
	}

	while(_findnext(handle,&data)==0)
	{
		if(data.attrib & _A_SUBDIR)
		{
			if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
			{
				strcpy(buf,folder);
				strcat(buf,data.name);
				strcat(buf,"\\");
				ExploreFolder(buf,files);
			}
		}
		else
		{
			FileInfo file_data;
			file_data.m_path = folder;
			file_data.m_filename = data.name;
			file_data.m_file_size = data.size;
			file_data.m_creation_date = CTime(data.time_create);
			files.push_back(file_data);
		}
	}
	_findclose(handle);
}
