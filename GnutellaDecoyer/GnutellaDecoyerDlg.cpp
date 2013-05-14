// GnutellaDecoyerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GnutellaDecoyerDll.h"
#include "GnutellaDecoyerDlg.h"


// CGnutellaDecoyerDlg dialog

IMPLEMENT_DYNAMIC(CGnutellaDecoyerDlg, CDialog)
CGnutellaDecoyerDlg::CGnutellaDecoyerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGnutellaDecoyerDlg::IDD, pParent)
	, m_vendor_counts_enabled(FALSE)
{
}

CGnutellaDecoyerDlg::~CGnutellaDecoyerDlg()
{
}

void CGnutellaDecoyerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_log_list);
	DDX_Control(pDX, IDC_Module_Connection_List, m_module_connection_list);
	DDX_Control(pDX, IDC_Supernode_Vendor_List, m_supernode_vendor_list);
	DDX_Check(pDX, IDC_VENDOR_COUNTS_CHECK, m_vendor_counts_enabled);
}


BEGIN_MESSAGE_MAP(CGnutellaDecoyerDlg, CDialog)
	ON_BN_CLICKED(IDC_RECONNECT_STATIC, OnBnClickedReconnectStatic)
	ON_BN_CLICKED(IDC_FILE_TRANSFER_BUTTON, OnBnClickedFileTransferButton)
	ON_BN_CLICKED(IDC_Disable_Sharing_BUTTON, OnBnClickedDisableSharingButton)
	ON_BN_CLICKED(IDC_Enable_Sharing_BUTTON, OnBnClickedEnableSharingButton)
	ON_BN_CLICKED(IDC_Rescan_Shared_BUTTON, OnBnClickedRescanSharedButton)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Min_Module_Count_Spin, OnDeltaposMinModuleCountSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_Max_Module_Count_Spin, OnDeltaposMaxModuleCountSpin)
	ON_BN_CLICKED(IDC_VENDOR_COUNTS_CHECK, OnBnClickedVendorCountsCheck)
	ON_WM_TIMER()
	ON_MESSAGE(WM_INIT_FILE_SHARING_MANAGER_THREAD_DATA, InitFileSharingManagerThreadData)
	ON_MESSAGE(WM_FILE_SHARING_MANAGER_UPDATE_SHARED_FILES_DONE, FileSharingManagerUpdateSharedFilesDone)
	ON_MESSAGE(WM_FILE_SHARING_MANAGER_REBUILD_SHARING_DB_STARTED, RebuildSharingDBStarted)
	ON_MESSAGE(WM_FILE_SHARING_MANAGER_REBUILD_SHARING_DB_ENDED, RebuildSharingDBEnded)
	ON_MESSAGE(WM_FILE_SHARING_MANAGER_THREAD_PROGRESS, FileSharingManagerThreadProgress)

END_MESSAGE_MAP()


// CGnutellaDecoyerDlg message handlers

void CGnutellaDecoyerDlg::OnBnClickedReconnectStatic()
{
	// TODO: Add your control notification handler code here
}

void CGnutellaDecoyerDlg::OnBnClickedFileTransferButton()
{
	m_ft_dlg.ShowWindow(SW_NORMAL);
}

void CGnutellaDecoyerDlg::OnBnClickedDisableSharingButton()
{
	// TODO: Add your control notification handler code here
}

void CGnutellaDecoyerDlg::OnBnClickedEnableSharingButton()
{
	// TODO: Add your control notification handler code here
}

void CGnutellaDecoyerDlg::OnBnClickedRescanSharedButton()
{
	p_parent->RebuildSharingDB();
}

void CGnutellaDecoyerDlg::OnDeltaposMinModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterModuleCounts((-1)*pNMUpDown->iDelta,0);
	*pResult = 0;
}

void CGnutellaDecoyerDlg::OnDeltaposMaxModuleCountSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	AlterModuleCounts(0,(-1)*pNMUpDown->iDelta);
	*pResult = 0;
}

void CGnutellaDecoyerDlg::OnBnClickedVendorCountsCheck()
{
	m_supernode_vendor_list.DeleteAllItems();
	UpdateData();
	p_parent->EnableVendorCounts(m_vendor_counts_enabled);
}

