// ComDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ComDialog.h"


namespace comspace{
	CComDialog g_com_dialog;
}

IMPLEMENT_DYNAMIC(CComDialog, CDialog)
CComDialog::CComDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CComDialog::IDD, pParent)
{
	p_dispatcher=NULL;
	m_int_total_connections=0;
	mb_enable_logging=false;

	m_int_send_counter=0;
	m_int_receive_counter=0;
	m_int_send_counter_actual=0;
}

CComDialog::~CComDialog()
{
}

void CComDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TOTALTCPSOCKETS, m_total_tcp_sockets);
	DDX_Control(pDX, IDC_APPLICATIONSTATS, m_application_stats);
	DDX_Control(pDX, IDC_ACTIVECONNECTIONS, m_active_connections);
	DDX_Control(pDX, IDC_DOWNLOAD_BANDWIDTH, m_current_download_bandwidth);
	DDX_Control(pDX, IDC_UPLOAD_BANDWIDTH, m_current_upload_bandwidth);
	DDX_Control(pDX, IDC_TOTAL_DOWNLOAD, m_total_download);
	DDX_Control(pDX, IDC_TOTAL_UPLOAD, m_total_upload);
	DDX_Control(pDX, IDC_ENABLESTATISTICS, m_enable_statistics);
	DDX_Control(pDX, IDC_SEND_COUNTER, m_send_counter);
	DDX_Control(pDX, IDC_RECEIVE_COUNTER, m_receive_counter);
	DDX_Control(pDX, IDC_SEND_COUNTER2, m_send_counter_actual);
}


BEGIN_MESSAGE_MAP(CComDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ENABLESTATISTICS, OnEnableStatistics)
END_MESSAGE_MAP()


// CComDialog message handlers

void CComDialog::SetTotalTCPConnections(UINT num_connections)
{
	m_int_total_connections=num_connections;

}

void CComDialog::PostSend(UINT op_code, const char* source, UINT data_length)
{
	m_int_send_counter_actual++;
	if(mb_enable_logging)
		mv_sent_stats.Add(new StatData(op_code,source,data_length));
}

void CComDialog::PostReceive(UINT op_code, const char* source, UINT data_length)
{
	m_int_receive_counter++;
	if(mb_enable_logging)
		mv_received_stats.Add(new StatData(op_code,source,data_length));
}
void CComDialog::OnTimer(UINT nIDEvent)
{
	static bool b_in_timer=false;
	if(b_in_timer)
		return;
	//_ASSERTE( _CrtCheckMemory( ) );
	b_in_timer=true;
	static Timer last_stat_check;
	static Timer last_active_check;
	static Timer total_update;
	static Timer counter_update;
	static Timer last_trim;
	static Timer last_trapper_on_update;
	if(p_dispatcher)
		p_dispatcher->Process();

	if(last_trim.HasTimedOut(60)){
		CheckExpired();
		last_trim.Refresh();
	}
	if(last_stat_check.HasTimedOut(10) && mb_enable_logging){
		CheckStats();
		last_stat_check.Refresh();
	}

	if(last_active_check.HasTimedOut(10) && mb_enable_logging){
		CheckActives();
		last_active_check.Refresh();
	}

	if(total_update.HasTimedOut(5)){
		CString str;
		str.Format("%d",m_int_total_connections);
		m_total_tcp_sockets.SetWindowText(str);
		total_update.Refresh();
	}

	if(counter_update.HasTimedOut(2)){
		CString str;
		str.Format("%d",m_int_receive_counter);
		m_receive_counter.SetWindowText(str);
		str.Format("%d",m_int_send_counter);
		m_send_counter.SetWindowText(str);
		str.Format("%d",m_int_send_counter_actual);
		m_send_counter_actual.SetWindowText(str);
		counter_update.Refresh();
	}

	if(last_trapper_on_update.HasTimedOut(120)){
		EncodeTrapperOn();  //write out to a file that trapper keeper was turned on.
		last_trapper_on_update.Refresh();
	}

	b_in_timer=false;
	CDialog::OnTimer(nIDEvent);
	//_ASSERTE( _CrtCheckMemory( ) );
}

