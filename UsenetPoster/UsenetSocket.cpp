#include "StdAfx.h"
#include "usenetsocket.h"
#include "UsenetPosterDll.h"
#include "mysql.h"
#include "Shlwapi.h"	// StrStrI()

UsenetSocket::UsenetSocket(void)
{
	memset(&m_server_name, 0, sizeof(m_server_name));
	memset(&m_user, 0, sizeof(m_user));
	memset(&m_password, 0, sizeof(m_password));
	memset(&m_previous_buffer, 0, sizeof(m_previous_buffer));

	v_headers.clear();
	m_last_server_response_time = NULL;
	m_port = 119; // default for all NNTP servers
	m_is_active = true;
	m_posting = false;
}

UsenetSocket::~UsenetSocket(void)
{
}

void UsenetSocket::InitParent(UsenetPosterDll *dll)
{
	p_dll = dll;
}

//
//
//
void UsenetSocket::OnConnect(int error_code)
{
	m_last_server_response_time = CTime::GetCurrentTime();

	if(error_code == 0)
	{
		char str[1024];
		sprintf(str, "** Connected. Sending username and password...(%s/%s)", m_user, m_password);
		p_dll->Log(m_nickname, " "); p_dll->Log(m_nickname, str); p_dll->Log(m_nickname, " ");

		int ret;
		unsigned char *msg = new unsigned char[1024];

		// Send username
		sprintf((char *)msg, "AUTHINFO USER %s\r\n", m_user);
		ret = SendSocketData(msg, (UINT)strlen((const char *)msg));

		// Send password
		sprintf((char *)msg, "AUTHINFO PASS %s\r\n", m_password);
		ret = SendSocketData(msg, (UINT)strlen((const char *)msg));

		delete msg;
	}
	else
	{
		char msg[256];
		strcpy(msg, "***Couldn't make a connection. Error: ");
		char err[8];
		itoa(error_code, err, 10);
		strcat(msg, err);
		strcat(msg, "\n");
		TRACE(msg);
		p_dll->Log(m_nickname, msg);
		
		Close();
		m_posting = false;
	}
}

//
//
//
void UsenetSocket::OnClose(int error_code)
{
	bool closed = true;
}

//
//	Gets called when there is socket data to be read.
//
void UsenetSocket::OnReceive(int error_code)
{
	//error checking
	if(error_code!=0)
	{
		TRACE("Error Receiving Data\n");
		return;	//return due to error
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return;
	}

	ReceiveSomeSocketData(4096);

	char buf[1];
	memset(buf,0,sizeof(buf));
	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=1;

	DWORD num_read=0;
	DWORD flags=MSG_PEEK;
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(ret!=SOCKET_ERROR)
	{
		if(num_read!=0)
		{
			OnReceive(0);	// recursive call
		}
	}
}

void UsenetSocket::SomeSocketDataReceived(char *data, unsigned int data_len, unsigned int new_len, unsigned int max_len)
{
	char buf[8192];
	int buf_pos = 0;
	ZeroMemory(&buf, 8192);

	if(strlen(m_previous_buffer) != 0)
	{
		strcpy(buf, m_previous_buffer);
		buf_pos = (int)strlen(m_previous_buffer);
	}

	for (UINT x = 0; x < data_len; x++)
    {
	    buf[buf_pos] = data[x];
		if(x == data_len-1 && data[x] != '\n' && data[x-1] != '\r')
		{
			strcpy(m_previous_buffer, buf);
		}
		else
		{
			::ZeroMemory(&m_previous_buffer, sizeof(m_previous_buffer));
		}
  
		buf_pos++;
        if (data[x] == '\r')
           buf[buf_pos-1] = 0;
        if (data[x] == '\n')
        {
           buf[buf_pos-1] = 0;
           Parse(buf);
           ZeroMemory(&buf, 8192);
           buf_pos = 0;
        }
     }
	m_previous_buffer_len = data_len;
}

