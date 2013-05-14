// KazaaSupplyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "KazaaSupplyTakerDll.h"
#include "KazaaSupplyDlg.h"
#include "DBInterface.h"
#include "ThreadData.h"
#include "DatabaseItem.h"
#include "SupplyTableEntry.h"
#include "KazaaControllerHeader.h"
#include "Resource.h"

#include <algorithm>
#include <iostream>

#define PROCESSING_THREAD_STATUS_DURATION	10	// seconds

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/*
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
//	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
*/
/////////////////////////////////////////////////////////////////////////////
// CKazaaSupplyDlg dialog

CKazaaSupplyDlg::CKazaaSupplyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKazaaSupplyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKazaaSupplyDlg)
//	m_MaxKazaa = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CKazaaSupplyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKazaaSupplyDlg)
	DDX_Control(pDX, IDC_EDIT_MAX_KAZAA, m_edit_box);
	DDX_Control(pDX, IDC_LIST1, m_list_box);
//	DDX_Control(pDX, IDOK, m_button_go);
//	DDX_Text(pDX, IDC_EDIT_MAX_KAZAA, m_MaxKazaa);
//	DDV_MinMaxInt(pDX, m_MaxKazaa, 1, 60);
	//}}AFX_DATA_MAP
//	DDX_Control(pDX, IDC_STOP_BUTTON, m_button_stop);
//	DDX_Control(pDX, IDC_EDIT_SEARCH_MORE, m_search_more_edit);
}

BEGIN_MESSAGE_MAP(CKazaaSupplyDlg, CDialog)
	ON_MESSAGE(WM_THREAD_LOG,ThreadLog)
//	ON_MESSAGE(WM_PROCESS_RAW_DATA,ProcessRawData)
//	ON_MESSAGE(WM_RAW_DATA_PROCESSING_COMPLETE,ProcessingRawDataComplete)
	ON_MESSAGE(WM_RAW_DUMP,RawDump)
	ON_MESSAGE(WM_FIND_MORE_DUMP, FindMoreDump)

	//{{AFX_MSG_MAP(CKazaaSupplyDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnGo)
	ON_BN_CLICKED(IDC_STOP_BUTTON, OnStop)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_On_Top_Check, OnBnClickedOnTopCheck)
	ON_BN_CLICKED(IDC_MINIMIZE_CHECK, OnBnClickedMinimizeCheck)
	ON_BN_CLICKED(IDC_REQUEST_SNODE_BUTTON, OnBnClickedRequestSnodeButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKazaaSupplyDlg message handlers

BOOL CKazaaSupplyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.
/*
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
*/
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// ***
	//  Initialization is cool....

	DeleteFile("supply_taker_output_log.txt");
	DeleteFile("processing_output_log.txt");

	// DEFAULT SETTINGS:
	m_edit_box.SetWindowText("10");							// default number of kazaas to run (m_MaxKazaa)

	m_kza_launcher.DisableMinimizingKazaaWindows();

	GetDlgItem(IDC_EDIT_SEARCH_MORE)->SetWindowText("15");	// default number of search mores

	m_running = false;
	m_stop_pressed = false;

	GetDlgItem(IDOK)->EnableWindow(FALSE);

	// ***

	return TRUE;  // return TRUE  unless you set the focus to a control
}
/*
void CKazaaSupplyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
*/
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CKazaaSupplyDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CKazaaSupplyDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//
//
//
// ***********************************
//
//	OOOOO NN  N    GGGGG OOOOO  (( ))
//	O	O N N N    G     O   O ((   ))
//	O	O N N N    G  GG O   O ((   ))
//	OOOOO N  NN    GGGGG OOOOO  (( ))
//
// ***********************************
//
//
//
void CKazaaSupplyDlg::OnGo() 
{
	Log(0, NULL, "*** Starting to take supply. ***");

	m_running = true;
	m_stop_pressed = false;

	// just in case
	KillTimer(0);	// for triggering kazaa's, fast
	KillTimer(1);	// for requesting supernodes, once per day

	// Open all the desktops...
	m_desk1 = OpenDesktop("Desktop1",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	m_desk2 = OpenDesktop("Desktop2",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);
	m_desk3 = OpenDesktop("Desktop3",DF_ALLOWOTHERACCOUNTHOOK,FALSE,GENERIC_ALL);

	UpdateData(TRUE);

	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_BUTTON)->EnableWindow(TRUE);
	
	// Set the number of times to search more
	CString str;
	GetDlgItem(IDC_EDIT_SEARCH_MORE)->GetWindowText(str);
	manager.m_search_more_count = atoi(str);

	// Set the maximum number of kazaa's
	char max_kazaa_buf[1024];
	m_edit_box.GetLine(0,max_kazaa_buf,sizeof(max_kazaa_buf));
	unsigned int max_kazaa=atoi(max_kazaa_buf);

	m_kza_launcher.CheckKazaaConnectionStatus(5);	// seconds
	m_kza_launcher.SetKazaaLaunchingInterval(7);	// seconds
	m_kza_launcher.SetMaxKazaaNumber(max_kazaa);
	m_kza_launcher.ResumeLaunchingKazaa();

	// This is just in case we hit start before the program auto-starts...(either way should be fine, as long as we have projects)
	if (manager.v_ptrWorkerGuys.size() < (UINT)max_kazaa)
	{
		unsigned int i=1;
		for (i=1; i <= (unsigned int)max_kazaa; i++)	// Make the workers
		{
			manager.CreateWorkerGuy(i);
		}

		m_worker_iter = manager.v_ptrWorkerGuys.begin();
	}

	SetTimer(3, 5000, NULL);
}

