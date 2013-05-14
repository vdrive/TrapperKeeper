// SLSKSpooferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SLSKSpoofer.h"
#include "SLSKSpooferDlg.h"
#include ".\slskspooferdlg.h"
//#include "ProjectKeywordsVector.h"
//#include "SLSKController.h"
//#include "TrackInformation.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSLSKSpooferDlg dialog


//IMPLEMENT_DYNAMIC(CSLSKSpooferDlg, CDialog)
CSLSKSpooferDlg::CSLSKSpooferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSLSKSpooferDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

void CSLSKSpooferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_EDIT1, un_edit_box);
	//	DDX_Control(pDX, IDC_EDIT2, pass_edit_box);
	//	DDX_Control(pDX, IDC_EDIT3, port_edit_box);
	//DDX_Control(pDX, IDC_RICHEDIT22, status_edit_box);
	//	DDX_Control(pDX, IDC_EDIT4, noc);
	DDX_Control(pDX, IDC_LIST1, m_log_list);
	DDX_Control(pDX, IDC_EDIT5, m_server_status);
	DDX_Control(pDX, IDC_EDIT6, m_peer_status);
	DDX_Control(pDX, IDC_EDIT7, m_parent_status);
	DDX_Control(pDX, IDC_LIST2, m_project_log);
	DDX_Control(pDX, IDC_EDIT8, m_num_parents_connecting);
	DDX_Control(pDX, IDC_EDIT9, m_num_peer_connecting);
	DDX_Control(pDX, IDC_EDIT10, m_num_server_mods);
	DDX_Control(pDX, IDC_EDIT11, m_num_parent_mods);
	DDX_Control(pDX, IDC_EDIT12, m_num_peer_mods);
}

BEGIN_MESSAGE_MAP(CSLSKSpooferDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SLSK_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_EN_CHANGE(IDC_RICHEDIT22, OnEnChangeRichedit22)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_Update, OnBnClickedUpdate)
	ON_LBN_SELCHANGE(IDC_LIST1, OnLbnSelchangeList1)
END_MESSAGE_MAP()


// CSLSKSpooferDlg message handlers

BOOL CSLSKSpooferDlg::OnInitDialog()
{
	
	CDialog::OnInitDialog();
//	sm_instance = this;

	m_controller.InitDialog(this);
	//p_parent->InitDialog(this);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

//	account_info.Init();
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSLSKSpooferDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSLSKSpooferDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSLSKSpooferDlg::OnBnClickedButton1()
{
	

	// TODO: Add your control notification handler code here
}



void CSLSKSpooferDlg::OnBnClickedButton2()
{
	m_controller.stop();
	// TODO: Add your control notification handler code here
}

void CSLSKSpooferDlg::OnEnChangeRichedit22()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}



