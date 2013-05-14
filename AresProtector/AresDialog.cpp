// AresDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AresDialog.h"
#include "AresProtectionSystem.h"
#include <shldisp.h> //for system info
#include <Iphlpapi.h>
#include <PDHMsg.h>
#include <Psapi.h>
#include "..\tkcom\linkedlist.h"
#include ".\aresdialog.h"
#include "AresFile.h"

// CAresDialog dialog

IMPLEMENT_DYNAMIC(CAresDialog, CDialog)
CAresDialog::CAresDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAresDialog::IDD, pParent)
{
}

CAresDialog::~CAresDialog()
{
}

void CAresDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ULTRAPEERS, m_ultra_peers);
	DDX_Control(pDX, IDC_KNOWNULTRAPEERS, m_known_ultra_peers);
	DDX_Control(pDX, IDC_CONNECTEDULTRAPEERS, m_connected_ultra_peers);
	DDX_Control(pDX, IDC_SYSTEMSTATUS, m_system_status);
	DDX_Control(pDX, IDC_TOTALCPU, m_total_cpu);
	DDX_Control(pDX, IDC_ARESCPU, m_ares_cpu);
	DDX_Control(pDX, IDC_RECEIVEBANDWIDTH, m_receive_bandwidth);
	DDX_Control(pDX, IDC_SENDBANDWIDTH, m_send_bandwidth);
	DDX_Control(pDX, IDC_SYSTEMMEMORYUSAGE, m_system_memory);
	DDX_Control(pDX, IDC_TRAPPERKEEPERMEMORYUSAGE, m_tk_memory);
	DDX_Control(pDX, IDC_DECOY_UPLOADS, m_peers);
	DDX_Control(pDX, IDC_EVENTTIMESLIDER, m_event_time_slider);
	DDX_Control(pDX, IDC_EVENTTIMETEXT, m_event_time);
	DDX_Control(pDX, IDC_RECORDEVENTS, m_record_events);
	DDX_Control(pDX, IDC_EVENTLIST, m_event_list);
	DDX_Control(pDX, IDC_ULTRAPEERSWITHFILES, m_ultra_peers_with_files);
	DDX_Control(pDX, IDC_SPOOFSLOADED, m_spoofs_loaded);
	DDX_Control(pDX, IDC_HEXVIEW, m_hex_view);
	DDX_Control(pDX, IDC_FILETRANSFERPEERCOUNT, m_file_transfer_peer_count);
	DDX_Control(pDX, IDC_SENDQUEUESIZE, m_send_queue_size);
	DDX_Control(pDX, IDC_INTERDICTIONTARGETCOUNT, m_interdiction_target_count);
	DDX_Control(pDX, IDC_INTERDICTIONTRANSFER, m_interdiction_transfer_amount);
	DDX_Control(pDX, IDC_INTERDICTIONCONNECTIONCOUNT, m_interdiction_connection_count);
	DDX_Control(pDX, IDC_INTERDICTIONCONNECTIONTRANSFERCOUNT, m_interdiction_connection_transfer_count);
	DDX_Control(pDX, IDC_INTERDICTIONBANNEDCLIENTRATIO, m_interdiction_banned_client_ratio);
	DDX_Control(pDX, IDC_INTERDICTIONQUEUESIZE, m_interdiction_queue_size);
	DDX_Control(pDX, IDC_HASH_CACHE_EFFICIENCY, m_hash_cache_efficiency);
}


BEGIN_MESSAGE_MAP(CAresDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RECORDEVENTS, OnBnClickedRecordEvents)
	ON_NOTIFY(NM_CLICK, IDC_ULTRAPEERS, OnClickUltrapeers)
	ON_NOTIFY(LVN_KEYDOWN, IDC_ULTRAPEERS, OnLvnKeydownUltrapeers)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_PURGEDEADCONNECTIONSNOW, OnBnClickedPurgedeadconnectionsnow)
END_MESSAGE_MAP()


// CAresDialog message handlers

