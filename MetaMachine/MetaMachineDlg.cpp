// MetaMachineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MetaMachineDlg.h"
#include <mmsystem.h>
#include "Buffer2000.h"
#include "MetaSystem.h"


//namespace metamachinespace{
//	CMetaMachineDlg g_meta_dialog;
//}

IMPLEMENT_DYNAMIC(CMetaMachineDlg, CDialog)
CMetaMachineDlg::CMetaMachineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMetaMachineDlg::IDD, pParent)
{
	m_failed_queries=0;
	m_remaining_queries=-1;
}

CMetaMachineDlg::~CMetaMachineDlg()
{
	CloseHandle(m_log_file);
	CloseHandle(m_log_file2);
	CloseHandle(m_log_file3);
}

void CMetaMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RETURNS, m_returns);
	DDX_Control(pDX, IDC_PROJECTINFO, m_project_info);
	DDX_Control(pDX, IDC_SERVER_STATUS, m_server_status_tree);
	DDX_Control(pDX, IDC_FAILQUERYCOUNT, m_fail_query_count_control);
	DDX_Control(pDX, IDC_FAILEDQUERIES, m_lb_failed_queries);
	DDX_Control(pDX, IDC_REMAININGDBQUERIES, m_s_remaining_queries);
	DDX_Control(pDX, IDC_MAINTHREADSTATUS, m_main_thread_alive);
	DDX_Control(pDX, IDC_SERVERLOG, m_server_log);
}


BEGIN_MESSAGE_MAP(CMetaMachineDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_REFRESH, OnProjectInfoRefresh)
	ON_BN_CLICKED(IDC_REFRESHSERVERS, OnRefreshServers)
	ON_BN_CLICKED(IDC_BUILDSPOOFTABLE, OnBuildSpoofTable)
	ON_BN_CLICKED(IDC_REPORTRESULTS, OnReportResults)
	ON_BN_CLICKED(IDC_UPDATEPROJECTSDATABASE, OnUpdateProjectsDatabase)
	ON_BN_CLICKED(IDC_BUILDSWARMTABLE2, OnBuildSwarmTable2)
	ON_BN_CLICKED(IDC_BUILDFALSEDECOYTABLE, OnBuildFalseDecoyTable)
END_MESSAGE_MAP()


// CMetaMachineDlg message handlers

BOOL CMetaMachineDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_log_file= CreateFile("c:\\metamachine\\log_file.txt",			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                FILE_SHARE_READ,			// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	m_log_file2= CreateFile("c:\\metamachine\\log_file2.txt",			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                FILE_SHARE_READ,			// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	m_log_file3= CreateFile("c:\\metamachine\\project and track updating.txt",			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                FILE_SHARE_READ,			// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	this->SetTimer(1,50,NULL);  //Rapid timer to supply the heart beat for MetaTCP.  Messages from it will clock in the GUI thread.

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMetaMachineDlg::OnDestroy()
{
	this->KillTimer(1);
	CDialog::OnDestroy();
	CloseHandle(m_log_file);
}

void CMetaMachineDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
}

void CMetaMachineDlg::OnClose()
{
	CDialog::OnClose();
}

void CMetaMachineDlg::OnOK()
{
}

void CMetaMachineDlg::OnCancel()
{
	this->ShowWindow(SW_HIDE);
}

BOOL CMetaMachineDlg::DestroyWindow()
{
	return CDialog::DestroyWindow();
}

void CMetaMachineDlg::Log(const char* info)
{
	mv_needed_logs.Add(new LogMsg(info));
}

void CMetaMachineDlg::ServerLog(const char* ip,Buffer2000 &data)
{
	const byte* p_buf=data.GetBufferPtr();
	int buf_len=data.GetLength();
	
	CString output;
	CTime cur_time=CTime::GetCurrentTime();
	output.Format("%s %s - ",ip,cur_time.Format("%H:%M"));
	for(int i=0;i<buf_len;i++){
		if(isalnum(p_buf[i]) || p_buf[i]==' '){
			output+=(char)p_buf[i];
		}
		if(p_buf[i]=='\n')
			output+="\r\n       ";  //indent after carriage return for logging readability so we can associate logs with one particular message visually
	}
	output+="\r\n";

	mv_needed_server_logs.Add(new LogMsg(output));
	//mv_needed_server_logs.Add(new LogMsg(info));
}


UINT CMetaMachineDlg::GetMyID(void)
{
	PHOSTENT hostinfo;
	char this_host[1024];
	UINT ipCount=0;
	char *ip=NULL;
	UINT id;
	if( gethostname ( this_host, sizeof(this_host)) == 0)
	{
		//TRACE("Host name: %s\n", this_host);

		if((hostinfo = gethostbyname(this_host)) != NULL)
		{
			//int nCount = 0;
			if(hostinfo->h_addr_list[ipCount])
			{
				id=*(UINT*)hostinfo->h_addr_list[ipCount];
				ip = inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[ipCount]);
				//memcpy(&my_id,(void*)(),sizeof(my_id));
				//memcpy(&hostIPs[ipCount],(struct in_addr *)hostinfo->h_addr_list[ipCount],sizeof(in_addr));
			}
			else{
				TRACE("MetaMachine Test Failed.  Failure Point:  No local ips?.\n");
				return 0;
			}
		}
		else{
			TRACE("MetaMachine Test Failed.  Failure Point:  gethostbyname().\n");
			return 0;
		}
	}
	else{
		TRACE("MetaMachine Test Failed.  Failure Point:  gethostname().\n");
		return 0;
	}
	return id;
}