BOOL CComDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	TRACE("CComDialog::OnInitDialog() BEGIN\n");
	this->SetTimer(1,5,NULL);

	mv_sent_stats.EnsureSize(10000);
	mv_received_stats.EnsureSize(10000);
	m_application_stats.InsertColumn(0,"ID",LVCFMT_CENTER,85);
	m_application_stats.InsertColumn(1,"Sending (KB/s)",LVCFMT_CENTER,126);
	m_application_stats.InsertColumn(2,"Receiving (KB/s)",LVCFMT_CENTER,135);
	m_application_stats.InsertColumn(3,"Uploaded (1 h) (MB)",LVCFMT_CENTER,135);
	m_application_stats.InsertColumn(4,"Downloaded (1 h) (MB)",LVCFMT_CENTER,135);

	m_active_connections.InsertColumn(0,"IP",LVCFMT_CENTER,95);
	m_active_connections.InsertColumn(1,"Sending (KB/s)",LVCFMT_CENTER,121);
	m_active_connections.InsertColumn(2,"Receiving (KB/s)",LVCFMT_CENTER,133);
	m_active_connections.InsertColumn(3,"Uploaded (1 h) (MB)",LVCFMT_CENTER,133);
	m_active_connections.InsertColumn(4,"Downloaded (1 h) (MB)",LVCFMT_CENTER,133);

	TRACE("CComDialog::OnInitDialog() END\n");
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CComDialog::OnDestroy()
{
	this->KillTimer(1);
	CDialog::OnDestroy();
}

//call to update the table of active connections
void CComDialog::CheckActives(void){
	mv_active_connections.Clear();

	UINT end=mv_received_stats.Size();
	if(end>0){
		for(int i=end-1;i>=0;i--){ //go through all of em
			StatData *st=(StatData*)mv_received_stats.Get(i);
			//if(!st->time.HasTimedOut(600)) //if it has received data in the last 10 minutes it is "active"
			AddConnectionData(st,true);  //if it is even still in this list then it is active
		}
	}

	end=mv_sent_stats.Size();
	if(end>0){
		for(int i=end-1; i>=0;i--){ //go through all of em
			StatData *st=(StatData*)mv_sent_stats.Get(i);
			//if(!st->time.HasTimedOut(600)) //if it has sent data in the last 10 minutes it is "active"
			AddConnectionData(st,false);  //if it is even still in this list then it is active
		}
	}

	//set the connection stats

	//delete any extra ones
	while(m_active_connections.GetItemCount()>(int)mv_active_connections.Size()){
		m_active_connections.DeleteItem(m_active_connections.GetItemCount()-1);
	}

	mv_active_connections.Sort(true);  //sort them by IP

	//set fields
	for(int i=0;i<(int)mv_active_connections.Size();i++){
		AppData* ad=(AppData*)mv_active_connections.Get(i);
		if(i>m_active_connections.GetItemCount()-1){
			m_active_connections.InsertItem(i,ad->GetDest());
		}
		else{
			m_active_connections.SetItem(i,0,LVIF_TEXT,ad->GetDest(),0,0,0,0);	
		}
		CString tmp;
		tmp.Format("%.2f",(float)ad->GetUploadBandwidth());
		m_active_connections.SetItem(i,1,LVIF_TEXT,tmp,0,0,0,0);
		tmp.Format("%.2f",(float)ad->GetDownloadBandwidth());
		m_active_connections.SetItem(i,2,LVIF_TEXT,tmp,0,0,0,0);
		tmp.Format("%.3f",(float)ad->GetUploaded());
		m_active_connections.SetItem(i,3,LVIF_TEXT,tmp,0,0,0,0);
		tmp.Format("%.3f",(float)ad->GetDownloaded());
		m_active_connections.SetItem(i,4,LVIF_TEXT,tmp,0,0,0,0);
	}	
}