BOOL CAresDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
//	AfxSocketInit();
	this->SetTimer(1,5000,NULL);  //every X seconds fire gui update timer
	this->SetTimer(2,2000,NULL);  //every X seconds fire performance counter timer
	this->SetTimer(3,1000,NULL);  //every X seconds fire system update timer
	this->SetTimer(4,500,NULL);  //every X seconds fire system update timer
	this->SetTimer(5,33,NULL);  //every X seconds fire tcp update timer

	m_ultra_peers.InsertColumn(0,"IP:Port",LVCFMT_LEFT,140);
	m_ultra_peers.InsertColumn(1,"Age (DD:HH:MM:SS)",LVCFMT_LEFT,140);
	m_ultra_peers.InsertColumn(2,"Network Users",LVCFMT_LEFT,113);
	m_ultra_peers.InsertColumn(3,"Network Files",LVCFMT_LEFT,113);
	m_ultra_peers.InsertColumn(4,"Network MBs",LVCFMT_LEFT,113);
	m_ultra_peers.InsertColumn(5,"Status",LVCFMT_LEFT,450);

	m_peers.InsertColumn(0,"IP:Port",LVCFMT_LEFT,140);
	m_peers.InsertColumn(1,"Age (DD:HH:MM:SS)",LVCFMT_LEFT,140);
	m_peers.InsertColumn(2,"Status",LVCFMT_LEFT,450);

	m_event_list.InsertColumn(0,"Event Type",LVCFMT_LEFT,128);
	m_event_list.InsertColumn(1,"Time",LVCFMT_LEFT,128);
	m_event_list.InsertColumn(2,"Code",LVCFMT_LEFT,128);
	m_event_list.InsertColumn(3,"Message Length",LVCFMT_LEFT,128);

	m_event_list.SetExtendedStyle(m_event_list.GetExtendedStyle()|LVS_EX_FULLROWSELECT);
	m_peers.SetExtendedStyle(m_peers.GetExtendedStyle()|LVS_EX_FULLROWSELECT);
	m_ultra_peers.SetExtendedStyle(m_ultra_peers.GetExtendedStyle()|LVS_EX_FULLROWSELECT);
	
	

	//m_ultra_peers.InsertColumn(0,"IP:Port",LVCFMT_LEFT,100);
	//m_ultra_peers.InsertColumn(1,"Age (DD:HH:MM:SS)",LVCFMT_LEFT,100);
	//m_ultra_peers.InsertColumn(2,"Status",LVCFMT_LEFT,100);
	

	m_record_events.SetCheck(1);
	m_event_time.SetWindowText("1");
	m_event_time_slider.SetRange(1,60,TRUE);
	m_event_time_slider.SetPos(1);
	
	InitPerformanceCounters();

	this->SetWindowPos(NULL,0,0,1425,885,0/*SWP_SHOWWINDOW*/);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAresDialog::OnDestroy()
{
	CDialog::OnDestroy();

	this->KillTimer(1);
	this->KillTimer(2);
	this->KillTimer(3);
	this->KillTimer(4);
	ClosePerformanceCounters();
}

void CAresDialog::OnTimer(UINT nIDEvent)
{
	static CCriticalSection slock;

	CSingleLock lock(&slock,TRUE);
	if(nIDEvent==1){
		UpdateGUI();
	}
	else if(nIDEvent==2){
		CheckPerformanceCounters();
	}
	else if(nIDEvent==3){
		AresProtectionSystemReference ref;
		ref.System()->Update();
		ref.System()->UpdateInterdictionConnections();
	}
	else if(nIDEvent==4){
		AresProtectionSystemReference ref;
		ref.System()->UpdateFileTransferConnections();
	}
	else if(nIDEvent==5){
		AresProtectionSystemReference ref;
		ref.System()->UpdateTCP();
	}
	CDialog::OnTimer(nIDEvent);
}