void CKazaaSupplyDlg::OnStop()
{
	Log(0, NULL, " *** Stopping... ***");
	m_running = false;
	m_stop_pressed = true;

	GetDlgItem(IDC_STOP_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	KillTimer(0);	// kazaa trigger
	KillTimer(1);	// supernode reqeuster

	// Close the desktop handles...
	CloseDesktop(m_desk1);
	CloseDesktop(m_desk2);
	CloseDesktop(m_desk3);

	// Close all Kazaa's
//	manager.GetKazaaList();
//	manager.CloseAllKazaas();

	bool ok = false;
	ok = m_kza_launcher.KillAllKazaa();

	manager.FreeWorkerGuys();

}

void CKazaaSupplyDlg::StopTimer()
{
	Log(0,NULL,"--------------------");
	Log(0,NULL,"Timer Stopped.  Waiting for track supply and deletion processing to finish...");
	Log(0,NULL,"--------------------");

	KillTimer(0);
}

void CKazaaSupplyDlg::StartTimer()
{
	ResetWatchDogTimers();

	if (m_running==true)
	{
		Log(0,NULL,"--------------------");
		Log(0,NULL,"Timer Started.");
		Log(0,NULL,"--------------------");

		SetTimer(0, 200, NULL);
	}
	else
	{
		Log(0,NULL,"Recieved 'Start Timer' message, but 'Stop' has been pressed, or we have not begun taking data.  Press 'Go' to begin.");
	}
}

void CKazaaSupplyDlg::ResetWatchDogTimers()
{
	for (UINT i=0; i < manager.v_ptrWorkerGuys.size(); i++)
		manager.v_ptrWorkerGuys[i]->ResetWatchdog();
}

void CKazaaSupplyDlg::OnTimer(UINT nIDEvent) 
{
//	CheckForErrorBoxes();
//	manager.KillOldProcessIds();

	int i=0;
	int timer_time = 200;
	KillTimer(nIDEvent);

	if (nIDEvent == 1)		// request supernodes
	{
//		if (manager.m_got_sd_ip)
//			manager.RequestSuperNodeList();

//		SetTimer(1, (1000*60*60*24), NULL);	// once per day
	}
	else if (nIDEvent == 0 && m_running)	// trigger
	{
		if (m_worker_iter == manager.v_ptrWorkerGuys.end())
			m_worker_iter = manager.v_ptrWorkerGuys.begin();

		// Check to see if Kazaa has disappeared for some lame reason.
//		if (::IsWindow((*m_worker_iter)->m_kza_hwnd) == 0)
//			(*m_worker_iter)->m_state = 1;

		// Trigger the worker guy - only if we are not in the process of changing the project files
		if (manager.m_com.m_reading_new_projects == false)
			(*m_worker_iter)->Trigger();

/*
		// If this worker guy is in state 1, don't move on untill after he moves to state 2, because he is trying to
		// get a new Kazaa and if we move on then a different WorkerGuy could get his...
		if ( (*m_worker_iter)->m_state != 1)
		{
			m_worker_iter++;
		}
		else
		{
			timer_time = 1000;
		}
*/
		m_worker_iter++;
		SetTimer(0, timer_time, NULL);
	}
	else if (nIDEvent == 3)
	{
		manager.GetKazaaList();
		UINT num_kazaas = (UINT)manager.v_temp_kazaa.size();
		UINT num_workers = (UINT)manager.v_ptrWorkerGuys.size();

		if ( num_kazaas == num_workers )
		{
			KillTimer(3);

			m_kza_launcher.CheckKazaaConnectionStatus(30);	// seconds
			m_kza_launcher.SetKazaaLaunchingInterval(10);	// seconds

			StartTimer();	// timer 0, For triggering kazaas
		}
		else
		{
			SetTimer(3, 5000, NULL);
		}
	}
}



//
//
//
void CKazaaSupplyDlg::Log(int worker_id, HWND kza_hwnd, char *buf)
{
	char msg[4096];
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());

	char handle[30];
	memset(handle, 0, sizeof(handle));
	
	// Put the identifiying WorkerGuy's id, and kazaa window handle in the log
	if (worker_id != 0)
	{
		strcat(msg, "(Id:");
		strcat(msg, itoa(worker_id, handle, 10));
		strcat(msg, " HWND:");
		strcat(msg, ltoa((long)kza_hwnd, handle, 16));
		strcat(msg, ") ");
	}

	strcat(msg,buf);
	m_list_box.InsertString(0,msg);

	strcat(msg, "\n");
 
	// Open the log file
	if ( m_log_file.Open("supply_taker_output_log.txt", CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate,NULL)==0 )
		MessageBox("Error Opening supply_taker_output_log.txt", "File Open Error", MB_OK);
	else
		m_log_file.SeekToEnd();

	// Write to the log file
	m_log_file.WriteString(msg);

	// Close the log file
	m_log_file.Close();

	while(m_list_box.GetCount()>1000)
	{
		m_list_box.DeleteString(m_list_box.GetCount()-1);
	}
}

//
//
//
void CKazaaSupplyDlg::Log(int worker_id, int start_ip, int end_ip, char *buf)
{
	char msg[4096];
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());

	char ip[30];
	memset(ip, 0, sizeof(ip));
	
	// Put the identifiying WorkerGuy's id, and kazaa window handle in the log
	if (worker_id != 0)
	{
		char s_ip[32];
		char e_ip[32];

		sprintf(s_ip, "%u.%u.%u.%u", (start_ip>>24)&0xFF,(start_ip>>16)&0xFF,(start_ip>>8)&0xFF,(start_ip>>0)&0xFF);
		sprintf(e_ip, "%u.%u.%u.%u", (end_ip>>24)&0xFF,(end_ip>>16)&0xFF,(end_ip>>8)&0xFF,(end_ip>>0)&0xFF);

		strcat(msg, "(Id:" );
		strcat(msg, itoa(worker_id, ip, 10) );
		strcat(msg, " IP Range:(" );
		strcat(msg, s_ip );
		strcat(msg, "," );
		strcat(msg, e_ip );
		strcat(msg, ") " );
	}

	strcat(msg,buf);
	m_list_box.InsertString(0,msg);

	strcat(msg, "\n");
 
	// Open the log file
	if ( m_log_file.Open("supply_taker_output_log.txt", CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate,NULL)==0 )
		MessageBox("Error Opening supply_taker_output_log.txt", "File Open Error", MB_OK);
	else
		m_log_file.SeekToEnd();

	// Write to the log file
	m_log_file.WriteString(msg);

	// Close the log file
	m_log_file.Close();

	while(m_list_box.GetCount()>1000)
	{
		m_list_box.DeleteString(m_list_box.GetCount()-1);
	}
}

