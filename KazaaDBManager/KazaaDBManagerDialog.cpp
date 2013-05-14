// KazaaDBManagerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "KazaaDBManagerDialog.h"


// KazaaDBManagerDialog dialog

IMPLEMENT_DYNAMIC(KazaaDBManagerDialog, CDialog)
KazaaDBManagerDialog::KazaaDBManagerDialog(CWnd* pParent /*=NULL*/)
	: CDialog(KazaaDBManagerDialog::IDD, pParent)
{
	m_last_progress=0;
	m_last_max_progress=0;
}

KazaaDBManagerDialog::~KazaaDBManagerDialog()
{
}

void KazaaDBManagerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESSBAR, m_progress_bar);
	DDX_Control(pDX, IDC_STATUS, m_status_text);
	DDX_Control(pDX, IDC_FILELIST, m_file_list);
	DDX_Control(pDX, IDC_SECONDSTORESCAN, m_seconds_to_rescan);
	DDX_Control(pDX, IDC_FILESCHANGING, m_files_changing);
	DDX_Control(pDX, IDC_KAZAADBCURRENT, m_kazaa_db_current);
}


BEGIN_MESSAGE_MAP(KazaaDBManagerDialog, CDialog)
	ON_BN_CLICKED(IDC_SCANFILES, OnScanFiles)
	ON_BN_CLICKED(IDC_WRITETODATABASES, OnWriteToDatabases)
	ON_BN_CLICKED(IDC_CLEARFILES, OnClearFiles)
	ON_BN_CLICKED(IDC_DELETEDATABASES, OnDeleteDatabases)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_FILELIST, OnLvnColumnClickFilelist)
END_MESSAGE_MAP()


// KazaaDBManagerDialog message handlers

void KazaaDBManagerDialog::OnScanFiles()
{
	m_system.mb_user_request_rescan=true;  //signal the system to do rescan the files
}

void KazaaDBManagerDialog::OnWriteToDatabases()
{
	m_system.mb_user_request_write_databases=true;  //signal the system to write what it has out to the database
}

void KazaaDBManagerDialog::OnClearFiles()
{
	m_system.mb_user_request_clear_files=true;  //signal the system to clear the files
}

void KazaaDBManagerDialog::OnDeleteDatabases()
{
	m_system.mb_user_request_delete=true;  //signal the system to delete the kazaa databases
}

BOOL KazaaDBManagerDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_system.StartThread();

	this->SetTimer(1,200,NULL);
	this->SetTimer(2,1000,NULL);
	this->SetTimer(3,3000,NULL);
	this->SetTimer(4,30000,NULL);

	m_file_list.InsertColumn(0,"Folder",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(1,"File Name",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(2,"File Size (Bytes)",LVCFMT_LEFT,100);
	m_file_list.InsertColumn(3,"Kazaa Hash",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(4,"Description",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(5,"Keywords",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(6,"Title",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(7,"Artist",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(8,"Album",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(9,"Category",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(10,"Quality",LVCFMT_LEFT,90);
	m_file_list.InsertColumn(11,"Integrity",LVCFMT_LEFT,70);
	m_file_list.InsertColumn(12,"Movie",LVCFMT_LEFT,70);
	m_file_list.SetExtendedStyle(LVS_EX_FULLROWSELECT|m_file_list.GetExtendedStyle());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void KazaaDBManagerDialog::OnDestroy()
{
	this->KillTimer(1);
	this->KillTimer(2);
	this->KillTimer(3);
	this->KillTimer(4);
	//m_system.StopThread();
	//TRACE("KazaaDBManager KazaaDBManagerDialog::OnDestroy() system thread stopped.\n");
	CDialog::OnDestroy();
}

void KazaaDBManagerDialog::OnTimer(UINT nIDEvent)
{
	if(nIDEvent==1){  //update the progress bar
		int progress;
		int max_progress;
		string status=m_system.GetStatus();
		m_system.GetProgress(progress,max_progress);
		
		if(m_last_max_progress!=max_progress){
			m_last_max_progress=max_progress;
			m_progress_bar.SetRange(0,max_progress);
		}
		if(progress!=m_last_progress){
			m_last_progress=progress;
			m_progress_bar.SetPos(progress);
		}

		if(stricmp(m_status.c_str(),status.c_str())!=0){
			m_status=status;
			m_status_text.SetWindowText(status.c_str());
		}
	}
	else if(nIDEvent==2){  //update time to autorescan
		CString tmp;
		int minutes,seconds;
		m_system.GetTimeToRescan(minutes,seconds);
		tmp.Format("%d:%d",minutes,seconds);
		m_seconds_to_rescan.SetWindowText(tmp);
	}
	else if(nIDEvent==3){  //update files changing status
		CString tmp;
		if(m_system.GetFilesChanging()){
			m_files_changing.SetWindowText("Yes");
		}
		else{
			m_files_changing.SetWindowText("No");
		}
		
		if(m_system.GetDBFilesCurrent()){
			m_kazaa_db_current.SetWindowText("Yes");
		}
		else{
			m_kazaa_db_current.SetWindowText("No");
		}

	}
	else if(nIDEvent==4){  //update file list control
		m_system.FillFileListCtrl(m_file_list);
	}
	
	CDialog::OnTimer(nIDEvent);
}

//
//
// callback function used in sorting the file list list control on the gui, so the user can sort by header
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort)
{

	DWORD_PTR *params=(DWORD_PTR *)lParamSort;
	CListCtrl *list=(CListCtrl *)params[0];
	int sub_item=(int)params[1];

	CString cstring1,cstring2;

	HWND hList = list->GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = lParam1;
	int index1 = ListView_FindItem(hList, -1, &lvfi);
	lvfi.lParam = lParam2;
	int index2 = ListView_FindItem(hList, -1, &lvfi);
	cstring1=list->GetItemText(index1,sub_item);
	cstring2=list->GetItemText(index2,sub_item);

	return cstring1.CompareNoCase(cstring2);
}

void KazaaDBManagerDialog::OnLvnColumnClickFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
	params[0]=(DWORD_PTR)&m_file_list;
	params[1]=pNMLV->iSubItem;
	m_file_list.SortItems(CompareFunc,(DWORD_PTR)params);
	*pResult = 0;
}

void KazaaDBManagerDialog::StopSystem(void)
{
	m_system.StopThread();
}