void CAresDialog::UpdateGUI(void)
{
	
	AresProtectionSystemReference ref;

	//ENTER CRITICAL SECTION
//	CSingleLock lock(&ref.System()->m_tcp_lock,TRUE);  

	Vector* v_hosts=ref.System()->GetConnectedHosts();

	CString tmp_str;

	int share_files_count=0;

	static Timer host_update;

	if(host_update.HasTimedOut(15)){
		host_update.Refresh();

		int index=0;
		for(int bin_index=0;bin_index<NUMCONBINS;bin_index++){
			for(int j=0;j<(int)v_hosts[bin_index].Size();j++){
				AresConnection *ac=(AresConnection*)v_hosts[bin_index].Get(j);

				if(ac->IsSharingFiles())
					share_files_count++;

				tmp_str.Format("%s:%d",ac->GetIP(),ac->GetPort());
				if(m_ultra_peers.GetItemCount()<index+1)
					m_ultra_peers.InsertItem(index,tmp_str);
				m_ultra_peers.SetItemText(index,0,tmp_str);
				//CTimeSpan ts=ac->GetAliveTime();
				//tmp_str.Format("%.2d:%.2d:%.2d:%.2d",ts.GetDays(),ts.GetHours(),ts.GetMinutes(),ts.GetSeconds());

				CString age_string=ac->GetFormattedAgeString();
				m_ultra_peers.SetItemText(index,1,age_string);
				tmp_str.Format("%u",ac->GetNetworkUsers());
				m_ultra_peers.SetItemText(index,2,tmp_str);
				tmp_str.Format("%u",ac->GetNetworkFiles());
				m_ultra_peers.SetItemText(index,3,tmp_str);
				tmp_str.Format("%u",ac->GetNetworkMBs());
				m_ultra_peers.SetItemText(index,4,tmp_str);
				m_ultra_peers.SetItemText(index,5,ac->GetStatus());
				m_ultra_peers.SetItemData(index,ac->GetID());
				index++;
			}
		}

		while(m_ultra_peers.GetItemCount()>index)
			m_ultra_peers.DeleteItem(m_ultra_peers.GetItemCount()-1);

		tmp_str.Format("%d",index);
		m_connected_ultra_peers.SetWindowText(tmp_str);

		tmp_str.Format("%d",share_files_count);
		m_ultra_peers_with_files.SetWindowText(tmp_str);
	}

	tmp_str.Format("%d",ref.System()->GetHostCache()->Size());
	m_known_ultra_peers.SetWindowText(tmp_str);

	tmp_str.Format("%u",ref.System()->GetSpoofsLoaded());
	m_spoofs_loaded.SetWindowText(tmp_str);

	m_system_status.SetWindowText(ref.System()->GetStatus());

	Vector* ft_connections=ref.System()->GetFTConnections();
	tmp_str.Format("%u clients",ft_connections->Size());
	m_file_transfer_peer_count.SetWindowText(tmp_str);

	//update ft client window

	static Timer ft_update;

	if(ft_update.HasTimedOut(15)){
		ft_update.Refresh();
		int index=0;

		for(int bin_index=0;bin_index<NUMCONBINS;bin_index++){
			for(int j=0;j<(int)ft_connections[bin_index].Size();j++){
				AresFTConnection *ac=(AresFTConnection*)ft_connections[bin_index].Get(j);

				tmp_str.Format("%s:%d",ac->GetIP(),ac->GetPort());
				if(m_peers.GetItemCount()<index+1)
					m_peers.InsertItem(index,tmp_str);
				m_peers.SetItemText(index,0,tmp_str);
				//CTimeSpan ts=ac->GetAliveTime();
				//tmp_str.Format("%.2d:%.2d:%.2d:%.2d",ts.GetDays(),ts.GetHours(),ts.GetMinutes(),ts.GetSeconds());

				m_peers.SetItemText(index,1,ac->GetFormattedAgeString());
				m_peers.SetItemText(index,2,ac->GetStatus());
				m_peers.SetItemData(index,ac->GetID());
				index++;
			}
		}

		while(m_peers.GetItemCount()>index)
			m_peers.DeleteItem(m_peers.GetItemCount()-1);
	}


	tmp_str.Format("%u bytes/sec",ref.System()->GetFTDataPerSecond());
	m_send_queue_size.SetWindowText(tmp_str);


	tmp_str.Format("%.2f %%",ref.System()->GetBannedClientRatio()*100);
	m_interdiction_banned_client_ratio.SetWindowText(tmp_str);

	tmp_str.Format("%u",ref.System()->GetInterdictionConnectionCount());
	m_interdiction_connection_count.SetWindowText(tmp_str);

	tmp_str.Format("%u",ref.System()->GetInterdictionTargetCount());
	m_interdiction_target_count.SetWindowText(tmp_str);

	tmp_str.Format("%u",ref.System()->GetInterdictionConnectionTransferredCount());
	m_interdiction_connection_transfer_count.SetWindowText(tmp_str);

	tmp_str.Format("%u",ref.System()->GetInterdictionTransferAmount());
	m_interdiction_transfer_amount.SetWindowText(tmp_str);

	tmp_str.Format("%u",ref.System()->GetInterdictionQueueSize());
	m_interdiction_queue_size.SetWindowText(tmp_str);

	tmp_str.Format("%.2f",ref.System()->GetHashCacheEfficiency());
	m_hash_cache_efficiency.SetWindowText(tmp_str);

	CheckPerformanceCounters();
}