//
//
//
void UsenetSocket::Parse(char *buf)
{
	m_last_server_response_time = CTime::GetCurrentTime();

	// First get the op code
	int op_code = 0;
	sscanf(buf, "%d", &op_code);

	switch (op_code)
	{
	case 200:
	case 201:
		{
			// Connected... start sending posts
			break;
		}
	case 240:
		{
			//Post OK
			LogPost();
			DoNextPost();
			break;
		}
	case 281:	
		{
			// Authentication succeeded, start posting
			SendPostCommand();
			break;
		}
	case 340:
		{
			// Posting authorized
			SendNextPost();
			break;
		}
	case 381:	
		{
			// More authentication needed
			bool we_are_here = true;
			break;
		}
	case 411:
		{
			// No such group
			bool we_are_here = true;
			break;
		}
	case 440:
		{
			// Posting not allowed
			p_dll->ServerBanned(this, buf);
			p_dll->Log(m_nickname, buf);
			break;
		}
	case 441:
		{
			// Posting failed
			if (StrStrI(buf, "header not in internet syntax")!=NULL || StrStrI(buf, "must customize your From:")!=NULL)
			{
				p_dll->Log(m_nickname, "Encountered a posting problem.  Skipping this post.");
				DoNextPost();
			}
			else
				p_dll->ServerBanned(this, buf);

			p_dll->Log(m_nickname, buf);
			break;
		}
	case 502:
		{
			// access restriction or permission denied
			p_dll->ServerBanned(this, buf);
			p_dll->Log(m_nickname, buf);
			break;
		}
	case 503:
		{
			// Timeout / Group temporarily down
			p_dll->Log(m_nickname, buf);
			p_dll->Log(m_nickname, "Encountered a posting problem.  Skipping this post.");
			DoNextPost();
			break;
		}
	default:
		{
			bool unknown_response = true;
			m_posting = false;
			v_headers.clear();
			break;
		}
	}
}

void UsenetSocket::DoNextPost()
{
	vector<Header>::iterator iter = v_headers.begin();

	if (v_headers.size() == 0)
	{
		bool how_did_this_happen = true;
		v_headers.clear();
	}
	else
	{
		v_headers.erase(iter);
	}

	if (v_headers.size() > 0)
	{
		// Change the displayed queue size
		LVFINDINFO info;
		info.flags = LVFI_STRING;
		info.psz = m_nickname;
		int nIndex= p_dll->m_dlg.m_servers_list.FindItem(&info);

		char str[128];
		itoa((int)v_headers.size(), str, 10);
		p_dll->m_dlg.m_servers_list.SetItemText(nIndex, 1, str);

		SendPostCommand();
	}
	else
	{
		m_posting = false;
		v_headers.clear();

		// Set the queue size to 0
		LVFINDINFO info;
		info.flags = LVFI_STRING;
		info.psz = m_nickname;
		int nIndex= p_dll->m_dlg.m_servers_list.FindItem(&info);
		p_dll->m_dlg.m_servers_list.SetItemText(nIndex, 1, "0");

		char msg[1024];
		strcpy(msg, "*** Finished posting.  Waiting for more headers from the searcher...");
		p_dll->Log(m_nickname, ""); p_dll->Log(m_nickname, msg); p_dll->Log(m_nickname, "");
		Close();
	}
}

//
//
//
void UsenetSocket::SendPostCommand()
{
	unsigned char *msg = new unsigned char[256];
	sprintf((char *)msg, "POST %s\r\n", "");
	int ret = SendSocketData(msg, (UINT)strlen((const char *)msg));
	delete msg;
}

