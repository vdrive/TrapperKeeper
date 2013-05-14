#include "StdAfx.h"
#include "usenetsocket.h"
#include "Newsgroup.h"
#include "UsenetSearcherDll.h"
#include "ThreadData.h"

UsenetSocket::UsenetSocket(void)
{
	v_my_projects.clear();
	v_newsgroups.clear();

	memset(m_server_name, 0, sizeof(m_server_name));
	memset(m_user, 0, sizeof(m_user));
	memset(m_password, 0, sizeof(m_password));
	memset(m_nickname, 0, sizeof(m_nickname));
	memset(m_previous_buffer, 0, sizeof(m_previous_buffer));

	m_port = 119; // default for all NNTP servers
	m_newsgroup_index = 0;
	m_first_header_index = 0;
	m_last_header_index = 0;
	m_new_headers = 0;

	m_receiving_headers = false;
	m_searching = false;

	m_watchdog = NULL;

	GetNewsGroups();
}

UsenetSocket::~UsenetSocket(void)
{
}

void UsenetSocket::InitParent(UsenetSearcherDll *dll)
{
	p_dll = dll;
}

//
//
//
void UsenetSocket::OnConnect(int error_code)
{
	if(error_code == 0)
	{
		p_dll->Log(m_nickname, "*** Connected. Sending username now..."); p_dll->Log(m_nickname, "");

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
		
		p_dll->SwitchServers(msg);

		char email_msg[2048];
		sprintf(email_msg, "*** USENET SEARCHER NOTIFICATION ***\n\n%s\n\nServer nickname: %s\nServer: %s", msg, m_nickname, m_server_name);
		p_dll->m_smtp_socket.SendMail(email_msg);
		
		Close();
	}
}

//
//
//
void UsenetSocket::OnClose(int error_code)
{
	// clear the vectors
	v_my_projects.clear();
	v_new_headers.clear();
	v_send_data_buffers.clear();
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

//
//
//
void UsenetSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
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
			::ZeroMemory(&m_previous_buffer, 8192);
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
	if (m_receiving_headers)
	{
		LogHeader(buf);
		return;
	}

	// First get the op code
	int op_code = 0;
	sscanf(buf, "%d", &op_code);

	switch (op_code)
	{
	case 200:
		{
			// Connected... we already sent username and password, wait to parse it and gain access
			m_searching = true;
			m_full_scan_tick = GetTickCount();
			m_watchdog = CTime::GetCurrentTime();
			break;
		}
	case 211:
		{
			// Group selected... gives the range of headers in the group
			int ret=0, num_headers=0, first_header=0, last_header=0;
			ret = sscanf(buf, "%d %d %d %d", &op_code, &num_headers, &first_header, &last_header);
			ASSERT(ret == 4);

			m_new_headers = last_header-v_newsgroups[m_newsgroup_index].m_last_header_received;

			// If this is the first iteration, set the most recent header to the last one.
			if ( (v_newsgroups[m_newsgroup_index].m_last_header_received != 0) && (m_new_headers > 0) )
			{
				// Start retrieving all the new headers, starting with the last one we received.
				m_first_header_index = v_newsgroups[m_newsgroup_index].m_last_header_received + 1;
				m_last_header_index = last_header;
				v_newsgroups[m_newsgroup_index].m_last_header_received = last_header;
				RequestNewHeaders();
			}
			else
			{
				char msg[2048];
				sprintf(msg, "%s (0 new headers)", v_newsgroups[m_newsgroup_index].m_name);
				p_dll->Log(m_nickname, msg);
				
				// Set the last header and move on
				v_newsgroups[m_newsgroup_index].m_last_header_received = last_header;
				m_newsgroup_index++;
				DoNextNewsGroup();
			}
			break;
		}
	case 221:
		{
			// We should never get here... because we already know we are receiving headers; we just requested them.
			m_receiving_headers = true;
			break;
		}
	case 281:	
		{
			// Authentication succeeded, start retrieving headers from newsgroups
			DoNextNewsGroup();
			break;
		}
	case 381:	
		{
			// More authentication needed
			break;
		}
	case 411:
		{
			// No such group
			char msg[1024];
			sprintf(msg, "*** %s does not exist ***", v_newsgroups[m_newsgroup_index].m_name);
			p_dll->Log(m_nickname, msg);

			m_newsgroup_index++;
			DoNextNewsGroup();
			break;
		}
	case 502:
		{
			// access restriction or permission denied
			p_dll->SwitchServers(buf);

			char email_msg[2048];
			sprintf(email_msg, "*** USENET SEARCHER NOTIFICATION ***\n\nAccess Restriction or Permission Denied.\n\nServer nickname: %s\nServer: %s", m_nickname, m_server_name);
			p_dll->m_smtp_socket.SendMail(email_msg);
			break;
		}
	default:
		{
			p_dll->Log(m_nickname, "Unhandled response from server:");
			p_dll->SwitchServers(buf);

			char email_msg[2048];
			sprintf(email_msg, "*** USENET SEARCHER NOTIFICATION ***\n\nUnhandled Responce From Server:\n\t%s\nServer nickname: %s\nServer: %s", buf ,m_nickname, m_server_name);
			p_dll->m_smtp_socket.SendMail(email_msg);
			break;
		}
	}
}

