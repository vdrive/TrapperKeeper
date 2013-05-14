// SamplePlugInDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DCScannerDlg.h"
#include "DllInterface.h"


// DCScannerDlg dialog

IMPLEMENT_DYNAMIC(DCScannerDlg, CDialog)
DCScannerDlg::DCScannerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(DCScannerDlg::IDD, pParent)
{
}

DCScannerDlg::~DCScannerDlg()
{
	this->OnDestroy();
	//this->DisconnectAll();
}

void DCScannerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, StatusListBox);
	DDX_Control(pDX, IDC_CONNECTED_HUBS, m_connected_hubs);
	DDX_Control(pDX, IDC_EDIT1, spoofs_sent_text);
	DDX_Control(pDX, IDC_EDIT2, m_active_projects);
}


BEGIN_MESSAGE_MAP(DCScannerDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()

BOOL DCScannerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//initialize the hub list collector's window
	m_listcollector.InitDlg(this);
	m_listcollector.DownloadHubList();	//downloads the hublist
	m_hublist_position=0;
	//v_spoof_collector.clear();		//clear the vector that contains projects that have been entered in db
	//initialize the hub sockets
	this->StatusUpdate("Trying to connect to database...");
	if(!db_connection.Connect("38.119.66.81","root","","mediadefender"))
	{
		::AfxMessageBox("Error: Could not create database connection. Check Database!");
	}
	else
	{
		this->StatusUpdate("Successfully connected to database");
		m_date = db_connection.GetDate();		//gets current date
	}
	for(int i=0;i<SCANNERS;i++)
	{
		hubsocket[i].InitDlg(this);		//give each client access to dialog
		hubsocket[i].GetDatabaseObject(db_connection);		//give each client access to the database
		hubsocket[i].GetDownloadManager(m_download_manager);
		//hubsocket[i].GetProjectData(m_keywords_vector);		//get the project data check whether to spoof
	}
	b_projects_changed=true;
	SetTimer(0,10000,NULL);				//ten seconds
	SetTimer(4,7000,NULL);				//seven seconds
	SetTimer(2,60*1000*60*24,NULL);		//one day
	SetTimer(3,60*1000*60,NULL);		//one hour
	return TRUE;
}
// DCScannerDlg message handlers
void DCScannerDlg::OnCancel()		// user hit ESC
{
//	CDialog::OnCancel();
//	OnExit();
}

//
//
//
void DCScannerDlg::OnClose()
{
	ShowWindow(SW_HIDE);	
}

//prints out the status to the listbox
void DCScannerDlg::StatusUpdate(char *post_data)
{
	StatusListBox.InsertString(0,post_data);
}

//hub list has been received
void DCScannerDlg::ReceivedHubList(vector<Hub> hub_list)
{
	this->StatusUpdate("List Received");
	this->StatusUpdate("Starting Hub Scanning...");
	m_hub_list=hub_list;
}
//
//
//

//Timer that is called every 5 seconds to do status checks on hub sockets
void DCScannerDlg::OnTimer(UINT nIDEvent)
{
	//every 7 seconds
	if(nIDEvent==4)
	{
		//send junk to all connected clients downloading
		m_download_manager.SendJunk();
		return;
	}
	
	int x=0;
	m_hubs_connected=0;
	//every day
	if(nIDEvent==2 || b_projects_changed)	//if projects have changed get the new vector
	{
		if(p_pm->HaveProjectsChanged())
		{
			m_keywords_vector=&p_pm->m_project_keywords_vector;
			for(int i=0;i<SCANNERS;i++)
			{
				hubsocket[i].GetProjectData(m_keywords_vector);		//get the project data check whether to spoof
			}
		}
		//v_spoof_collector.clear();	//clear vector of projects
		this->DisconnectAll();
		b_projects_changed=false;
		return;
	}
	//every hour
	if(nIDEvent==3)
	{
		//v_spoof_collector.clear();	//clear the spoof collector vector every hour!
		//check whether to insert current data into the dcdata database
		if(strcmp(m_date.c_str(),db_connection.GetDate().c_str())!=0)
		{
			m_date = db_connection.GetDate();
			db_connection.InsertDCMasterData();		//insert data into dc master
		}
		StatusListBox.ResetContent();	//reset listbox window
		return;
	}
	if(m_keywords_vector->v_projects.size()>0)
	{
		while(x<SCANNERS && m_hublist_position<(signed)m_hub_list.size())
		{
			m_current_hub=m_hub_list[m_hublist_position];
			//set a default port number to 411
			if(!hubsocket[x].IsSocket())
			{
				hubsocket[x].Close();
				//hubsocket[x].HubConnect(m_current_hub.m_address,m_current_hub.m_port);	
				hubsocket[x].HubConnect("38.119.66.29",411);
				m_hublist_position++;	//increment position in list
			}
			else
			{
				m_hubs_connected++;	//increment the number of hubs connected to
			}
		x++;	//increment hubsocket
		}
	}

	if(m_hublist_position == (signed)m_hub_list.size())
	{
		m_hublist_position = 0;	//reset the hub list position
	}
	//memset(m_buffer,0,sizeof(m_buffer));
	strcpy(m_buffer,"");
	m_connected_hubs.SetWindowText(itoa(m_hubs_connected,m_buffer,10));
	m_active_projects.SetWindowText(itoa((int)m_keywords_vector->v_projects.size(),m_buffer,10));
	CDialog::OnTimer(nIDEvent);
}

void DCScannerDlg::SetProjectManager(ProjectManager &pm)
{
	p_pm=&pm;	//set project manager
}

void DCScannerDlg::OnDestroy()
{
	this->DisconnectAll();
	KillTimer(0);
	KillTimer(2);
	db_connection.Disconnect();	//disconnect from db
	CDialog::OnDestroy();
}

//
//increments the number of spoofs displayed in dialog window
void DCScannerDlg::IncrementSpoofs(int increment)
{
	char *spoofs = new char[50];
	m_spoofs_sent += increment;
	this->spoofs_sent_text.SetWindowText(itoa(m_spoofs_sent,spoofs,10));
	delete [] spoofs;
}


//disconnects every hub...resets all connections
void DCScannerDlg::DisconnectAll()
{
	this->StatusUpdate("Disconnecting from all hubs...");
	for(int i=0;i<SCANNERS;i++)
	{
		hubsocket[i].Close();
		hubsocket[i].Startup();
	}
	this->StatusUpdate("Disconnected from all hubs complete!");
}
//reset hub connections
void DCScannerDlg::OnBnClickedButton1()
{
	m_download_manager.AddPassiveDownload();
	//this->DisconnectAll();
}