void CMetaMachineDlg::OnTimer(UINT nIDEvent)
{
	static int in_timer_count=0;
	static Timer control_update;
	in_timer_count++;
	if(in_timer_count>1)
		return;

	TCPReference tcp;
	if(in_timer_count>1){  //check again just to make sure
		in_timer_count--;
		return;
	}

	tcp.TCP()->Update();  //heartbeat for the whole communications system with the server
	MetaSystemReference system;
	system.System()->Update();
	
	if(mv_needed_logs.Size()>0){
		//WriteToLogFile("Beginning Needed Log Update");
		if(m_returns.GetLineCount()>2000){
			m_returns.SetWindowText("");
		}
		m_returns.SetSel(m_returns.GetTextLength(),-1);
		m_returns.ReplaceSel(((LogMsg*)mv_needed_logs.Get(0))->GetMsg());
		mv_needed_logs.Remove(0);
		//WriteToLogFile("Finished Needed Log Update");
	}
//mv_needed_server_logs

	if(mv_needed_server_logs.Size()>0){
		//WriteToLogFile("Beginning Needed Log Update");
		if(m_server_log.GetLineCount()>3000){
			m_server_log.SetWindowText("");
		}
		m_server_log.SetSel(m_server_log.GetTextLength(),-1);
		m_server_log.ReplaceSel(((LogMsg*)mv_needed_server_logs.Get(0))->GetMsg());
		mv_needed_server_logs.Remove(0);
		//WriteToLogFile("Finished Needed Log Update");
	}

	if(control_update.HasTimedOut(60)){
		//WriteToLogFile("Beginning Control Update");
		CString txt;
		txt.Format("%d",m_failed_queries);
		m_fail_query_count_control.SetWindowText(txt);

		int insert_count=0;
		if(m_lb_failed_queries.GetCount()>1000){
			m_lb_failed_queries.ResetContent();
		}
		while(mv_failed_queries.Size()>0 && insert_count++<200){
			LogMsg* lm=(LogMsg*)mv_failed_queries.Get(0);
			m_lb_failed_queries.InsertString(0,lm->GetMsg());
			mv_failed_queries.Remove(0);
		}

		txt.Format("%d",m_remaining_queries);
		m_s_remaining_queries.SetWindowText(txt);

		MetaSystemReference system;
		bool stat=system.System()->IsMainThreadDead();
		if(stat)
			m_main_thread_alive.SetWindowText("No - FAILURE");
		else
			m_main_thread_alive.SetWindowText("Yes - System Operational");

		control_update.Refresh();
		//WriteToLogFile("Ended Control Update");
	}

	in_timer_count--;

	CDialog::OnTimer(nIDEvent);
}

void CMetaMachineDlg::OnSave()
{
	MetaSystemReference system;
	system.System()->SaveToServer();
}

void CMetaMachineDlg::OnProjectInfoRefresh()
{
	MetaSystemReference system;
	system.System()->UpdateProjectInfoTree(m_project_info);
}

void CMetaMachineDlg::OnRefreshServers()
{
	MetaSystemReference system;
	system.System()->UpdateServerTree(m_server_status_tree);
}

void CMetaMachineDlg::OnBuildSpoofTable()
{
	MetaSystemReference system;
	system.System()->BuildSpoofTable();
}

void CMetaMachineDlg::OnReportResults()
{
	MetaSystemReference system;
	system.System()->ReportAllResults();	
}

void CMetaMachineDlg::AddFailedQuery(const char* query)
{
	mv_failed_queries.Add(new LogMsg(query));
	if(mv_failed_queries.Size()>1000000){
		mv_failed_queries.RemoveRange(0,10000);
	}
	m_failed_queries++;
}

void CMetaMachineDlg::SetRemainingQueries(int queries)
{
	m_remaining_queries=queries;
}

void CMetaMachineDlg::WriteToLogFile(const char* str)
{
	CString cstr;
	CTime cur_time=CTime::GetCurrentTime();
	cstr.Format("%s - %s\r\n",cur_time.Format("%H:%M"),str);
	DWORD tn;
	WriteFile(m_log_file,cstr.GetBuffer(cstr.GetLength()),cstr.GetLength(),&tn,NULL);
}

void CMetaMachineDlg::WriteToLogFile2(const char* str)
{
	CString cstr;
	CTime cur_time=CTime::GetCurrentTime();
	cstr.Format("%s - %s\r\n",cur_time.Format("%H:%M"),str);
	DWORD tn;
	WriteFile(m_log_file2,cstr.GetBuffer(cstr.GetLength()),cstr.GetLength(),&tn,NULL);
}

void CMetaMachineDlg::WriteToLogFile3(const char* str)
{
	CString cstr;
	CTime cur_time=CTime::GetCurrentTime();
	cstr.Format("%s - %s\r\n",cur_time.Format("%H:%M"),str);
	DWORD tn;
	WriteFile(m_log_file3,cstr.GetBuffer(cstr.GetLength()),cstr.GetLength(),&tn,NULL);
}


void CMetaMachineDlg::OnUpdateProjectsDatabase()
{
	MetaSystemReference system;
	system.System()->UpdateProjectsDatabase();  //set a flag to trigger a database update
}

void CMetaMachineDlg::OnBuildSwarmTable()
{
	MetaSystemReference system;
	system.System()->BuildSwarmTable();
}

void CMetaMachineDlg::OnBuildSwarmTable2()
{
	MetaSystemReference system;
	system.System()->BuildSwarmTable();
}

void CMetaMachineDlg::OnBuildFalseDecoyTable()
{
	MetaSystemReference system;
	system.System()->BuildFalseDecoyTable();
}