//
//
//
void UsenetSocket::DoNextNewsGroup()
{
	m_watchdog = CTime::GetCurrentTime();

	if(m_newsgroup_index < (int)v_newsgroups.size())
	{
		char *buf = new char[2048];
		sprintf(buf, "GROUP %s\r\n", v_newsgroups[m_newsgroup_index].m_name);
		SendSocketData((unsigned char *)buf, (UINT)strlen(buf));
		delete buf;
	}
	else
	{	
		// All newsgroups have been searched.
		char msg[1024];
		sprintf(msg, "All groups searched (%.3f seconds).  Closing socket.  Found %d new headers.", ((double)(GetTickCount()-m_full_scan_tick))/1000.0 ,v_new_headers.size());
		p_dll->Log(m_nickname, msg); p_dll->Log(m_nickname, "");

		m_newsgroup_index = 0;
		Close();	// close the socket
		m_searching = false;

		if (v_new_headers.size() > 0)
		{
			StartWorkerThread();
			v_new_headers.clear();
		}
	}
}

//
//	 Get all of the new headers and put them in a vector for the worker thread.
//		precondition:  m_newsgroup_index is the index for the current newsgroup in v_newsgroups
//					   m_first_header_index is where we should start requesting
//					   m_last_header_index is the last one we should request
//
void UsenetSocket::RequestNewHeaders()
{
	ASSERT( m_first_header_index <= m_last_header_index );

	m_watchdog = CTime::GetCurrentTime();
	m_group_tick = GetTickCount();
	m_receiving_headers = true;

	char *buf = new char[32];
	sprintf(buf, "XOVER %u-%u\r\n", m_first_header_index, m_last_header_index);
	SendSocketData((unsigned char *)buf, (UINT)strlen(buf));
	delete buf;
}

//
//	This function parses the response for each header.
//	Push it onto v_new_headers().
//
//	Format of response: header id, subject, author, date, message-id, references, byte count, line count, header_id
//
void UsenetSocket::LogHeader(char *buf)
{
	int header_id=0, ret=0;

	// This is just the intro
	if ( strstr(buf, "224") == buf )
		return;

	// Check to see if we are done
	if((strlen(buf)==1) && (strstr(buf, ".") == buf))
	{
		char msg[1024];
		sprintf(msg, "%s (%d new headers, %.3f seconds)", v_newsgroups[m_newsgroup_index].m_name, m_new_headers, ((double)(GetTickCount()-m_group_tick))/(1000.0));
		p_dll->Log(m_nickname, msg);

		m_receiving_headers = false;
		m_newsgroup_index++;
		DoNextNewsGroup();
		return;
	}

	int byte_count = 0;

	// This is the order of the fields returned for each header after executing an XOVER command:
	//		header id, subject, author, date, message-id, references, byte count, line count
	//
	Header header;
	ret = sscanf(buf, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t\t%d\t%d",
			&header_id,
			header.m_subject,
			header.m_from,
			header.m_post_date,
			header.m_message_id,
			&byte_count,
			&header.m_lines);

	int lines = header.m_lines;

	strcpy(header.m_group, v_newsgroups[m_newsgroup_index].m_name);

	v_new_headers.push_back(header);
}

//
//	Reads all the newsgroups we are interested in from the file 'newsgroups.txt' located
//	in the trapper keeper folder.
//
void UsenetSocket::GetNewsGroups()
{
	CStdioFile file;
	CString line;

	CFileException ex;
	if (file.Open("newsgroups.txt", CFile::modeRead, &ex) == 0)
	{
		char error[256];
		itoa(ex.m_cause, error, 10);
		CString msg = "Could not open 'newgroups.txt' error: ";
		msg += error;
		MessageBox(NULL, msg, "File Open Error", MB_OK);
	}
	else
	{
		while (file.ReadString(line) != NULL)
		{
			if ( (line.Compare("")==0) || (strstr(line, "#")!=NULL) )
				continue;

			Newsgroup group;
			group.m_name = line;
			v_newsgroups.push_back(group);
		}
	}
}

//
//
//
void UsenetSocket::StartWorkerThread()
{
	ThreadData *data = new ThreadData; // This be freed by the thread
	data->v_headers = v_new_headers;
	data->v_projects = p_dll->v_project_keywords;
	data->m_dlg_hwnd = p_dll->m_dlg_hwnd;
	data->p_dll = p_dll;

	// Start a new thread to insert the find more results
	::PostMessage(p_dll->m_dlg_hwnd,WM_SCAN_THREAD, (WPARAM)data,(LPARAM) 0);
}