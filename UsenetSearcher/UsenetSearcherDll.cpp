#include "StdAfx.h"
#include "UsenetSearcherDll.h"
#include "ProjectDBInterface.h"

//
//
//
UsenetSearcherDll::UsenetSearcherDll(void)
{
	m_server_index = 0;
}

//
//
//
UsenetSearcherDll::~UsenetSearcherDll(void)
{
}

//
//
//
void UsenetSearcherDll::DllInitialize()
{
	m_dlg.Create(IDD_USENET_SEARCHER_DLG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_dlg_hwnd = m_dlg.GetSafeHwnd();
}

//
//
//
void UsenetSearcherDll::DllUnInitialize()
{
	// Kill all timers
	m_dlg.KillTimer(1);	// search & getprojects() timer
	m_dlg.KillTimer(2);	// watchdog

	UINT num_socks =  (UINT)v_usenet_socket_ptrs.size();

	// Socket clean-up and memory de-allocation
	for (UINT i=0; i < num_socks; i++)
	{
		v_usenet_socket_ptrs[i]->Close();
		delete v_usenet_socket_ptrs[i];
	}

	v_usenet_socket_ptrs.clear();

	// Clean up the SMTP Socket
	m_smtp_socket.Close();

	WSocket::Cleanup();
}

//
//
//
void UsenetSearcherDll::DllStart()
{
	WSocket::Startup();

	// Register com
	m_com.Register(this, 6200);
	m_com.InitParent(this);

	m_smtp_socket.InitParent(this);

	// Now create all the searching sockets: (multiple searching servers if so desired)

	// 1st searching server, GIGANEWS:
	UsenetSocket *giganews_socket = new UsenetSocket;
	strcpy(giganews_socket->m_server_name, "news-60.giganews.com");
	strcpy(giganews_socket->m_user, "gn124059");
	strcpy(giganews_socket->m_password, "V3Lrt");
	strcpy(giganews_socket->m_nickname, "giganews");
	giganews_socket->InitParent(this);
	v_usenet_socket_ptrs.push_back(giganews_socket);

	// 2nd searching server, EASYNEWS:
	UsenetSocket *easynews_socket = new UsenetSocket;
	strcpy(easynews_socket->m_server_name, "news.easynews.com");
	strcpy(easynews_socket->m_user, "pauldaman");
	strcpy(easynews_socket->m_password, "ebertsux37");
	strcpy(easynews_socket->m_nickname, "easynews");
	easynews_socket->InitParent(this);
	v_usenet_socket_ptrs.push_back(easynews_socket);

	// 3rd searching server, SUPERNEWS:
	UsenetSocket *supernews_socket = new UsenetSocket;
	strcpy(supernews_socket->m_server_name, "news.supernews.com");
	strcpy(supernews_socket->m_user, "Oct146826");
	strcpy(supernews_socket->m_password, "onthedl");
	strcpy(supernews_socket->m_nickname, "supernews");
	supernews_socket->InitParent(this);
	v_usenet_socket_ptrs.push_back(supernews_socket);

	// Update the dialog
	m_dlg.GetDlgItem(IDC_STATIC_NICKNAME)->SetWindowText(v_usenet_socket_ptrs[m_server_index]->m_nickname);
	m_dlg.GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(v_usenet_socket_ptrs[m_server_index]->m_server_name);
	m_dlg.GetDlgItem(IDC_STATIC_USER)->SetWindowText(v_usenet_socket_ptrs[m_server_index]->m_user);
	m_dlg.GetDlgItem(IDC_STATIC_PASS)->SetWindowText(v_usenet_socket_ptrs[m_server_index]->m_password);

	GetProjects();
	m_last_project_update = CTime::GetCurrentTime();

	m_dlg.SetTimer(1, 1000, NULL);		// search timer
	m_dlg.SetTimer(2, 5000, NULL);		// watchdog
}

//
//
//
void UsenetSearcherDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void UsenetSearcherDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

//
//
//
void UsenetSearcherDll::Log(char *source, char *buf)
{
	char msg[2056];
	memset(msg, 0, 2056);
	sprintf(msg, "(%s) %s", source, buf);
	m_dlg.Log(msg);
}

//
//
//
void UsenetSearcherDll::GetProjects()
{
	v_project_keywords.clear();

	TRACE("USENET SEARCHER: Getting projects...");
	m_dlg.Log("");
	m_dlg.Log("Getting Projects...");

	ProjectDBInterface db;
	db.OpenDBConnection("38.118.160.161", "onsystems", "ebertsux37", "project_management");
	db.ReadProjectKeywords(v_project_keywords);
	db.CloseDBConnection();

	for (UINT i=0; i < v_project_keywords.size(); i++)
	{
		m_dlg.Log((char *)v_project_keywords[i].m_project_name.c_str());
	}

	char buf[256];
	sprintf(buf, "Found %d projects for Usenet Protection.", v_project_keywords.size());
	m_dlg.Log(buf);
	m_dlg.Log("");
}

//
//
//
void UsenetSearcherDll::SwitchServers(char *buf)
{
	v_usenet_socket_ptrs[m_server_index]->m_searching = false;
	v_usenet_socket_ptrs[m_server_index]->v_new_headers.clear();
	v_usenet_socket_ptrs[m_server_index]->Close();	// Close the old connection	

	Log(v_usenet_socket_ptrs[m_server_index]->m_nickname, "");
	Log(v_usenet_socket_ptrs[m_server_index]->m_nickname, buf);
	Log(v_usenet_socket_ptrs[m_server_index]->m_nickname, "Switching servers...");
	Log(v_usenet_socket_ptrs[m_server_index]->m_nickname, "");

	if (m_server_index < (int)v_usenet_socket_ptrs.size()-1)
	{		
		m_server_index++;	// get the next one
	}
	else
	{
		m_server_index = 0;
		char email_msg[2048];
		sprintf(email_msg, "*** USENET SEARCHER NOTIFICATION ***\n\n*** CRUCIAL ERROR - OUT OF SEARCHING SERVERS ***\n\nSearching servers are hard-coded into the UsenetSearcher project.\n\nIn the file: 'UsenetSearcherDll.cpp' in DllStart()");
		m_smtp_socket.SendMail(email_msg);
	}

	m_dlg.GetDlgItem(IDC_STATIC_NICKNAME)->SetWindowText(v_usenet_socket_ptrs[m_server_index]->m_nickname);
	m_dlg.GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(v_usenet_socket_ptrs[m_server_index]->m_server_name);
	m_dlg.GetDlgItem(IDC_STATIC_USER)->SetWindowText(v_usenet_socket_ptrs[m_server_index]->m_user);
	m_dlg.GetDlgItem(IDC_STATIC_PASS)->SetWindowText(v_usenet_socket_ptrs[m_server_index]->m_password);
}