BOOL CGnutellaDecoyerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	UpdateData(FALSE);

	// TODO:  Add extra initialization here
	m_ft_dlg.Create(IDD_GNUTELLA_FT_DIALOG,this);
	m_ft_dlg.InitParent(this);
	m_ft_dlg.ShowWindow(SW_HIDE);
	m_log_list.SetHorizontalExtent(1000);
	InitRunningSinceWindowText();

	CRect rect;
	m_module_connection_list.GetWindowRect(&rect);
	int nWidth = rect.Width();
	m_module_connection_list.InsertColumn(SUB_MOD,"M",LVCFMT_LEFT,nWidth *47/242);
	m_module_connection_list.InsertColumn(SUB_CONNECTED_CONNECTING_IDLE,"+   0   -",LVCFMT_CENTER,nWidth *80/242);
	m_module_connection_list.InsertColumn(SUB_PING_PONG_PUSH_QUERY_QUERYHIT,"Pi Po Pu Qu",LVCFMT_CENTER,nWidth *115/242);

	m_supernode_vendor_list.GetWindowRect(&rect);
	nWidth = rect.Width();
	m_supernode_vendor_list.InsertColumn(SUB_SNV_NUM,"Num",LVCFMT_LEFT,nWidth *40/260);
	m_supernode_vendor_list.InsertColumn(SUB_SNV_VENDOR,"Vendor",LVCFMT_LEFT,nWidth *220/260);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CGnutellaDecoyerDlg::InitParent(GnutellaDecoyerDll* parent)
{
	p_parent = parent;
	char buf[256];
	sprintf(buf,"%u",p_parent->AlterModuleCounts(-1,-1));
	GetDlgItem(IDC_Min_Modules_Static)->SetWindowText(buf);
	sprintf(buf,"%u",p_parent->AlterModuleCounts(1,1));
	GetDlgItem(IDC_Max_Modules_Static)->SetWindowText(buf);
}

//
//
//
void CGnutellaDecoyerDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	p_parent->OnTimer(nIDEvent);

	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CGnutellaDecoyerDlg::Log(const char* log)
{
	CString msg = (CTime::GetCurrentTime()).Format("%Y-%m-%d %H:%M:%S - ");
	msg += log;
	if(m_log_list.GetCount()>50000)
	{
		m_log_list.DeleteString(m_log_list.GetCount()-1);
	}
	m_log_list.InsertString(0,msg);
}

//
//
//
void CGnutellaDecoyerDlg::ModuleCountHasChanged(int count)
{
	// See if we have more modules than needed displayed
	while(m_module_connection_list.GetItemCount()>count)
	{
		m_module_connection_list.DeleteItem(m_module_connection_list.GetItemCount()-1);
	}
}

//
//
//
void CGnutellaDecoyerDlg::ReportConnectionStatus(ConnectionModuleStatusData &status)//,vector<ConnectionModuleStatusData> *all_mod_status)
{
	UINT i;
	char buf[1024+1];

	// See if this mod is already represented in the list
	int index=-1;
	for(i=0;i<(UINT)m_module_connection_list.GetItemCount();i++)
	{
		if(m_module_connection_list.GetItemData(i)==status.m_mod)
		{
			index=i;
			break;
		}
	}

	// If we didn't find it, then add a new item
	if(index==-1)
	{
		sprintf(buf,"%02u",status.m_mod);
		index=m_module_connection_list.InsertItem(m_module_connection_list.GetItemCount(),buf,0);
		m_module_connection_list.SetItemData(index,status.m_mod);
	}

	sprintf(buf,"%02u %02u %02u",status.m_connected_socket_count,status.m_connecting_socket_count,status.m_idle_socket_count);
	m_module_connection_list.SetItemText(index,SUB_CONNECTED_CONNECTING_IDLE,buf);

	// Compile the ping,pong,push,query,query-hit counts
	unsigned int pi=0,po=0,pu=0,qu=0,qh=0;
	
	pi+=status.m_ping_count;
	po+=status.m_pong_count;
	pu+=status.m_push_count;
	qu+=status.m_query_count;

	sprintf(buf,"%u : %u : %u : %u",pi,po,pu,qu);
	m_module_connection_list.SetItemText(index,SUB_PING_PONG_PUSH_QUERY_QUERYHIT,buf);
}