bool CAresDialog::CheckPerformanceCounters(void)
{
    // Collect the pdh query data
	//TRACE("CAresDialog::CheckPerformanceCounters(void) BEGIN\n");
	PDH_STATUS pdh_status=PdhCollectQueryData(m_pdh);

	if(pdh_status!=ERROR_SUCCESS)
	{
		if(pdh_status==PDH_INVALID_HANDLE)
		{
			//m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText("Error collecting pdh query data - PDH_INVALID_HANDLE");
			//m_log_file.WriteString("Error collecting pdh query data - PDH_INVALID_HANDLE\n");
			//CString reason;
			//reason.AppendFormat("Error code: %X\n",(DWORD)pdh_status);
			//m_log_file.WriteString(reason);
			//m_log_file.Flush();
			TRACE("CAresDialog::CheckPerformanceCounters(void) Error pdh_status==PDH_INVALID_HANDLE\n");
		}
		else if(pdh_status==PDH_NO_DATA)
		{
			TRACE("CAresDialog::CheckPerformanceCounters(void) Error pdh_status==PDH_NO_DATA\n");
		}
		else
		{
			TRACE("CAresDialog::CheckPerformanceCounters(void) Error pdh_status==unknown error\n");
		}
	}

	// Check the processor usage counter
	PDH_FMT_COUNTERVALUE value;
	PDH_STATUS error;

	error=PdhGetFormattedCounterValue(m_pdh_counters[0],PDH_FMT_LONG,0,&value);
	if(error)
	{
		TRACE("CAresDialog::CheckPerformanceCounters(void) error formating processor data\n",value.longValue);
		//m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText("Error formatting pdh proc counter data");
		//m_log_file.WriteString("Error formatting pdh proc counter data. ");
		//CString reason;
		//reason.AppendFormat("Error code: %X\n",(DWORD)error);
		//m_log_file.WriteString(reason);
		//m_log_file.Flush();
	}
	else
	{
		//TRACE("CAresDialog::CheckPerformanceCounters(void) Processor usage is %u\n",value.longValue);
		CString tmp;
		tmp.Format("%u %%",value.longValue);
		m_total_cpu.SetWindowText(tmp);
		//status.m_percent_processor_usage=value.longValue;
	}

	// Check the total bandwidth counter
	error=PdhGetFormattedCounterValue(m_pdh_counters[1],PDH_FMT_LONG,0,&value);
	if(error)
	{
		TRACE("CAresDialog::CheckPerformanceCounters(void) error formating total bandwidth data.   ERROR CODE: %X\n",error);
		//char msg[256];
		//sprintf(msg, "Error formatting pdh counter total bw data. Error Code: %X\n", error);		
		//m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
		//m_log_file.WriteString(msg);
		//m_log_file.Flush();
	}
	else
	{
		//TRACE("CAresDialog::CheckPerformanceCounters(void) total bandwidth= %u\n",value.longValue);
		//status.m_total_used_bandwidth=value.longValue*8;
	}

	// Check the download bandwidth in bits per second
	error=PdhGetFormattedCounterValue(m_pdh_counters[2],PDH_FMT_LONG,0,&value);

	if(error)
	{
		TRACE("CAresDialog::CheckPerformanceCounters(void) error formating receive bandwidth data.   ERROR CODE: %X\n",error);
		//char msg[256];
		//sprintf(msg, "Error checking download bandwidth. Error Code: %X\n", error);		
		//m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
		//MessageBox(NULL,"Error checking current bandwidth","Error",MB_OK);
		//m_log_file.WriteString(msg);
		//m_log_file.Flush();
		//return false;
	}
	else
	{
		//TRACE("CAresDialog::CheckPerformanceCounters(void) receive bandwidth= %u\n",value.longValue);
		CString tmp;
		tmp.Format("%.02f bytes/sec",(float)((double)value.longValue/8.0));
		m_receive_bandwidth.SetWindowText(tmp);
		//status.m_received_per_sec = value.longValue*8;
	}

	// Check the upload bandwidth in bits per second
	error=PdhGetFormattedCounterValue(m_pdh_counters[3],PDH_FMT_LONG,0,&value);
	if(error)
	{
		TRACE("CAresDialog::CheckPerformanceCounters(void) error formating send bandwidth data.   ERROR CODE: %X\n",error);
		//char msg[256];
		//sprintf(msg, "Error checking upload bandwidth. Error Code: %X\n", error);		
		//m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
		//MessageBox(NULL,"Error checking current bandwidth","Error",MB_OK);
		//m_log_file.WriteString(msg);
		//m_log_file.Flush();
		//return false;
	}
	else
	{
		//TRACE("CAresDialog::CheckPerformanceCounters(void) send bandwidth= %u\n",value.longValue);

		CString tmp;
		tmp.Format("%.02f bytes/sec",(float)((double)value.longValue/8.0));
		m_send_bandwidth.SetWindowText(tmp);
		//status.m_sent_per_sec = value.longValue*8;
	}

	// Check the system memory usage
	error=PdhGetFormattedCounterValue(m_pdh_counters[4],PDH_FMT_LONG,0,&value);

	if(error)
	{
		TRACE("CAresDialog::CheckPerformanceCounters(void) error formating total memory data.   ERROR CODE: %X\n",error);
		//char msg[256];
		//sprintf(msg, "Error checking system memory usage. Error Code: %X\n", error);		
		//m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText(msg);
		//MessageBox(NULL,"Error checking current bandwidth","Error",MB_OK);
		//m_log_file.WriteString(msg);
		//m_log_file.Flush();
		//return false;
	}
	else
	{
		//TRACE("CAresDialog::CheckPerformanceCounters(void) total memory usage= %u\n",value.longValue);

		CString tmp2=FormatNumberForDisplay(value.longValue);
		CString tmp;
		tmp.Format("%s bytes",tmp2);
		m_system_memory.SetWindowText(tmp);
		//status.m_committed_memory = value.longValue;
	}

	// Measure the processes mem usage
	
	HANDLE hHandle;
	DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&hHandle,0,FALSE,DUPLICATE_SAME_ACCESS);
	
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.WorkingSetSize=0;
	pmc.cb=sizeof(PROCESS_MEMORY_COUNTERS);
	GetProcessMemoryInfo(hHandle,&pmc,sizeof(PROCESS_MEMORY_COUNTERS));

	CloseHandle(hHandle);

	CString tmp2=FormatNumberForDisplay((UINT)pmc.WorkingSetSize);
	CString tmp;
	tmp.Format("%s bytes",tmp2);
	m_tk_memory.SetWindowText(tmp);
	//status.m_trapper_keeper_mem_usage = (UINT)pmc.WorkingSetSize;	// in bytes
	

	//TRACE("CAresDialog::CheckPerformanceCounters(void) END\n");
	return true;
}