void CKazaaSupplyDlg::OnBnClickedOnTopCheck()
{
	CButton *check=(CButton *)GetDlgItem(IDC_On_Top_Check);
	if(check->GetCheck()==BST_CHECKED)
	{
		::SetWindowPos(GetSafeHwnd(),HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	}
	else	// BST_UNCHECKED
	{
		::SetWindowPos(GetSafeHwnd(),HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	}
}

void CKazaaSupplyDlg::OnCancel()
{
	ShowWindow(SW_HIDE);
}


void CKazaaSupplyDlg::OnBnClickedMinimizeCheck()
{
	CButton *check=(CButton *)GetDlgItem(IDC_MINIMIZE_CHECK);
	if(check->GetCheck()==BST_CHECKED)
	{
		manager.m_minimize = true;
	}
	else
	{
		manager.m_minimize = false;
	}
}

void CKazaaSupplyDlg::CheckForErrorBoxes()
{
	// Check for those annoying kazaa lite not connnected message boxes.
	HWND box_hwnd = NULL;
	box_hwnd = ::FindWindow(NULL, "Kazaa Lite");
	while (box_hwnd != NULL)
	{
		HWND ok_button = ::GetWindow(box_hwnd, GW_CHILD);
		::PostMessage(ok_button, WM_LBUTTONDOWN, (WPARAM)MK_LBUTTON, (LPARAM)0x00050005);
		::PostMessage(ok_button, WM_LBUTTONUP, (WPARAM)0, (LPARAM)0x00050005);
		Log(NULL, NULL, "Clicked OK on 'Kazaa Lite Not Connected' messsage box");
		TRACE("Closing 'Clicked OK on 'Kazaa Lite' messsage box\n");

		Sleep(1000);  // wait for the message box to close, without this it will click a thousand times per second. bad.
		box_hwnd = ::FindWindow(NULL, "Kazaa Lite");  // is there another stupid message box?
	}

	box_hwnd = NULL;
	box_hwnd = ::FindWindow(NULL, "Error");
	while (box_hwnd != NULL)
	{
		HWND ok_button = ::GetWindow(box_hwnd, GW_CHILD);
		::PostMessage(ok_button, WM_LBUTTONDOWN, (WPARAM)MK_LBUTTON, (LPARAM)0x00050005);
		::PostMessage(ok_button, WM_LBUTTONUP, (WPARAM)0, (LPARAM)0x00050005);
		Log(NULL, NULL, "Clicked OK on 'Error' messsage box");
		TRACE("Clicked OK on 'Error' message box.\n");

		Sleep(1000);  // wait for the click to happen.
		box_hwnd = ::FindWindow(NULL, "Error");  // is there another stupid message box?
	}

	box_hwnd = NULL;
	box_hwnd = ::FindWindow(NULL, "kazaalite.kpp - Application Error");
	while (box_hwnd != NULL)
	{
		HWND ok_button = ::GetWindow(box_hwnd, GW_CHILD);
		::PostMessage(ok_button, WM_LBUTTONDOWN, (WPARAM)MK_LBUTTON, (LPARAM)0x00050005);
		::PostMessage(ok_button, WM_LBUTTONUP, (WPARAM)0, (LPARAM)0x00050005);
		Log(NULL, NULL, "Clicked OK on 'kazaalite.kpp - Application Error' message box");
		TRACE("Clicked OK on 'kazaalite.kpp - Application Error' message box\n");

		Sleep(1000);  // wait for the click to happen.
		box_hwnd = ::FindWindow(NULL, "Error");  // is there another stupid message box?
	}

	bool retry_error = false;
	HWND server_busy_hwnd = NULL;
	server_busy_hwnd = ::FindWindow(NULL, "Server Busy");
	while (server_busy_hwnd != NULL)
	{
		retry_error = true;
		HWND temp = NULL;
		HWND switch_to_button = NULL;
		HWND retry_button = NULL;
		
		char name[256];
		memset(name, 0, sizeof(name));

		while(temp = ::GetWindow(server_busy_hwnd, GW_CHILD))
		{
			if (GetClassName(temp, name, sizeof(name)) != 0)
			{
				if (strstr(name, "Button") == 0)
				{
					if (switch_to_button == NULL)
						switch_to_button = temp;
					else
						retry_button = temp;
				}
			}
		}

		::PostMessage(switch_to_button, WM_LBUTTONDOWN, (WPARAM)MK_LBUTTON, (LPARAM)0x00050005);
		::PostMessage(switch_to_button, WM_LBUTTONUP, (WPARAM)0, (LPARAM)0x00050005);

		Log(NULL, NULL, "Clicked 'Swtich To' on 'Server Busy' message box");
		TRACE("Clicked on 'Switch To' button on 'Server Busy' message box...\n");

		Sleep(1000);  // wait for the click to happen.
		server_busy_hwnd = ::FindWindow(NULL, "Server Busy");  // is there another stupid message box?
	}

/*
	if (retry_error)
	{
		manager.GetKazaaList();
		manager.CloseAllKazaas();
	}
*/

}

//
//
//
void ReplaceInvalidCharacters(CString *cstring)
{
	cstring->Replace("\\","\\\\");
	cstring->Replace("'","\\'");
}

/*
//
// DatabaseItem less than operator for projects
//
bool DatabaseItemProjectLessThan(DatabaseItem item1,DatabaseItem item2)
{
	// Compare the project names
	if(strcmp(item1.Column(0),item2.Column(0))<0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool ExtractProjects(HWND hwnd,MYSQL *conn,CString today,vector<DatabaseItem> &raw_projects,bool recursed=false)
{
	unsigned int i;

	CString query;
	MYSQL_RES *res;
	MYSQL_ROW row;

	// Clear the output vector
	raw_projects.clear();

	// Status variables
	CTime time=CTime::GetCurrentTime();
	unsigned int num_rows=0;
	unsigned int row_count=0;
	char *msg=NULL;

	// Select the distinct projects from raw
	query="select distinct project from raw where to_days(timestamp) < ";
	query+=today;
	mysql_query(conn,query);
	res=mysql_store_result(conn);	// allocates memory
	num_rows=(unsigned int)mysql_num_rows(res);
	row_count=0;
	while((row=mysql_fetch_row(res))!=NULL)
	{
		DatabaseItem item;
		item.AddColumn(row[0]);	// project
		raw_projects.push_back(item);

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ExtractProjects() - 1/2 - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}
	}
	mysql_free_result(res);	// frees memory

	// Sort the vector
	sort(raw_projects.begin(),raw_projects.end(),DatabaseItemProjectLessThan);

	// Select all projects from the projects table, and associate the ids with the raw projects
	query="select * from projects";
	mysql_query(conn,query);
	res=mysql_store_result(conn);	// allocates memory
	num_rows=(unsigned int)mysql_num_rows(res);
	row_count=0;
	while((row=mysql_fetch_row(res))!=NULL)
	{
		// Create a database item from the columns
		DatabaseItem item;
		item.AddColumn(row[1]);	// project

		// Find the first and last occurances of this project, if any
		pair<vector<DatabaseItem>::iterator,vector<DatabaseItem>::iterator> item_iters;
		item_iters=equal_range(raw_projects.begin(),raw_projects.end(),item,DatabaseItemProjectLessThan);

		// Make sure that some items were found with this same project
		if(item_iters.first!=raw_projects.end())
		{
			// We've found it
			unsigned int id;
			sscanf(row[0],"%u",&id);
			item_iters.first->ID(id);
		}

//		for(i=0;i<raw_projects.size();i++)
//		{
//			// If the id has not been set yet, and this project matches, then set the id
//			if((raw_projects[i].ID()==0) && (strcmp(raw_projects[i].Column(0),row[1])==0))
//			{
//				unsigned int id;
//				sscanf(row[0],"%u",&id);
//				raw_projects[i].ID(id);
//				break;
//			}
//		}

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ExtractProjects() - 2/2 - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}
	}
	mysql_free_result(res);	// frees memory

	// Check to see if there are any ids that have not been set yet.  If so, add these to the table, and call this function again.
	bool projects_added=false;
	for(i=0;i<raw_projects.size();i++)
	{
		// If this project's id has not been set yet, add it to the table.
		if(raw_projects[i].ID()==0)
		{
			projects_added=true;
			query="insert into projects values(NULL,'";
			query+=raw_projects[i].Column(0);
			query+="')";
			mysql_query(conn,query);
		}
	}

	// If we've added projects, call this function again
	if(projects_added)
	{
		// Check for infinite loops
		if(recursed)
		{
			return false;
		}
		else
		{
			return ExtractProjects(hwnd,conn,today,raw_projects,true);
		}
	}

	return true;
}

//
// less than operator for users
//
bool DatabaseItemUserLessThan(DatabaseItem item1,DatabaseItem item2)
{
	unsigned int ip1,ip2;
	sscanf(item1.Column(0),"%u",&ip1);
	sscanf(item2.Column(0),"%u",&ip2);

	if(ip1<ip2)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool ExtractUsers(HWND hwnd,MYSQL *conn,CString today,vector<DatabaseItem> &raw_users,bool recursed=false)
{
	unsigned int i;

	CString query;
	MYSQL_RES *res;
	MYSQL_ROW row;

	// Clear the output vector
	raw_users.clear();

	// Status variables
	CTime time=CTime::GetCurrentTime();
	unsigned int num_rows=0;
	unsigned int row_count=0;
	char *msg=NULL;

	// Select the distinct ip,user pairs from raw, no longer ordered by ip,user
	query="select distinct ip,user from raw where to_days(timestamp) < ";
	query+=today;
//	query+=" order by ip,user";
	mysql_query(conn,query);
	res=mysql_store_result(conn);	// allocates memory
	num_rows=(unsigned int)mysql_num_rows(res);
	row_count=0;
	while((row=mysql_fetch_row(res))!=NULL)
	{
		DatabaseItem item;
		item.AddColumn(row[0]);	// ip
		item.AddColumn(row[1]);	// user
		raw_users.push_back(item);

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ExtractUsers() - 1/3 - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}
	}
	mysql_free_result(res);	// frees memory

	// Sort the raw users so that we can do a n log n search on the vector
//	msg=new char[1024];
//	strcpy(msg,"PROCESSING THREAD - Sorting Raw Users...");
//	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
	sort(raw_users.begin(),raw_users.end(),DatabaseItemUserLessThan);
//	msg=new char[1024];
//	strcpy(msg,"PROCESSING THREAD - Sorting Complete");
//	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Select all users from the users table, and associate the ids with the raw users, no longer ordered by ip,user
	query="select * from users";// order by ip,user";
	mysql_query(conn,query);
	res=mysql_store_result(conn);	// allocates memory
	num_rows=(unsigned int)mysql_num_rows(res);
	row_count=0;
	while((row=mysql_fetch_row(res))!=NULL)
	{
		// Create a database item from the columns
		DatabaseItem item;
		item.AddColumn(row[1]);	// ip
		item.AddColumn(row[2]);	// username

		// Find the first and last occurances of this ip, if any
		pair<vector<DatabaseItem>::iterator,vector<DatabaseItem>::iterator> item_iters;
		item_iters=equal_range(raw_users.begin(),raw_users.end(),item,DatabaseItemUserLessThan);

		// Make sure that some items were found with this same IP
		if(item_iters.first!=raw_users.end())
		{
			while(1)
			{
				// Check to see if this is the right username for this IP
				if(strcmp(item_iters.first->Column(1),item.Column(1))==0)
				{
					// We've found it
					unsigned int id;
					sscanf(row[0],"%u",&id);
					item_iters.first->ID(id);
					break;
				}

				// Check to see if we didn't find it
				if(item_iters.first==item_iters.second)
				{
					break;
				}

				// Get ready to look at the next username with the same IP
				item_iters.first++;
			}
		}

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ExtractUsers() - 2/3 - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}
	}
	mysql_free_result(res);	// frees memory

	// Check to see if there are any ids that have not been set yet.  If so, add these to the table, and call this function again.
	bool users_added=false;
	num_rows=(unsigned int)raw_users.size();
	row_count=0;
	for(i=0;i<raw_users.size();i++)
	{
		// If this project's id has not been set yet, add it to the table.
		if(raw_users[i].ID()==0)
		{
			// Replace any invalid characters in the username before inserting
			CString user=raw_users[i].Column(1);
			ReplaceInvalidCharacters(&user);

			users_added=true;
			query="insert into users values(NULL,";
			query+=raw_users[i].Column(0);	// ip
			query+=",'";
			query+=user;	// user
			query+="')";
			mysql_query(conn,query);
		}

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ExtractUsers() - 3/3 - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}

	}

	// If we've added projects, call this function again
	if(users_added)
	{
		// Check for infinite loops
		if(recursed)
		{
			return false;
		}
		else
		{
			return ExtractUsers(hwnd,conn,today,raw_users,true);
		}
	}

	return true;
}

//
// less than operator for files
//
bool DatabaseItemFileLessThan(DatabaseItem item1,DatabaseItem item2)
{
	// Compare the hashes
	if(strcmp(item1.Column(3),item2.Column(3))<0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool ExtractFiles(HWND hwnd,MYSQL *conn,CString today,vector<DatabaseItem> &raw_files,bool recursed=false)
{
	unsigned int i;

	CString query;
	MYSQL_RES *res;
	MYSQL_ROW row;

	// Clear the output vector
	raw_files.clear();

	// Status variables
	CTime time=CTime::GetCurrentTime();
	unsigned int num_rows=0;
	unsigned int row_count=0;
	char *msg=NULL;

	// Get the hash counts for file fucking
	query="insert into HashCount select hash, filename, count(hash) from raw where to_days(timestamp) < ";
	query+=today;
	query+=" group by hash";
	mysql_query(conn, query);

	// Select the distinct title,filename,size,hash files from raw
	query="select distinct title,filename,size,hash from raw where to_days(timestamp) < ";
	query+=today;
	mysql_query(conn,query);
	res=mysql_store_result(conn);	// allocates memory
	num_rows=(unsigned int)mysql_num_rows(res);
	row_count=0;
	while((row=mysql_fetch_row(res))!=NULL)
	{
		DatabaseItem item;
		item.AddColumn(row[0]);	// title
		item.AddColumn(row[1]);	// filename
		item.AddColumn(row[2]);	// size
		item.AddColumn(row[3]);	// hash
		raw_files.push_back(item);

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ExtractFiles() - 1/3 - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}
	}
	mysql_free_result(res);	// frees memory

	// We are done with 1/3
	msg=new char[1024];
	strcpy(msg,"PROCESSING THREAD - ExtractFiles() - 1/3 - Completed");
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Sort the vector
	msg=new char[1024];
	strcpy(msg,"PROCESSING THREAD - Sorting Raw Files...");
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
	sort(raw_files.begin(),raw_files.end(),DatabaseItemFileLessThan);
	msg=new char[1024];
	strcpy(msg,"PROCESSING THREAD - Raw Files Sorted");
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Select all files from the files table, and associate the ids with the raw files
	query="select * from files";
	mysql_query(conn,query);
	res=mysql_store_result(conn);	// allocates memory
	num_rows=(unsigned int)mysql_num_rows(res);
	row_count=0;
	while((row=mysql_fetch_row(res))!=NULL)
	{
		// Create a database item from the columns
		DatabaseItem item;
		item.AddColumn(row[1]);	// title
		item.AddColumn(row[2]);	// filename
		item.AddColumn(row[3]);	// size
		item.AddColumn(row[4]);	// hash

		// Find the first and last occurances of this hash, if any
		pair<vector<DatabaseItem>::iterator,vector<DatabaseItem>::iterator> item_iters;
		item_iters=equal_range(raw_files.begin(),raw_files.end(),item,DatabaseItemFileLessThan);

		// Make sure that some items were found with this same hash
		if(item_iters.first!=raw_files.end())
		{
			while(1)
			{
				// Check to see if this is the right title,filename, and size for this hash
				if((strcmp(item_iters.first->Column(0),item.Column(0))==0) && 
					(strcmp(item_iters.first->Column(1),item.Column(1))==0) && 
					(strcmp(item_iters.first->Column(2),item.Column(2))==0))
				{
					// We've found it
					unsigned int id;
					sscanf(row[0],"%u",&id);
					item_iters.first->ID(id);
					break;
				}

				// Check to see if we didn't find it
				if(item_iters.first==item_iters.second)
				{
					break;
				}

				// Get ready to look at the next username with the same IP
				item_iters.first++;
			}
		}

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ExtractFiles() - 2/3 - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}
	}
	mysql_free_result(res);	// frees memory
	
	// We are done with 2/3
	msg=new char[1024];
	strcpy(msg,"PROCESSING THREAD - ExtractFiles() - 2/3 - Completed");
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Check to see if there are any ids that have not been set yet.  If so, add these to the table, and call this function again.
	bool files_added=false;
	num_rows=(unsigned int)raw_files.size();
	row_count=0;
	for(i=0;i<raw_files.size();i++)
	{
		// If this project's id has not been set yet, add it to the table.
		if(raw_files[i].ID()==0)
		{
			// Replace any invalid characters before inserting
			CString title=raw_files[i].Column(0);
			CString filename=raw_files[i].Column(1);
			ReplaceInvalidCharacters(&title);
			ReplaceInvalidCharacters(&filename);

			files_added=true;
			query="insert into files values(NULL,'";
			query+=title;		// title
			query+="','";
			query+=filename;	// filename
			query+="',";
			query+=raw_files[i].Column(2);		// size
			query+=",'";
			query+=raw_files[i].Column(3);		// hash
			query+="')";
			mysql_query(conn,query);
		}

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ExtractFiles() - 3/3 - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}
	}

	// We are done with 3/3
	msg=new char[1024];
	strcpy(msg,"PROCESSING THREAD - ExtractFiles() - 3/3 - Completed");
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// If we've added projects, call this function again
	if(files_added)
	{
		// Check for infinite loops
		if(recursed)
		{
			return false;
		}
		else
		{
			return ExtractFiles(hwnd,conn,today,raw_files,true);
		}
	}

	return true;
}

//
// SupplyTableEntry less than operator
//
bool SupplyTableEntryLessThan(SupplyTableEntry entry1,SupplyTableEntry entry2)
{
	if(entry1.m_project_id<entry2.m_project_id)
	{
		return true;
	}
	else if(entry1.m_project_id>entry2.m_project_id)
	{
		return false;
	}
	else	// they are the same
	{
		if(entry1.m_user_id<entry2.m_user_id)
		{
			return true;
		}
		else if(entry1.m_user_id>entry2.m_user_id)
		{
			return false;
		}
		else	// they are the same
		{
			if(entry1.m_file_id<entry2.m_file_id)
			{
				return true;
			}
			else if(entry1.m_file_id>entry2.m_file_id)
			{
				return false;
			}
			else	// they are the same
			{
				return false;
			}
		}
	}
}

//
//
//
bool ProcessRaw(HWND hwnd,MYSQL *conn,CString today,vector<DatabaseItem> &raw_projects,vector<DatabaseItem> &raw_users,vector<DatabaseItem> &raw_files)
{
	unsigned int i;
	bool ret=true;

	CString query;
	MYSQL_RES *res;
	MYSQL_ROW row;

	// Status variables
	CTime time=CTime::GetCurrentTime();
	unsigned int num_rows=0;
	unsigned int row_count=0;
	char *msg=NULL;

	// Select the latest instances of the distinct entries from raw
	query="select distinct project,ip,user,title,filename,size,hash,max(timestamp),to_days(timestamp) from raw where to_days(timestamp) < ";
	query+=today;
	query+=" group by 1,2,3,4,5,6,7,9";
	mysql_query(conn,query);
	res=mysql_store_result(conn);	// allocates memory
   	num_rows=(unsigned int)mysql_num_rows(res);
	row_count=0;
	while((row=mysql_fetch_row(res))!=NULL)
	{
		unsigned int project_id=0;
		unsigned int user_id=0;
		unsigned int file_id=0;

		// For each raw item, find the ids for them
		for(i=0;i<raw_projects.size();i++)
		{
			if(strcmp(raw_projects[i].Column(0),row[0])==0)
			{
				project_id=raw_projects[i].ID();
				break;
			}
		}
		for(i=0;i<raw_users.size();i++)
		{
			if((strcmp(raw_users[i].Column(0),row[1])==0) && 
				(strcmp(raw_users[i].Column(1),row[2])==0))
			{
				user_id=raw_users[i].ID();
				break;
			}
		}
		for(i=0;i<raw_files.size();i++)
		{
			if((strcmp(raw_files[i].Column(0),row[3])==0) && 
				(strcmp(raw_files[i].Column(1),row[4])==0) &&
				(strcmp(raw_files[i].Column(2),row[5])==0) && 
				(strcmp(raw_files[i].Column(3),row[6])==0))
			{
				file_id=raw_files[i].ID();
				break;
			}
		}

		// This should never happen
		if((file_id==0) || (user_id==0) || (project_id==0))
		{
			ret=false;
			break;
		}

		// Insert all the results with their timestamp: row[7], they must be unique because they are from
		// different days.
		char buf[1024];
		memset(buf,0,sizeof(buf));

		sprintf(buf,"insert into supply values(%u,%u,%u,'%s')",project_id,user_id,file_id,row[7]);	// timestamp
		mysql_query(conn,buf);

		// Check for errors on the insert
		if(mysql_error(conn)[0] != '\0')
		{
			CStdioFile file;
			if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();

				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " (PROCESSING THREAD): ";
				log += query;
				log += "\n";
				log += "Error: ";
				log += mysql_error(conn);
				log += "\n\n";

				file.WriteString(log);
				file.Close();
			}
			else
			{
				DWORD error = GetLastError();
				MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
			}

			TRACE(mysql_error(conn));
			TRACE("\n");
			TRACE(query);
			TRACE("\n");
		}

		row_count++;

		if((CTime::GetCurrentTime()-time).GetTotalSeconds()>PROCESSING_THREAD_STATUS_DURATION)
		{
			time=CTime::GetCurrentTime();
			msg=new char[1024];
			double dividend=(double)row_count;
			double divisor=(double)num_rows;
			sprintf(msg,"PROCESSING THREAD - ProcessRaw() - %.2f %% Complete",100.0*dividend/divisor);
			::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
		}
	}

	mysql_free_result(res);	// frees memory

	return ret;
}

//
//
//
UINT ProcessingThreadProc(LPVOID pParam)
{
	ThreadData *data = (ThreadData *)pParam;

	HWND hwnd = data->m_hwnd;

	char *msg=NULL;

	CTime last_status;

	// Process the raw data
	msg=new char[1024];
	strcpy(msg,"PROCESSING THREAD - Processing Raw Data");
 	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
	last_status=CTime::GetCurrentTime();

	// Store the start time
	DWORD start=GetTickCount();

	// Connect to the database
	MYSQL *conn;
	conn=mysql_init(NULL);
	mysql_real_connect(conn,data->m_db_ip,data->m_db_login,data->m_db_pass,data->m_db_name,0,NULL,0);

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	bool GotLock = false;
	while (GotLock == false || ( (GetTickCount()-start / 1000.0) < (60*5) ) ) // until we get the lock or 5 minutes is up
	{
		// Is the lock free?
		mysql_query(conn, "SELECT GET_LOCK(\"processing_lock\", 5)");	// 1 second timeout, if it timesout, just try again
		res=mysql_store_result(conn);
		row=mysql_fetch_row(res);

		if(strcmp(row[0],"1")==0)	// if we got the lock, the query will return 1
			GotLock = true;

		mysql_free_result(res);
	}

	// Check to see if we got the lock
	if (!GotLock)
	{
		mysql_close(conn);
		delete data;
		return 0;
	}

	msg=new char[1024];
	sprintf(msg,"PROCESSING THREAD - Got the processing lock");
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	CString today,query;
	DWORD tick;

	// To process todays data
//	mysql_query(conn,"select to_days(now())+1");
	
	// To process all previous days data
	mysql_query(conn,"select to_days(now())");

	res=mysql_store_result(conn);	// allocates memory
	row=mysql_fetch_row(res);
	today=row[0];
	mysql_free_result(res);	// frees memory

//	unsigned int num_rows,row_count;

	// Extract all of the distinct items from the raw and find their ids.  If they don't have ids, then create them.
	vector<DatabaseItem> projects;
	vector<DatabaseItem> users;
	vector<DatabaseItem> files;

	// Process Projects
	tick=GetTickCount();
	if(ExtractProjects(hwnd,conn,today,projects)==false)	// check to see if we started getting into an infinite loop
	{
		// Close connection to the database
		mysql_close(conn);

		// Post error message
		msg=new char[1024];
		strcpy(msg,"PROCESSING THREAD - Recursion Error Extracting Projects");
		::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

		// Finished processing the raw data
		::PostMessage(hwnd,WM_RAW_DATA_PROCESSING_COMPLETE,0,0);

		return 1;
	}
	msg=new char[1024];
	sprintf(msg,"PROCESSING THREAD - Project Processing Completed in %.3f sec",((double)(GetTickCount()-tick))/1000.0);
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Process Users
	tick=GetTickCount();
	if(ExtractUsers(hwnd,conn,today,users)==false)	// check to see if we started getting into an infinite loop
	{
		// Close connection to the database
		mysql_close(conn);

		// Post error message
		msg=new char[1024];
		strcpy(msg,"PROCESSING THREAD - Recursion Error Extracting Users");
		::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

		// Finished processing the raw data
		::PostMessage(hwnd,WM_RAW_DATA_PROCESSING_COMPLETE,0,0);
		
		return 2;
	}
	msg=new char[1024];
	sprintf(msg,"PROCESSING THREAD - User Processing Completed in %.3f sec",((double)(GetTickCount()-tick))/1000.0);
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Process Files
	tick=GetTickCount();
	if(ExtractFiles(hwnd,conn,today,files)==false)	// check to see if we started getting into an infinite loop
	{
		// Close connection to the database
		mysql_close(conn);

		// Post error message
		msg=new char[1024];
		strcpy(msg,"PROCESSING THREAD - Recursion Error Extracting Files");
		::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

		// Finished processing the raw data
		::PostMessage(hwnd,WM_RAW_DATA_PROCESSING_COMPLETE,0,0);
		
		return 3;
	}
	msg=new char[1024];
	sprintf(msg,"PROCESSING THREAD - File Processing Completed in %.3f sec",((double)(GetTickCount()-tick))/1000.0);
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Process Raw
	tick=GetTickCount();
	if(ProcessRaw(hwnd,conn,today,projects,users,files)==false)
	{
		// Close connection to the database
		mysql_close(conn);

		// Post error message
		msg=new char[1024];
		strcpy(msg,"PROCESSING THREAD - Horribly Awful Error Processing Raw Data");
		::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

		// Finished processing the raw data
		::PostMessage(hwnd,WM_RAW_DATA_PROCESSING_COMPLETE,0,0);

		return 4;
	}
	msg=new char[1024];
	sprintf(msg,"PROCESSING THREAD - Raw Processing Completed in %.3f sec",((double)(GetTickCount()-tick))/1000.0);
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Delete all of the raw data that is before "today"
	query="delete from raw where to_days(timestamp) < ";
	query+=today;
	mysql_query(conn,query);

	if(mysql_error(conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " (PROCESSING THREAD): ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
	}

	// Store end time
	DWORD end=GetTickCount();

	// Finished processing the raw data
	::PostMessage(hwnd,WM_RAW_DATA_PROCESSING_COMPLETE,0,0);

	msg=new char[1024];
	sprintf(msg,"PROCESSING THREAD - Raw Data Processing Completed in %.3f sec",((double)(end-start))/1000.0);
	::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

	// Release the processing lock
	mysql_query(conn, "SELECT RELEASE_LOCK(\"processing_lock\")");
	res=mysql_store_result(conn);
	row=mysql_fetch_row(res);

	if(strcmp(row[0],"1")==0)	// if it returns 1, the lock was released properly, and ret will be set to true
	{
		msg=new char[1024];
		sprintf(msg,"PROCESSING THREAD - Released processing lock");
		::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
	}
	else
	{
		msg=new char[1024];
		sprintf(msg,"PROCESSING THREAD - Error releasing processing lock");
		::PostMessage(hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
	}

	mysql_free_result(res);

	// Close connection to the database
	mysql_close(conn);

	delete data;
	return 0;
}
*/

//
//
//
UINT RawDumpThreadProc(LPVOID pParam)
{
	ThreadData *data = (ThreadData *)pParam;

	// Connect to the database
	MYSQL *conn;
	conn=mysql_init(NULL);

	if (!mysql_real_connect(conn,data->m_db_ip,data->m_db_login,data->m_db_pass,data->m_db_name,0,NULL,0))
	{
		char *msg=NULL;

		msg=new char[1024];
		sprintf(msg,"(Id:%u DUMPING THREAD) - Failed to connect to database... skipping...",data->m_worker_id);
		::PostMessage(data->m_hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

		CString errmsg = "Failed to connect to database: Error: ";
		errmsg += mysql_error(conn);
		TRACE(errmsg);
	}
	else
	{
		unsigned int i,j;
		char *msg=NULL;

		msg=new char[1024];
		sprintf(msg,"(Id:%u DUMPING THREAD) - Dumping %u raw results to database",data->m_worker_id,data->v_results.size());
		::PostMessage(data->m_hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

		DWORD start=GetTickCount();	// ms since the system was started

		// Write the search results to the database
		char buf[1024];
		for(i=0;i<data->v_results.size();i++)
		{
			CString query,cstring;
			query = "insert into ";
			query += data->m_table_name;
			query += " (project,ip,user,title,filename,track,size,hash,swarming,timestamp) values ('";

			// Project
			query+=data->m_project.c_str();
			query+="',";

			// IP
			sprintf(buf,"%u,'",data->v_results[i].m_ip);
			query+=buf;

			// User
			cstring=data->v_results[i].m_user.c_str();
			cstring.Replace("\\","\\\\");		// replace a slash "\" with a "\\"
			cstring.Replace("'","\\'");
			query+=cstring;
			query+="','";

			// Title
			cstring=data->v_results[i].m_title.c_str();
			cstring.Replace("\\","\\\\");		// replace a slash "\" with a "\\"
			cstring.Replace("'","\\'");
			query+=cstring;
			query+="','";

			// Filename
			cstring=data->v_results[i].m_filename.c_str();
			cstring.Replace("'","\\'");
			query+=cstring;
			query+="',";

			// Track Number
			sprintf(buf,"%u",data->v_results[i].m_track_number);
			query+=buf;
			query+=",";

			// Size (bytes)
			sprintf(buf,"%u",data->v_results[i].m_size);
			query+=buf;
			query+=",'";

			// Hash
			char hash[40+1];
			memset(hash,0,sizeof(hash));
			for(j=0;j<20;j++)
			{
				sprintf(&hash[strlen(hash)],"%02x",(unsigned char)data->v_results[i].m_hash[j]);
			}
			query+=hash;

			// Swarming enabled?
			if (data->m_swarming)
			{
				query+="','T',";
			}
			else
			{
				query+="','F',";
			}

			// Timestamp
			query+="now())";

			// Only add this raw data to the supply if there are no Kazaa separating characters (0xb6) in it
			if(strchr(query,0xb6)==NULL)
			{
				int ret=mysql_query(conn,query);

				if(mysql_error(conn)[0] != '\0')
				{
					CString log;
					log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
					log += " (DUMPING THREAD ERROR) ";
					log += query;
					log += "\n";
					log += "Error: ";
					log += mysql_error(conn);
					log += "\n\n";

					CStdioFile file;
					if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
					{
						file.SeekToEnd();
						file.WriteString(log);
						file.Close();
					}
					else
					{
						DWORD error = GetLastError();
						MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
					}

					TRACE(mysql_error(conn));
					TRACE("\n");
					TRACE(query);
					TRACE("\n");

					char *msg=NULL;
					msg=new char[1024];
					strcpy(msg, log);
					::PostMessage(data->m_hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
				}
			}
		}

		// Close connection to the database
		mysql_close(conn);

		DWORD end=GetTickCount();	// ms since the system was started

		msg=new char[1024];
		sprintf(msg,"(Id:%u DUMPING THREAD) - %u Results Dumped to Database in %.3f sec", data->m_worker_id, data->v_results.size(),((double)(end-start))/1000.0);
		::PostMessage(data->m_hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
	}

	delete data;

	return 0;
}

//
//
//
LRESULT CKazaaSupplyDlg::RawDump(WPARAM wparam, LPARAM lparam)
{
	// Start the thread
	AfxBeginThread(RawDumpThreadProc,(LPVOID)wparam,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	return 0;
}

//
//
//
UINT FindMoreThreadProc(LPVOID pParam)
{
	ThreadData *data = (ThreadData *)pParam;

	// Connect to the database
	MYSQL *conn;
	conn=mysql_init(NULL);

	if (!mysql_real_connect(conn,data->m_db_ip,data->m_db_login,data->m_db_pass,data->m_db_name,0,NULL,0))  // all messed up
	{
		char *msg=NULL;
		msg=new char[1024];
		sprintf(msg,"(Id:%u) FIND MORE THREAD - Failed to connect to database... skipping...",data->m_worker_id);
		::PostMessage(data->m_hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

		CString errmsg = "Failed to connect to database: Error: ";
		errmsg += mysql_error(conn);
		TRACE(errmsg);
	}
	else
	{
		CString project = data->m_project.c_str();

		unsigned int i;
		char *msg=NULL;

		msg=new char[1024];
		sprintf(msg,"(Id:%u) FIND MORE THREAD - Loging %u artists to database",data->m_worker_id,data->v_results.size());
		::PostMessage(data->m_hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);

		DWORD start=GetTickCount();	// ms since the system was started

		char day[4096];
		CTime now=CTime::GetCurrentTime();
		sprintf(day,"%04u-%02u-%02u",now.GetYear(),now.GetMonth(),now.GetDay());


		// Write the search results to the database
		for(i=0;i<data->v_results.size();i++)
		{
			if (data->v_results[i].m_media_type == 1)	// Audio = 1
			{
				CString artist = data->v_results[i].m_artist.c_str();
				CString album = data->v_results[i].m_album.c_str();

				DBInterface::InsertToOtherArtists(conn, project, artist, album, day);
			}
		}

		// Close connection to the database
		mysql_close(conn);

		DWORD end=GetTickCount();	// ms since the system was started

		msg=new char[1024];
		sprintf(msg,"(Id:%u) FIND MORE THREAD - %u artists logged to database in %.3f sec", data->m_worker_id,data->v_results.size(),((double)(end-start))/1000.0);
		::PostMessage(data->m_hwnd,WM_THREAD_LOG,(WPARAM)msg,(LPARAM)0);
	}

	delete data;

	return 0;
}

//
//
//
LRESULT CKazaaSupplyDlg::FindMoreDump(WPARAM wparam, LPARAM lparam)
{
	// Start the thread
	AfxBeginThread(FindMoreThreadProc, (LPVOID)wparam,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	return 0;
}

/*
//
//
//
LRESULT CKazaaSupplyDlg::ProcessRawData(WPARAM wparam,LPARAM lparam)
{
	if (manager.m_processing == true)
		return 0;

	// Set the flag so that the raw data dumping threads know that we are already processing data
	manager.m_processing = true;

	// Start the thread
	AfxBeginThread(ProcessingThreadProc,(LPVOID)wparam,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);

	return 0;
}

//
//
//
LRESULT CKazaaSupplyDlg::ProcessingRawDataComplete(WPARAM wparam,LPARAM lparam)
{
	// Reset the flag so that raw data dumping threads know we are no longer processing data
	manager.m_processing = false;
	return 0;
}
*/

//
//
//
LRESULT CKazaaSupplyDlg::ThreadLog(WPARAM wparam,LPARAM lparam)
{
	char *ptr=(char *)wparam;
	Log(0, NULL, ptr);
	delete [] ptr;
	return 0;
}


void CKazaaSupplyDlg::OnBnClickedRequestSnodeButton()
{
	KazaaControllerHeader message;

	message.op = KazaaControllerHeader::Request_All_Supernodes;
	message.size = 0;

	char *ip = new char[256];
									// HARD-CODED Supernode IPs
//	strcpy(ip,"38.144.197.133");	// cogent 1
//	strcpy(ip,"38.118.154.2");		// some other bullshit ip
	strcpy(ip,"38.118.139.66");

//	manager.m_supernode_com.SendReliableData(ip, &message, sizeof(message));

	MessageBox("This has been disabled.  Now the KazaaLauncher is responsible for this.", "Disabled", MB_OK);
	delete ip;
}