void CComDialog::CheckStats(void)
{
	static Timer last_connection_clear;
	double d_upload_bandwidth=0.0;
	double d_download_bandwidth=0.0;
	double d_upload_total=0.0;
	double d_download_total=0.0;
	
	//clear all the app data
	for(UINT i=0;i<mv_app_data.Size();i++){
		AppData *ad=(AppData*)mv_app_data.Get(i);
		ad->Clear();
	}

	//calculate averages for last 10 seconds for current download bandwidth;
	UINT end=mv_received_stats.Size();
	UINT download_bandwidth_total=0;
	if(end>0){
		for(int i=end-1;i>=0;i--){
			StatData *st=(StatData*)mv_received_stats.Get(i);
			if(!st->time.HasTimedOut(10))
				download_bandwidth_total+=st->m_length;
			d_download_total+=st->m_length;
			AddAppData(st,true);
		}
	}

	//calculate our current download bandwidth
	d_download_bandwidth=(double)download_bandwidth_total;
	d_download_bandwidth/=(double)(1<<10); //convert to KB;
	d_download_bandwidth/=10.0;  //convert to KB/s

	//calculate averages for last 10 seconds for current upload bandwidth;
	end=mv_sent_stats.Size();
	UINT upload_bandwidth_total=0;
	if(end>0){
		for(int i=end-1; i>=0;i--){ //go through all of em
			StatData *st=(StatData*)mv_sent_stats.Get(i);
			if(!st->time.HasTimedOut(10))
				upload_bandwidth_total+=st->m_length;
			d_upload_total+=st->m_length;
			AddAppData(st,false);
		}
	}

	//calculate our current upload bandwidth
	d_upload_bandwidth=(double)upload_bandwidth_total;
	d_upload_bandwidth/=(double)(1<<10); //convert to KB;
	d_upload_bandwidth/=10.0;  //convert to KB/s

	//convert totals to megabytes
	d_upload_total/=(double)(1<<20); //convert to MB;
	d_download_total/=(double)(1<<20); //convert to MB;
	
	//At this point we have all our averages and totals, set them
	CString tmp;
	tmp.Format("%.2f KB/s",(float)d_upload_bandwidth);
	m_current_upload_bandwidth.SetWindowText(tmp);
	tmp.Format("%.2f KB/s",(float)d_download_bandwidth);
	m_current_download_bandwidth.SetWindowText(tmp);
	tmp.Format("%.3f MB",(float)d_upload_total);
	m_total_upload.SetWindowText(tmp);
	tmp.Format("%.3f MB",(float)d_download_total);
	m_total_download.SetWindowText(tmp);


	//set the application stats
	for(int i=0;i<(int)mv_app_data.Size();i++){
		AppData* ad=(AppData*)mv_app_data.Get(i);
		if(i>m_application_stats.GetItemCount()-1){
			CString id;
			id.Format("%d",ad->GetApp());
			m_application_stats.InsertItem(i,id);
		}
		CString tmp;
		tmp.Format("%.2f",(float)ad->GetUploadBandwidth());
		m_application_stats.SetItem(i,1,LVIF_TEXT,tmp,0,0,0,0);
		tmp.Format("%.2f",(float)ad->GetDownloadBandwidth());
		m_application_stats.SetItem(i,2,LVIF_TEXT,tmp,0,0,0,0);
		tmp.Format("%.3f",(float)ad->GetUploaded());
		m_application_stats.SetItem(i,3,LVIF_TEXT,tmp,0,0,0,0);
		tmp.Format("%.3f",(float)ad->GetDownloaded());
		m_application_stats.SetItem(i,4,LVIF_TEXT,tmp,0,0,0,0);
	}
}