bool CAresDialog::InitPerformanceCounters(void)
{
	TRACE("CAresDialog::InitPerformanceCounters(void) BEGIN\n");
	unsigned int i;

	char iai_buf[2048];
	DWORD iai_buf_len=2048;
	IP_ADAPTER_INFO *iai=(IP_ADAPTER_INFO *)iai_buf;

	DWORD ret = GetAdaptersInfo(iai,&iai_buf_len);

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
		
		TRACE(msg);
		TRACE("CAresDialog::InitPerformanceCounters(void) END ABNORMAL\n");
		return false;
	}

	TRACE("CAresDialog::InitPerformanceCounters(void) AddressLength: %d\n",iai->AddressLength);

	//fill in the mac address
	CString mac_address;
	for(i=0; i< iai->AddressLength;i++)
	{
		char temp[4];
		memset(&temp, 0, sizeof(temp));
		sprintf(temp, "%02X", iai->Address[i]);
		mac_address += temp;
		if(i!= iai->AddressLength-1)
			mac_address += ":";
	}

	TRACE("CAresDialog::InitPerformanceCounters(void) MAC address: %s\n",mac_address);

	
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
	TRACE("CAresDialog::InitPerformanceCounters(void) Network Adapter: %s\n",network_adaptor);

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
		TRACE("CAresDialog::InitPerformanceCounters(void) Error opening pdh query: %s\n");
		//::MessageBox(NULL,"Error opening pdh query","Error",MB_OK);
//		m_dlg.GetDlgItem(IDC_Status_Static)->SetWindowText("Error opening pdh query");
		//m_log_file.WriteString("Error opening pdh query\n");
//		m_log_file.Flush();
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

			TRACE("CAresDialog::InitPerformanceCounters(void) Error setting up query: %s\n",msg);
		}
	}

	TRACE("CAresDialog::InitPerformanceCounters(void) END - NORMAL\n");
//	m_log_file.WriteString("InitPerformanceCounters() ends\n");
	//m_log_file.Flush();
	return true;
}

void CAresDialog::ClosePerformanceCounters(void)
{
	PDH_STATUS error=0;
	unsigned int i;

	for(i=0;i<3;i++)
	{
		error=PdhRemoveCounter(m_pdh_counters[i]);
	}

	 error=PdhCloseQuery(m_pdh);
}