//
//
//
void CGnutellaDecoyerDlg::UpdateVendorCounts(vector<VendorCount> &vendor_counts)
{
	UINT i;

	// Sort the list
	sort(vendor_counts.begin(),vendor_counts.end());

	m_supernode_vendor_list.DeleteAllItems();

	for(i=0;i<vendor_counts.size();i++)
	{
		char count[32];
		_itoa(vendor_counts[i].m_count,count,10);
		int index=m_supernode_vendor_list.InsertItem(0,count,0);
		m_supernode_vendor_list.SetItemText(index,1,vendor_counts[i].m_vendor.c_str());
	}

	char buf[1024];
	CTime now=CTime::GetCurrentTime();
	sprintf(buf,"Supernode Vendors as of : %04u-%02u-%02u %02u:%02u:%02u",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
	GetDlgItem(IDC_Supernode_Vendors_Static)->SetWindowText(buf);
}

//
//
//
void CGnutellaDecoyerDlg::AlterModuleCounts(int dmin,int dmax)
{
	p_parent->AlterModuleCounts(dmin,dmax);	

	int min=p_parent->AlterModuleCounts(-1,-1);
	int max=p_parent->AlterModuleCounts(1,1);

	// Update both values
	char buf[256];
	sprintf(buf,"%u",min);
	GetDlgItem(IDC_Min_Modules_Static)->SetWindowText(buf);
	sprintf(buf,"%u",max);
	GetDlgItem(IDC_Max_Modules_Static)->SetWindowText(buf);

	// See if we have more modules than needed displayed
	while(m_module_connection_list.GetItemCount()>max)
	{
		m_module_connection_list.DeleteItem(m_module_connection_list.GetItemCount()-1);
	}
}

//
//
//
void CGnutellaDecoyerDlg::InitRunningSinceWindowText()
{
	// Update the time that the values were last cleared
	CTime time;
	time=CTime::GetCurrentTime();
	CString time_str = time.Format("Gnutella Decoyer: Running since %A %m/%d/%Y at %H:%M");

	char msg[1024];
	char date[32];
	sprintf(date,__DATE__);
	char m[16];
	unsigned int d;
	unsigned int y;
	sscanf(date,"%s %u %u",&m,&d,&y);
	sprintf(msg,"[ Built on %s %02u, %u at %s ]",m,d,y,__TIME__);

	time_str += (" - ");
	time_str += msg;
	SetWindowText(time_str);
}

//
//
//
LRESULT CGnutellaDecoyerDlg::InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam)
{
	p_parent->InitFileSharingManagerThreadData(wparam,lparam);
	return 0;
}

//
//
//
LRESULT CGnutellaDecoyerDlg::FileSharingManagerUpdateSharedFilesDone(WPARAM wparam,LPARAM lparam)
{
	p_parent->FileSharingManagerUpdateSharedFilesDone();
	return 0;
}

//
//
//
LRESULT CGnutellaDecoyerDlg::RebuildSharingDBStarted(WPARAM wparam,LPARAM lparam)
{
	CString text="Rebuilding sharing DB started at: ";
	text.AppendFormat("%s",CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"));
	GetDlgItem(IDC_Rebuilding_Sharing_DB_Started)->SetWindowText(text);
	return 0;
}

//
//
//
LRESULT CGnutellaDecoyerDlg::RebuildSharingDBEnded(WPARAM wparam,LPARAM lparam)
{
	CString text="Rebuilding sharing DB ended at: ";
	text.AppendFormat("%s",CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"));
	GetDlgItem(IDC_Rebuilding_Sharing_DB_Ended)->SetWindowText(text);
	return 0;
}

//
//
//
LRESULT CGnutellaDecoyerDlg::FileSharingManagerThreadProgress(WPARAM wparam,LPARAM lparam)
{
	CString text;
	text.AppendFormat("%d %% done",(int)wparam);
	GetDlgItem(IDC_FS_MANAGER_THREAD_PROGRESS_STATIC)->SetWindowText(text);
	return 0;
}

//
//
//
void CGnutellaDecoyerDlg::UpdateSpoofSize(UINT spoof_size, UINT temp_spoof_size,UINT hash_size)
{
	char buf[128];
	sprintf(buf,"Decoy Entries Size: %u", spoof_size);
	GetDlgItem(IDC_SPOOF_SIZE_STATIC)->SetWindowText(buf);
	sprintf(buf,"Temp Decoy Entries Size: %u", temp_spoof_size);
	GetDlgItem(IDC_TEMP_SPOOF_SIZE_STATIC)->SetWindowText(buf);
	sprintf(buf,"Hash Table Size: %u", hash_size);
	GetDlgItem(IDC_HASH_SIZE_STATIC)->SetWindowText(buf);
}

//
//
//
void CGnutellaDecoyerDlg::ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status)
{
	m_ft_dlg.ReportStatus(status);
}

//
//
//
int CGnutellaDecoyerDlg::GetModCount()
{
	return p_parent->GetModCount();
}