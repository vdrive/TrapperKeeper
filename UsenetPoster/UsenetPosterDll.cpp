#include "StdAfx.h"
#include "UsenetPosterDll.h"

//
//
//
UsenetPosterDll::UsenetPosterDll(void)
{
}

//
//
//
UsenetPosterDll::~UsenetPosterDll(void)
{
}

//
//
//
void UsenetPosterDll::DllInitialize()
{
	m_dlg.Create(IDD_USENET_POSTER_DLG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
}

//
//
//
void UsenetPosterDll::DllUnInitialize()
{
	m_dlg.KillTimer(1);	// watchdog timer

	UINT num_socks = (UINT)v_usenet_socket_ptrs.size();

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
void UsenetPosterDll::DllStart()
{
	WSocket::Startup();

	m_com.Register(this, 6200);	// For communicating with the UsenetSearcher
	m_com.InitParent(this);

	m_dlg.m_servers_list.SetExtendedStyle(m_dlg.m_servers_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_dlg.m_servers_list.InsertColumn(0,"Nickname",LVCFMT_LEFT, 100);
	m_dlg.m_servers_list.InsertColumn(1,"Queue Size",LVCFMT_CENTER, 70);
	m_dlg.m_servers_list.InsertColumn(2,"Server Address",LVCFMT_LEFT, 200);
	m_dlg.m_servers_list.InsertColumn(3,"Username",LVCFMT_LEFT, 100);
	m_dlg.m_servers_list.InsertColumn(4,"Password",LVCFMT_LEFT, 100);
	m_dlg.m_servers_list.InsertColumn(5,"Last Error Received",LVCFMT_LEFT, 450);

	// Now create all the searching sockets: (multiple searching servers if so desired)
	GetServerAccounts();

	m_smtp_socket.InitParent(this);

	m_dlg.SetTimer(1, 1000*60, NULL);	// watchdog timer
}

//
//
//
void UsenetPosterDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void UsenetPosterDll::Log(char *log_source, char *buf)
{
	char msg[2048];
	sprintf(msg, "(%s) %s", log_source, buf);
	m_dlg.Log(msg);
}

//
//
//
void UsenetPosterDll::StartPosting(UsenetSocket *socket_ptr)
{	
	socket_ptr->m_posting = true;
	socket_ptr->m_last_server_response_time = CTime::GetCurrentTime();

	int port = socket_ptr->m_port;

	char server_name[1024];
	strcpy(server_name, socket_ptr->m_server_name);

	int ret = 0;
	ret = socket_ptr->Close();	// just in case
	ret = socket_ptr->Create();
	ret = socket_ptr->Connect(server_name, port);
}

//
//
//
void UsenetPosterDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

//
//	Only call durring DLL start.  Only call once.
//
void UsenetPosterDll::GetServerAccounts()
{
	CStdioFile file;
	CString line;

	m_dlg.m_servers_list.DeleteAllItems();

	if (file.Open("posting_servers.txt", CFile::modeRead, NULL) != 0)
	{
		// opened successfully
		while (file.ReadString(line))
		{
			if (line.GetAt(0) == '/' || line.GetAt(0) == ' ')
				continue;

			char nickname[256];
			char server_name[256];
			char user[256];
			char password[256];

			int ret = sscanf(line, "%s %s %s %s", nickname, server_name, user, password);

			if (ret == 4)
			{
				int inserted = m_dlg.m_servers_list.InsertItem(m_dlg.m_servers_list.GetItemCount(),nickname);
				m_dlg.m_servers_list.SetItemText(inserted, 1, "0");
				m_dlg.m_servers_list.SetItemText(inserted, 2, server_name);
				m_dlg.m_servers_list.SetItemText(inserted, 3, user);
				m_dlg.m_servers_list.SetItemText(inserted, 4, password);

				UsenetSocket *usenet_socket = new UsenetSocket;
				strcpy(usenet_socket->m_nickname, nickname);
				strcpy(usenet_socket->m_server_name, server_name);
				strcpy(usenet_socket->m_user, user);
				strcpy(usenet_socket->m_password, password);
				usenet_socket->InitParent(this);
				v_usenet_socket_ptrs.push_back(usenet_socket);
			}
		}

		file.Close();
	}
	else
	{
		MessageBox(NULL, "Error opening the server list.\nThe file 'posting_server.txt' should be in the Trapper Keeper directory.","File Open Error", MB_OK);
	}
}

//
//
//
void UsenetPosterDll::ServerBanned(UsenetSocket *bad_socket_ptr, char *buf)
{
	char nickname[256];
	strcpy(nickname, bad_socket_ptr->m_nickname);

	bad_socket_ptr->Close();
	bad_socket_ptr->m_is_active = false;

	LVFINDINFO info;
	info.flags = LVFI_STRING;
	info.psz = nickname;
	int nIndex= m_dlg.m_servers_list.FindItem(&info);

	CString error_msg = CTime::GetCurrentTime().Format("%H:%m:%S");
	error_msg += " - ";
	error_msg += buf;
	m_dlg.m_servers_list.SetItemText(nIndex, 5, error_msg);

	// Find which server has the lowest posting load:
	UsenetSocket *good_socket_ptr = NULL;
	int low_load = 999999999;
	for (int index=0; index < (int)v_usenet_socket_ptrs.size(); index++)
	{
		if (v_usenet_socket_ptrs[index]->m_is_active == false)
			continue;
		
		int this_load = (int)v_usenet_socket_ptrs[index]->v_headers.size();

		if (this_load < low_load)
		{
			good_socket_ptr = v_usenet_socket_ptrs[index];
			low_load = this_load;
		}
	}

	if (good_socket_ptr == NULL)
	{
		char msg[256];
		memset(msg, 0, 256);
		sprintf(msg, "*** CRITICAL USENET NOTIFICATION ***\n\nBanned on all servers, new accounts needed!"); 
		m_smtp_socket.SendMail(msg);
		MessageBox(NULL, "Banned on all servers", "Need to get new accounts.", MB_OK);
		return;
	}

	// Unload headers to an active server with the lowest load:
	char message[1024];
	sprintf(message, "BANNED - Unloading %d headers to (%s)", bad_socket_ptr->v_headers.size(), good_socket_ptr->m_nickname);
	Log(nickname, message);

	char email_msg[1024];
	memset(email_msg, 0, 1024);
	sprintf(email_msg, "*** USENET NOTIFICATION ***\n\nThe '%s' usenet account has been banned.\nSwitching to %s.", bad_socket_ptr->m_nickname, good_socket_ptr->m_nickname);
	m_smtp_socket.SendMail(email_msg);

	// Push each header individually onto vector for posting
	vector<Header>::iterator iter = bad_socket_ptr->v_headers.begin();
	while (iter != bad_socket_ptr->v_headers.end())
	{
		good_socket_ptr->v_headers.push_back(*iter);
		bad_socket_ptr->v_headers.erase(iter);

		// Change the displayed queue size
		int nIndex;
		char str[128];
		LVFINDINFO info;

		// The good server (should increment):
		info.flags = LVFI_STRING;
		info.psz = good_socket_ptr->m_nickname;
		nIndex= m_dlg.m_servers_list.FindItem(&info);
		itoa((int)good_socket_ptr->v_headers.size(), str, 10);
		m_dlg.m_servers_list.SetItemText(nIndex, 1, str);

		// The bad server (should decrement to zero):
		info.flags = LVFI_STRING;
		info.psz = bad_socket_ptr->m_nickname;
		nIndex= m_dlg.m_servers_list.FindItem(&info);
		itoa((int)bad_socket_ptr->v_headers.size(), str, 10);
		m_dlg.m_servers_list.SetItemText(nIndex, 1, str);
	}
	
	// If this server is not posting yet, get it going... otherwise just let it keep going.
	if (good_socket_ptr->v_headers.size() > 0 && good_socket_ptr->m_posting == false)
	{
		StartPosting(good_socket_ptr);
	}

	//
	// Send error report via email
	//
}