CString CAresDialog::FormatNumberForDisplay(UINT number)
{
	CString val;
	val.Format("%u",number);

	for(int i=val.GetLength()-1;i>=0;i-=3){
		if(i!=val.GetLength()-1)
			val.Insert(i+1,",");
	}

	return val;
}

void CAresDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if((CSliderCtrl*)pScrollBar==&m_event_time_slider){
		CString tmp;
		int event_cache_time=m_event_time_slider.GetPos();
		if(!m_record_events.GetCheck())
			event_cache_time=0;
		tmp.Format("%d",event_cache_time);
		m_event_time.SetWindowText(tmp);

		AresProtectionSystemReference ref;
		ref.System()->SetEventCacheTime(event_cache_time);
		ref.System()->ResetConnectionPurgeTimer();
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CAresDialog::OnBnClickedRecordEvents()
{
	AresProtectionSystemReference ref;
	if(m_record_events.GetCheck()){
		ref.System()->SetEventCacheTime(m_event_time_slider.GetPos());
		CString tmp;
		tmp.Format("%d",m_event_time_slider.GetPos());
		m_event_time.SetWindowText(tmp);
	}
	else{
		m_event_time.SetWindowText("0");
		ref.System()->SetEventCacheTime(0);
	}
}

void CAresDialog::OnClickUltrapeers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	
	OnUltrapeerSelectionChange();
	*pResult = 0;
}

void CAresDialog::OnUltrapeerSelectionChange(void)
{
	AresProtectionSystemReference ref;
	ref.System()->ResetConnectionPurgeTimer();

	POSITION pos = m_ultra_peers.GetFirstSelectedItemPosition();
	if (pos == NULL)
		TRACE0("No items were selected!\n");

	int nItem = m_ultra_peers.GetNextSelectedItem(pos);
	TRACE1("Ultrapeer %d was selected!\n", nItem);
	
	if(nItem==-1)
		return;

	UINT host_handle=(UINT)m_ultra_peers.GetItemData(nItem);
	
	//CSingleLock lock(&ref.System()->m_tcp_lock,TRUE);  

	m_event_list.DeleteAllItems();

	AresConnection *ac=ref.System()->GetHost(host_handle);
	
	if(!ac)
		return;

	mv_selected_peer.Add(ac);

	LinkedList *events=ac->GetEventList();

	CSingleLock lock2(&events->m_list_lock,TRUE);

	events->StartIteration();
	int count=0;
	while(true){
		AresPacket *ap=(AresPacket*)events->GetCurrent();
		if(!ap)
			break;
		if(ap->GetEventType()==ARES_RECEIVE_EVENT)
			m_event_list.InsertItem(count,"Receive");
		else
			m_event_list.InsertItem(count," ->Send");
		
		m_event_list.SetItemText(count,1,ap->GetFormattedCreateTime());

		CString tmp;
		tmp.Format("0x%.2X",ap->GetMessageType());

		m_event_list.SetItemText(count,2,tmp);
		
		tmp.Format("%u",ap->GetLength());
		m_event_list.SetItemText(count,3,tmp);
		count++;
		events->Advance();
	}

	Vector v_tmp_group;
	ac->GetSharedFiles(v_tmp_group);
	if(v_tmp_group.Size()>0){
		Vector *v_files=(Vector*)v_tmp_group.Get(0);;
		CString tmp;
		tmp.Format("%d files shared: \r\n\r\n",v_files->Size());
		for(int i=0;i<(int)v_files->Size();i++){
			AresFile* tf=(AresFile*)v_files->Get(i);
			CString tmp2;
			tmp2.Format("%s \t%s\t %u\r\n",tf->m_file_name.c_str(),tf->m_file_extension.c_str(),tf->m_size);
			tmp+=tmp2;
		}
		m_hex_view.SetWindowText(tmp);
	}
	else m_hex_view.SetWindowText("");
}

void CAresDialog::OnLvnKeydownUltrapeers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	OnUltrapeerSelectionChange();
	*pResult = 0;
}

void CAresDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	AresProtectionSystemReference ref;
	ref.System()->ResetConnectionPurgeTimer();

	CDialog::OnMouseMove(nFlags, point);
}

void CAresDialog::OnBnClickedPurgedeadconnectionsnow()
{
	AresProtectionSystemReference ref;
	ref.System()->ForceConnectionPurge();

}