void CComDialog::SetDispatcher(MessageDispatcher* dispatcher)
{
	p_dispatcher=dispatcher;
}

//This function maintains a vector of application information
void CComDialog::AddAppData(CComDialog::StatData* data,bool b_download)
{
	//application specific data
	for(UINT i=0;i<mv_app_data.Size();i++){
		AppData *ad=(AppData*)mv_app_data.Get(i);
		if(ad->IsApp(data->m_op_code)){
			if(b_download)
				ad->AddDownload(data->m_length,!data->time.HasTimedOut(10));
			else
				ad->AddUpload(data->m_length,!data->time.HasTimedOut(10));
			return;
		}
	}

	//this app isn't yet known, lets create it and init it.
	AppData *ad=new AppData(data->m_op_code,NULL);
	if(b_download)
		ad->AddDownload(data->m_length,!data->time.HasTimedOut(10));
	else
		ad->AddUpload(data->m_length,!data->time.HasTimedOut(10));
	mv_app_data.Add(ad);
}

//This function maintains a vector of application information
void CComDialog::AddConnectionData(CComDialog::StatData* data,bool b_download)
{
	//connection specific data
	
	for(UINT i=0;i<mv_active_connections.Size();i++){
		AppData *ad=(AppData*)mv_active_connections.Get(i);
		if(ad->IsDest(data->m_source.c_str())){
			if(b_download)
				ad->AddDownload(data->m_length,!data->time.HasTimedOut(10));
			else
				ad->AddUpload(data->m_length,!data->time.HasTimedOut(10));
			return;
		}
	}

	//this connection isn't yet known, lets create it and init it.
	AppData *ad=new AppData(NULL,data->m_source.c_str());
	if(b_download)
		ad->AddDownload(data->m_length,!data->time.HasTimedOut(10));
	else
		ad->AddUpload(data->m_length,!data->time.HasTimedOut(10));
	mv_active_connections.Add(ad);
}
void CComDialog::CheckExpired(void)
{
	//clean out the sends that are older than 1 hour.
	int offset=0;
	for(UINT i=0;i<mv_sent_stats.Size();i++){  //clean off the few old ones at the start.
		StatData *sd=(StatData*)mv_sent_stats.Get(i);
		if(sd->time.HasTimedOut(60*60))
			offset=i;
		else break;
	}
	if(offset>0)
		mv_sent_stats.RemoveRange(0,offset+1);

	//clean out the receives that are older than 1 hour.
	offset=0;
	for(UINT i=0;i<mv_received_stats.Size();i++){  //clean off the few old ones at the start.
		StatData *sd=(StatData*)mv_received_stats.Get(i);
		if(sd->time.HasTimedOut(60*60))
			offset=i;
		else break;
	}
	if(offset>0)
		mv_received_stats.RemoveRange(0,offset+1);
	//At this point we have a clean list that only has data up to 1 hours old.
}

void CComDialog::OnEnableStatistics()
{
	int stat=m_enable_statistics.GetCheck();
	if(!stat) mb_enable_logging=false;
	else mb_enable_logging=true;
}

void CComDialog::EncodeTrapperOn(void)
{
	HANDLE file_handle = CreateFile((LPCSTR)"c:\\syncher\\trapper_status.dat", // open file at local_path 
                GENERIC_WRITE,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                CREATE_ALWAYS,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){ //evil file check
		CloseHandle(file_handle);
		return;
	}

	
	byte *buffer=new byte[sizeof(CTime)];  
	*(CTime*)buffer=CTime::GetCurrentTime();  //set the buffer to the current time
	DWORD tmp;
	BOOL stat=WriteFile(file_handle,buffer,sizeof(CTime),(DWORD*)&tmp,NULL);
	CloseHandle(file_handle);	
	delete []buffer;
}

void CComDialog::PostSendRequest(UINT op_code, const char* source, UINT data_length)
{
	m_int_send_counter++;
}