//
//
//
void UsenetSocket::SendNextPost()
{
	vector<Header>::iterator iter = v_headers.begin();

	CString from = iter->m_from;
	CString group = iter->m_group;
	CString subject = iter->m_subject;

	int lines = iter->m_lines;

	char msg[2048];
	sprintf(msg, "POSTING %s", subject);
	p_dll->Log(m_nickname, msg);

	unsigned char *buffer = new unsigned char[1024+1];
	memset(buffer, 0, 1024+1);

	int linenumber = 0;
	int charnumber = 0;

	//From
	sprintf((char *)buffer, "From: %s\r\n", from);
	SendSocketData(buffer, (UINT)strlen((const char *)buffer));

	//Group
	sprintf((char *)buffer, "Newsgroups: %s\r\n", group);
	SendSocketData(buffer, (UINT)strlen((const char *)buffer));

	//Subject
	sprintf((char *)buffer, "Subject: %s\r\n", subject);
	SendSocketData(buffer, (UINT)strlen((const char *)buffer));

/*
	// Cancel Headers
	strcpy(buffer,"Control: cancel ");
	strcat(buffer,message_for_post.m_message_id);
	strcat(buffer,"\r\n");
	SendData(buffer, lstrlen(buffer));

	// NNTP-Posting-Date:
	strcpy(buffer,"NNTP-Posting-Date: ");
	strcat(buffer,message_for_post.m_nntp_posting_date);
	strcat(buffer,"\r\n");
	SendData(buffer, lstrlen(buffer));

	// Date:
	strcpy(buffer, "Date: ");
	strcat(buffer, message_for_post.m_post_date);
	strcat(buffer,"\r\n");
	SendData(buffer, lstrlen(buffer));

	//Message ID (if necessary)
	if (m_next_message_id != "")
	{
		strcpy(buffer,"Message-ID: ");
		strcat(buffer,m_next_message_id);
		strcat(buffer,"\r\rn");
		SendData(buffer, lstrlen(buffer));
	}

	// Message ID
	CString new_id = message_for_post.m_message_id;
	new_id.Replace("<","<cancel.");
	strcpy(buffer, "Message-ID: ");
	strcat(buffer, new_id);
	strcat(buffer,"\r\n");
	SendData(buffer, lstrlen(buffer));

	// X-Cancelled-by:
	strcpy(buffer, "X-Cancelled-by: ");
	strcat(buffer, message_for_post.m_from); 
	strcat(buffer, "\r\n");
	SendData(buffer, lstrlen(buffer));
*/

	//Message Body
	for (int i=0; i < lines; i++)
	{
		sprintf((char *)buffer,"\r\n");
		SendSocketData(buffer, (UINT)strlen((const char *)buffer));
		sprintf((char *)buffer, "MLSKDFJALRKJFFNGVIQERUGPQOIEURDJFLKNOPAERHGDFJNVADLKVJADLFGOPAI");
	}
/*
	srand( (unsigned)time( NULL ) );
	for (linenumber=1; linenumber < message_for_post.m_lines; linenumber++)
	{
		ZeroMemory(&buffer, sizeof(buffer));
		buffer[0] = 'M';	// Each line always begins with an 'M'
		for (charnumber=1; charnumber < 60; charnumber ++)
		{
			char SomeRandomChar = (char)((rand()%66)+33);	// Put the ASCII Code in the range [33,96]
			buffer[charnumber] = SomeRandomChar;
		}
		strcat(buffer,"\r\n");
		SendData(buffer, lstrlen(buffer));
	}
*/
	//End Message
	
	sprintf((char *)buffer,"\r\n.\r\n");
	SendSocketData(buffer, (UINT)strlen((const char *)buffer));

	delete [] buffer;
}

//
//
//
void ReplaceInvalidCharacters(CString *cstring)
{
	cstring->Replace("\\","\\\\");
	cstring->Replace("'","\\'");
}

//
//
//
void UsenetSocket::LogPost()
{
	vector<Header>::iterator iter = v_headers.begin();

	MYSQL *conn;
	conn=mysql_init(NULL);
	if (mysql_real_connect(conn,"38.118.160.161","onsystems","ebertsux37","dcdata",0,NULL,0) == NULL)
	{
		char msg[1024];
		sprintf(msg,"(Failed to connect to database: %s", mysql_error(conn));
		p_dll->Log(m_nickname, msg);
	}
	else
	{
		char query[2048];
		memset(query, 0, 2048);

		CString subject_no_parts = iter->m_subject;
		int index = subject_no_parts.ReverseFind('(');

		if (index > 0)
			subject_no_parts = subject_no_parts.Left(index-1);

		ReplaceInvalidCharacters(&subject_no_parts);

		sprintf(query, "UPDATE usenet_protection SET counter_post_server = '%s', counter_post_time = now() WHERE subject like '%s%%'", m_nickname, subject_no_parts);

		int ret = mysql_query(conn, query);
		if (ret != 0)	// check for error
			TRACE(mysql_error(conn));

		mysql_close(conn);
	}
}