void CSLSKSpooferDlg::FillProjectsListView(void)
{
	m_project_log.DeleteString(m_project_log.GetCount());
	ProjectKeywords *proj;

	for (int i = 0; i < (int)m_projects.v_projects.size(); i++)
	{
		proj = &m_projects.v_projects[i];
		LogProjects(proj->m_project_name.c_str());
	}
}
void CSLSKSpooferDlg::OnTimer(UINT nIDEvent)
{
//	char* tempstr;

	

	if(nIDEvent == 1) 
	{
		//TRACE("EVENT 1 TRIGGERED\n");
//		Log("Event 1 triggered!!!");
		if(m_project_manager.IsReady())
		{
			//TRACE("IS READY!!\n");
		//	Log("Is Ready!!!");
//int size2 = (int)m_projects.v_projects.size();
			if(m_project_manager.HaveProjectsChanged()) 
			{
				ifstream fin("C:\\slsksettings.txt", ios::in);
				fin >> m_modnumber;
				fin.close();
			/*	ofstream cin1("C:\\spoofing.txt", ios::out);
				ofstream cin2("C:\\demand.txt", ios::out);
				ofstream cin3("C:\\supply.txt", ios::out);
				ofstream cin4("C:\\decoying.txt", ios::out);*/
				//TRACE("PROJECT CHANGE!!!\n");
			//	char* tempchar = new char[(int)strlen("Getting Projects!")];
				//strcpy(tempchar, "Getting Projects!");
			//	Log("Getting Projects!");
				m_project_manager.GetProjectsCopy(m_projects);
				int size = (int)m_projects.v_projects.size();
				for(int i = 0; i < size; i++) 
				{
					//TRACE("for loop\n");
					ProjectKeywords keyword = m_projects.v_projects[i];

					if(keyword.m_soulseek_spoofing_enabled && i % 4 == m_modnumber)// || keyword.m_soulseek_demand_enabled || keyword.m_soulseek_supply_enabled) 
					{  
						
					//	TrackInformation *ti = TrackInformation::GetTrackInformation(keyword.m_project_name);
					//	m_track_info.push_back(ti);
					//	cin1 << i << " " << keyword.m_project_name.c_str() << endl;
						LogProjects((const char*)keyword.m_project_name.c_str());
					//TRACE("\n");
					
					}

					/*if (keyword.m_soulseek_demand_enabled)
					{
						cin2 << i << " "<<  keyword.m_project_name.c_str() << endl;

					}

					if (keyword.m_soulseek_supply_enabled)
					{
						cin3 << i << " " << keyword.m_project_name.c_str()<< endl;
					}

					if (keyword.m_soulseek_decoying_enabled)
					{
						cin4 << i << " " << keyword.m_project_name.c_str() << endl;
					}*/
						//TRACE("dont with loop \n");
				//m_projects_changed = true;
				}
					//TRACE("end project changed\n");
					//cin1.close();
					//cin2.close();
					//cin3.close();
					//cin4.close();
					
					if (m_started_spoofing == false)
					{
			
						int size2 = (int)m_projects.v_projects.size();
				//		FillProjectsListView();
						m_controller.StartSpoofing();
						m_started_spoofing = true;
					}
			}
	

		
		}
	}
/*	else if(nIDEvent == 3) {
		if(m_project_manager.IsReady() && m_project_manager.HaveProjectsChanged()) {
			for (int i = 0; i < (int)m_track_info.size(); i++)
				delete m_track_info[i];
			m_track_info.clear();
			
			
			m_project_manager.GetProjectsCopy(m_projects);
			int size = (int)m_projects.v_projects.size();
			for(i = 0; i < size; i++) {
				ProjectKeywords keyword = m_projects.v_projects[i];
				if(keyword.m_soulseek_spoofing_enabled) {  // change these to soulseek
					TrackInformation *ti = TrackInformation::GetTrackInformation(keyword.m_project_name);
					m_track_info.push_back(ti);
					m_project_log.DeleteString(m_project_log.GetCount()-1);
					LogProjects(keyword.m_project_name.c_str());
					//TRACE("\n");
				}
				m_projects_changed = true;
			}
		}
	}*/
	//if(nIDEvent == 7)
	//{
	//	m_project_status_list.WriteFakeSuppliesToDB();
	//}
	//else
	//{
//		parent->OnTimer(nIDEvent);
	//if(nIDEvent == 4) {
		
		
//	}	
	//}
	m_controller.OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

void CSLSKSpooferDlg::Log(const char* log)
{

	CString msg = (CTime::GetCurrentTime()).Format("%Y-%m-%d %H:%M:%S - ");
	msg += log;
	if(m_log_list.GetCount()>5000)
	{
		m_log_list.DeleteString(m_log_list.GetCount()-1);
	}
	m_log_list.InsertString(0,msg);
	//delete [] log;
}

void CSLSKSpooferDlg::LogProjects(const char* log)
{
	CString msg = (CTime::GetCurrentTime()).Format("%Y-%m-%d %H:%M:%S - ");
	msg += log;
	if(m_project_log.GetCount()>5000)
	{
		m_project_log.DeleteString(m_project_log.GetCount()-1);
	}
	m_project_log.InsertString(0,msg);
	//delete [] log;
}

void CSLSKSpooferDlg::setServerStatus(const char* status)
{
	m_server_status.SetWindowText(status);
	delete [] status;
}

void CSLSKSpooferDlg::setParentStatus(const char* status)
{
	m_parent_status.SetWindowText(status);
	delete [] status;
}

void CSLSKSpooferDlg::setPeerStatus(const char* status)
{
	m_peer_status.SetWindowText(status);
	delete [] status;
}

void CSLSKSpooferDlg::setPeerConnectingStatus(const char* status)
{
	m_num_peer_connecting.SetWindowText(status);
	delete [] status;
}

void CSLSKSpooferDlg::setParentConnectingStatus(const char* status)
{
	m_num_parents_connecting.SetWindowText(status);
	delete [] status;
}

void CSLSKSpooferDlg::setParentModStatus(const char* status)
{
	m_num_parent_mods.SetWindowText(status);
	delete [] status;
}

void CSLSKSpooferDlg::setPeerModStatus(const char* status)
{
	m_num_peer_mods.SetWindowText(status);
	delete [] status;
}

void CSLSKSpooferDlg::setServerModStatus(const char* status)
{
	m_num_server_mods.SetWindowText(status);
	delete [] status;
}

//void CSLSKSpooferDlg::InitParent(SLSKController *parent)
//{
//	p_parent = parent;
//	p_parent->InitDialog(this);
//}
void CSLSKSpooferDlg::OnBnClickedButton3()
{
	m_controller.AddServerConnection();
}

void CSLSKSpooferDlg::OnBnClickedUpdate()
{
	m_controller.UpdateProjects();
}

void CSLSKSpooferDlg::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
}
