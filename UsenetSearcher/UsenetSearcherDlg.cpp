// UsenetSearcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UsenetSearcherDll.h"
#include "UsenetSearcherDlg.h"
#include "Shlwapi.h"			// StrStrI()
#include "mysql.h"

// UsenetSearcherDlg dialog

IMPLEMENT_DYNAMIC(UsenetSearcherDlg, CDialog)
UsenetSearcherDlg::UsenetSearcherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(UsenetSearcherDlg::IDD, pParent)
{
}

UsenetSearcherDlg::~UsenetSearcherDlg()
{
}

void UsenetSearcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MSG_BOX, m_list_box);
	DDX_Control(pDX, IDC_LIST_TREAD_BOX, m_thread_box);
}

//
//
//
void UsenetSearcherDlg::InitParent(UsenetSearcherDll *parent)
{
	p_dll = parent;
}

BEGIN_MESSAGE_MAP(UsenetSearcherDlg, CDialog)
	ON_MESSAGE(WM_SCAN_THREAD, ScanThread)
	ON_MESSAGE(WM_THREAD_LOG, ThreadLogMsg)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// UsenetSearcherDlg message handlers

void UsenetSearcherDlg::OnTimer(UINT nIDEvent)
{
	KillTimer(nIDEvent);

	switch(nIDEvent)
	{
	case 1:	// Main timer for searching, and to refresh the project list.
		{
			// Refresh the projects every 12 hours:
			CTimeSpan ts;
			ts = CTime::GetCurrentTime() - p_dll->m_last_project_update;
			if (ts.GetTotalHours() > 6)
			{
				p_dll->GetProjects();
				p_dll->m_last_project_update = CTime::GetCurrentTime();
			}

			int ret=0;
			if (p_dll->v_usenet_socket_ptrs[p_dll->m_server_index]->m_searching == false)
			{
				ret = p_dll->v_usenet_socket_ptrs[p_dll->m_server_index]->Close();	// just in case, possible the watchdog fired and the socket was never closed.
				ret = p_dll->v_usenet_socket_ptrs[p_dll->m_server_index]->Create();
				ret = p_dll->v_usenet_socket_ptrs[p_dll->m_server_index]->Connect(p_dll->v_usenet_socket_ptrs[p_dll->m_server_index]->m_server_name, p_dll->v_usenet_socket_ptrs[p_dll->m_server_index]->m_port);
			}

			SetTimer(1, 1000*45, NULL);
			break;
		}

	case 2:	// Watchdog timer
		{
			for (UINT i=0; i < p_dll->v_usenet_socket_ptrs.size(); i++)
			{
				CTimeSpan ts = CTime::GetCurrentTime() - p_dll->v_usenet_socket_ptrs[i]->m_watchdog;
				if (ts.GetTotalMinutes() >= 1 && p_dll->v_usenet_socket_ptrs[i]->m_searching == true)
				{
					char msg[1024];
					sprintf(msg, "   Watchdog fired for %s, reseting.", p_dll->v_usenet_socket_ptrs[i]->m_server_name);
					Log("* * * * * * * * *");
					Log(msg);
					Log("* * * * * * * * *");

					p_dll->v_usenet_socket_ptrs[i]->m_searching = false;
					p_dll->v_usenet_socket_ptrs[i]->m_receiving_headers = false;
					p_dll->v_usenet_socket_ptrs[i]->m_newsgroup_index = 0;
				}
			}

			SetTimer(2, 5000, NULL);	// 5 second watchdog timer, only triggered if searching pauses for 1 minute
			break;
		}

	default:
		{

			break;
		}
	}

	CDialog::OnTimer(nIDEvent);
}

//
//
//
void UsenetSearcherDlg::Log(char *buf)
{
	char msg[4096];
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
	strcat(msg, buf);

	m_list_box.InsertString(0,msg);
	while(m_list_box.GetCount()>1000)
	{
		m_list_box.DeleteString(m_list_box.GetCount()-1);
	}
}

//
//
//
void UsenetSearcherDlg::ThreadLog(char *buf)
{
	m_thread_box.InsertString(0, buf);
	while(m_thread_box.GetCount()>1000)
	{
		m_thread_box.DeleteString(m_thread_box.GetCount()-1);
	}
}

//
//
//
void ReplaceInvalidCharacters(CString *cstring)
{
	cstring->Replace("\\","\\\\");
	cstring->Replace("'","\\'");
}

void SendToPoster(vector<Header> v_headers, UsenetSearcherDll *dll)
{
	int num_headers = (int)v_headers.size();

	int length = sizeof(Header)*num_headers+1;

	byte *ptr = new byte[length];
	memset(ptr, 0, length);

	byte *data = ptr;

	for (int i=0; i < num_headers; i++)
	{
		memcpy(data, &v_headers[i], sizeof(Header));
		data += sizeof(Header);
	}

	bool ret = dll->m_com.SendReliableData("127.0.0.1", (void*)ptr, length);

	delete [] ptr;
	return;
}

//
//
//
vector<Header> ExtrapolateNewPosts(vector<Header> v_headers)
{
	OutputDebugString("Extrapolating new posts...\n");
	int i;
	UINT j;
	bool found;
	bool is_movie = false;
	bool is_parens = true;
	bool good_file_extention;

	char subject_and_parts[256+512];
	char subject_no_parts[256+1];
	char total_parts[128];
	char this_part[128];
	char parts[128];				// I don't think any post will have >=100,000 parts...
	char toAppend[128];				// For appending the extrapolated part
	char anything_extra[128];		// For holding anything extra after the parts

	char* ptr;
	char* endptr;
	char* file_extention_ptr;

	Header temp_header;

    // Iterator is used to loop through the vector.
    vector<Header>::iterator theIterator;
	vector<Header>::iterator tempIterator;
	vector<Header> temp_vector;
	
	theIterator = v_headers.begin();
	while ( theIterator != v_headers.end() )
	{
		ptr=NULL;
		endptr=NULL;
		file_extention_ptr=NULL;
		is_movie = false;
		good_file_extention = true;

		memset(subject_and_parts, 0, sizeof(subject_and_parts));
		strcpy(subject_and_parts, theIterator->m_subject);
		subject_and_parts[257] = '\0'; // just in case it's some super-long file name

		file_extention_ptr = StrRStrI(subject_and_parts, NULL, ".mp3");	// for MP3!
		if (file_extention_ptr == NULL)
		{
			file_extention_ptr = StrRStrI(subject_and_parts, NULL, ".r");	// for rars .r01 etc..
			is_movie = true;	// if it's not an .mp3 then assume it's a movie, if it's not a movie, we won't post it anyways...
		}

		if (file_extention_ptr == NULL)
			file_extention_ptr = StrRStrI(subject_and_parts, NULL, ".p");	// for parts .p01 etc..

		if (file_extention_ptr == NULL)
			file_extention_ptr = StrRStrI(subject_and_parts, NULL, ".zip");

		if (file_extention_ptr == NULL)  // this is neither a music nor a movie file
			good_file_extention = false;


		// Check to see if the "req" comes very shortly before the project name.
		// This is because sometimes we catch posts for other artists, when they are posted
		// in hopes that the person they post it for, will answer their request for *our* artist

		if (good_file_extention)
		{
			char *req = StrStrI(subject_and_parts, "req");
			if (req != NULL)
			{
				char *proj = StrStrI(req, theIterator->m_project);

				// ***												  *** //
				//														  //
				//			SET THE REQ: MARKER CUT-OFF POINT			  //
				//														  //
				// ***												  *** //
				if (proj != NULL)
				{
					if ( proj < (req + 10) ) // ***  an arbitrary choice for # char's before the project name
						good_file_extention = false;

					if ((StrStrI(subject_and_parts, "as req") != NULL) && good_file_extention == false )
						good_file_extention = true;
				}
			}
		}

		if (good_file_extention)
		{
			is_parens = true;
			ptr = strstr(file_extention_ptr, ")");
			if (ptr == NULL)
			{
				ptr = strstr(file_extention_ptr, "]");
				is_parens = false;
			}

			if ( ptr != NULL )	// This is a media file post, with parts, after the file extention
			{
				ptr = strstr(file_extention_ptr, "/");
				if (ptr != NULL)
				{
					ptr++;

					// Get the total number of parts from the string
					i=0;
					memset(parts, 0, sizeof parts);
					while( (*ptr != ')') && (*ptr != ']') )
					{
						parts[i++] = *ptr;
						ptr++;
					}
					theIterator->m_total_parts = atoi(parts);

					ptr = subject_and_parts;

					endptr = NULL;
					if (is_parens)
						endptr = strstr(file_extention_ptr, "(");
					else
						endptr = strstr(file_extention_ptr, "[");

					if (endptr != NULL)
					{
						// Get the subject only, so we can compare sepearte parts
						i=0;
						memset(subject_no_parts, 0, sizeof subject_no_parts);
						while( ptr != endptr )
						{
							subject_no_parts[i++] = *ptr;
							ptr++;
						}

						strcpy(theIterator->m_subject_no_parts, subject_no_parts);

						// Loop through the temp vector, see if the subject_no_parts already
						// exists there, and if so, move on. If not, push_back onto temp_vector
						found = false;
						if (temp_vector.empty())
						{
							temp_vector.push_back(*theIterator);
						}
						else
						{
							tempIterator = temp_vector.begin();
							while ( tempIterator != temp_vector.end() )
							{
								if ( strcmp(tempIterator->m_subject_no_parts, theIterator->m_subject_no_parts) == 0 )
								{
									found = true;	// Already exists
									break;
								}
								tempIterator++;
							}

							if (!found)
								temp_vector.push_back(*theIterator);
						}
					}
					else
					{
						OutputDebugString("Error 300 Occured in ExtrapolateNewPosts():");
						OutputDebugString(subject_and_parts);
						int len = (int)strlen(subject_and_parts);
						// I can't imagine what post would get us here...
						// blah blah blah.mp3 [12/30) maybe?
					}
				}
			}  // end the test for posts we want to counter-post (actuall mp3 or movies)
		}	// end if(good_file_extention)
		theIterator++;
	}

	// By here the temp_vector should be filled with unique-in-subject messages
	// Do the real extrapolation and fill up v_messages with complete messages
	OutputDebugString("Got all unique-in-subject messages.");

	v_headers.clear();
	tempIterator = temp_vector.begin();
	while( tempIterator != temp_vector.end() )
	{
		is_movie = false;	// assume it's an MP3
		is_parens = true;	// with parenthesis

		memset(total_parts, 0, sizeof total_parts);
		itoa(tempIterator->m_total_parts, total_parts, 10);

		strcpy(temp_header.m_project, tempIterator->m_project);
		strcpy(temp_header.m_subject_no_parts, tempIterator->m_subject_no_parts);
		strcpy(temp_header.m_from, tempIterator->m_from);
		strcpy(temp_header.m_group, tempIterator->m_group);
		strcpy(temp_header.m_post_date, tempIterator->m_post_date);
//		strcpy(temp_header.m_nntp_posting_date, tempIterator->m_nntp_posting_date);
//		strcpy(temp_header.m_nntp_posting_host, tempIterator->m_nntp_posting_host);
//		strcpy(temp_header.m_nntp_posting_host_ip, tempIterator->m_nntp_posting_host_ip);
//		strcpy(temp_header.m_client_posting_ip, tempIterator->m_client_posting_ip);
		strcpy(temp_header.m_message_id, tempIterator->m_message_id);

		temp_header.m_total_parts = tempIterator->m_total_parts;
		temp_header.m_lines = tempIterator->m_lines;

		file_extention_ptr = StrRStrI(tempIterator->m_subject, NULL, ".mp3");

		if (file_extention_ptr == NULL)
		{
			file_extention_ptr = StrRStrI(tempIterator->m_subject, NULL, ".r");
			is_movie = true;
		}
		if (file_extention_ptr == NULL)
			file_extention_ptr = StrRStrI(tempIterator->m_subject, NULL, ".p");

		if (file_extention_ptr == NULL)
			file_extention_ptr = StrRStrI(tempIterator->m_subject, NULL, ".zip");

		CString str = "9 - ";
		str += temp_header.m_subject_no_parts;
		str += " | ";
		str += temp_header.m_project;
		str += " | ";
		str += temp_header.m_nntp_posting_host;
		str += "\n";

		OutputDebugString(str);
		if (file_extention_ptr != NULL)
		{
			if (strstr(file_extention_ptr, ")") == NULL)
				is_parens = false;

			if ( !is_parens && (strstr(file_extention_ptr, "]") == NULL) )
			{
				OutputDebugString("Error 400 Occured in ExtrapolateNewPosts():");
				OutputDebugString(tempIterator->m_subject); // Not parens, and not ']'s either... very bad.
			}
			else
			{

				// ***
				//	At this point, we know whether the post is a music file,
				//	or a movie file; and whether it uses '('s or '['s
				// ***

				// Get anything extra after the parts
				memset(anything_extra, 0, sizeof anything_extra);
				if (is_parens)
					ptr = strrchr(file_extention_ptr, ')');
				else
					ptr = strrchr(file_extention_ptr, ']');

				ptr++;
				strcpy(anything_extra, ptr);

				//
				// KLUDGE:  if we want to do ALL posts for movies and music... so just
				//			treat movies just like mp3's.
				//
//				is_movie = false;

//				if ( (strstr(tempIterator->m_subject, "[") == NULL) && is_movie)
//					is_movie = false;	// treat this post like music, because it's not in the proper format

				if (is_movie)
				{
/*
					char first_half[256];
					memset(first_half, 0, sizeof first_half);

					int counter=0;

					while (tempIterator->m_subject_no_parts[counter] != '[')
					{
						first_half[counter] = tempIterator->m_subject_no_parts[counter];
						counter++;
					}

					char this_big_part_chr[3];
					memset(this_big_part_chr, 0, sizeof(this_big_part_chr));
					
					int x=0;
					counter++;
					while (tempIterator->m_subject_no_parts[counter] != '/')
					{
						this_big_part_chr[x++] = tempIterator->m_subject_no_parts[counter++];
					}
					int this_big_part = atoi(this_big_part_chr);

					char big_parts_chr[3];
					memset(big_parts_chr, 0, sizeof(big_parts_chr));

					x=0;
					char *p = StrRStrI(tempIterator->m_subject_no_parts, NULL, "/");
					while (*p != ']')
					{
						p++;
						big_parts_chr[x++] = *p;
						ASSERT(x < 10);
					}
					int big_parts = atoi(big_parts_chr);
*/
/*
					// Get the difference between this_big_part and the part #
					p = StrStrI(tempIterator->m_subject_no_parts, ".part");
					ASSERT(p != NULL);
					p += strlen(".part");

					char part_num_chr[3];
					memset(part_num_chr, 0, sizeof(part_num_chr));
	
					x = 0;
					while (*p != '.')
					{
						part_num_chr[x++] = *p;
						p++;
						ASSERT(x < 10);
					}
					int part_num = atoi(part_num_chr);

					// This is the difference we must maintain thru all the .parts
					int difference = this_big_part - part_num;
*/
/*
					// ***
					// This is where I left off on Tuesday...
					//
					char second_half[256];
					memset(second_half, 0, sizeof(second_half));
					//
					// ***

					counter=0;
					p = StrRStrI(tempIterator->m_subject_no_parts, NULL, "]");

					for (int this_big_part=1; this_big_part <= big_parts; this_big_part++)
					{
						// put that shit in here, one iteration for each .part subject...gezzz...
					}

					// ***
					//	This is the part from the MP3 section
					//		- put all this crap inside the above for() loop
					// ***
*/
					int parts_to_jack = 5;
					if (tempIterator->m_total_parts < 5)
						parts_to_jack = tempIterator->m_total_parts;

					// Seed the random number generator
					srand( (unsigned)time( NULL ) );

					// Create the parts we want to jack.  5 random parts in this case.
					for(i=1; i <= parts_to_jack; i++ )		
					{
						int max_parts = tempIterator->m_total_parts;
						int part_to_jack = ( rand() % max_parts ) + 1;

						memset(this_part, 0, sizeof(this_part));
						itoa(part_to_jack, this_part, 10);

						memset(toAppend, 0, sizeof(toAppend));
						if (is_parens)
							toAppend[0] = '(';
						else
							toAppend[0] = '[';

						// Add the leading zero, for parts less than 10, or 100. Perhaps even 1000 ex: (004/100)
						for (j=1; j <= (strlen(total_parts)-strlen(this_part)); j++)
							toAppend[j] = '0';

						strcat(toAppend, this_part);
						toAppend[j+strlen(this_part)] = '/';
						strcat(toAppend, total_parts);

						if (is_parens)
							toAppend[strlen(toAppend)] = ')';
						else
							toAppend[strlen(toAppend)] = ']';

						strcat(toAppend, anything_extra);

						memset(subject_and_parts, 0, sizeof(subject_and_parts));
						strcpy(subject_and_parts, tempIterator->m_subject_no_parts);
						strcat(subject_and_parts, toAppend);

						strcpy(temp_header.m_subject, subject_and_parts);

						// Push it back on the vector for posting.
						if (i <= parts_to_jack)
						{
							v_headers.push_back(temp_header);
						}

/*
					// Movie post, all parts... basically... double extrapolation.
					i = (rand() % tempIterator->m_total_parts) + 1;

					memset(this_part, 0, sizeof(this_part));
					itoa(i, this_part, 10);

					memset(toAppend, 0, sizeof(toAppend));
					if (is_parens)
						toAppend[0] = '(';
					else
						toAppend[0] = '[';

					// Add the leading zero, for parts less than 10, or 100. Perhaps even 1000 ex: (004/100)
					for (j=1; j <= (strlen(total_parts)-strlen(this_part)); j++)
						toAppend[j] = '0';

					strcat(toAppend, this_part);
					toAppend[j+strlen(this_part)] = '/';
					strcat(toAppend, total_parts);

					if (is_parens)
						toAppend[strlen(toAppend)] = ')';
					else
						toAppend[strlen(toAppend)] = ']';

					strcat(toAppend, anything_extra);

					memset(subject_and_parts, 0, sizeof(subject_and_parts));
					strcpy(subject_and_parts, tempIterator->m_subject_no_parts);
					strcat(subject_and_parts, toAppend);

					strcpy(temp_header.m_subject, subject_and_parts);

					// Push it back on the vector for posting (and reporting).
					v_headers.push_back(temp_header);

					// For movies, we only want to report the single piece we jack (no parts)
					strcpy(temp_header.m_subject, tempIterator->m_subject_no_parts);
*/
					}
				}		
				else
				{
/*
					// It's a music file, do which parts?  All?  First 3?
					int parts_to_jack = 1;

					if (tempIterator->m_total_parts >= 30)		// if more than 30, jack first 5
						parts_to_jack = 5;
					else if (tempIterator->m_total_parts >= 3) // if (3, 29), jack first 3
						parts_to_jack = 3;
					else										// if < 3, jack only the first part
						parts_to_jack = 1;
*/
					int parts_to_jack = tempIterator->m_total_parts;

					for(i=1; i <= tempIterator->m_total_parts; i++ )		
					{
						memset(this_part, 0, sizeof(this_part));
						itoa(i, this_part, 10);

						memset(toAppend, 0, sizeof(toAppend));
						if (is_parens)
							toAppend[0] = '(';
						else
							toAppend[0] = '[';

						// Add the leading zero, for parts less than 10, or 100. Perhaps even 1000 ex: (004/100)
						for (j=1; j <= (strlen(total_parts)-strlen(this_part)); j++)
							toAppend[j] = '0';

						strcat(toAppend, this_part);
						toAppend[j+strlen(this_part)] = '/';
						strcat(toAppend, total_parts);

						if (is_parens)
							toAppend[strlen(toAppend)] = ')';
						else
							toAppend[strlen(toAppend)] = ']';

						strcat(toAppend, anything_extra);

						memset(subject_and_parts, 0, sizeof(subject_and_parts));
						strcpy(subject_and_parts, tempIterator->m_subject_no_parts);
						strcat(subject_and_parts, toAppend);

						strcpy(temp_header.m_subject, subject_and_parts);

						// Push it back on the vector for posting.
						if (i <= parts_to_jack)
						{
							v_headers.push_back(temp_header);
						}

					}
				}
			} // no parens?
		} // file_extention == NULL?
		tempIterator++;
	}

	return v_headers;
}

//
//
//
UINT ScanThreadProc(LPVOID pParam)
{
	DWORD tick = GetTickCount();

	char *msg = NULL;	// for thread logging... the ThreadLogMsg() will free all memory allocations.
	
	ThreadData *ptr = (ThreadData *)pParam;

	// Create a new copy of this data for the thread
	ThreadData *data = new ThreadData;
	data->m_dlg_hwnd = ptr->m_dlg_hwnd;
	data->p_dll = ptr->p_dll;
	data->v_headers = ptr->v_headers;
	data->v_projects = ptr->v_projects;

	delete ptr;	// delete the old copy

	msg = new char[1024];
	sprintf(msg, "*** Starting new worker thread... Scanning %d headers for %d projects.", data->v_headers.size(), data->v_projects.size());
	::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);

	vector<Header>::iterator header_iter = data->v_headers.begin();
	//
	// First, check to see if any of these headers match any of our projects (save only the headers that match):
	//
	while (header_iter != data->v_headers.end())
	{
		bool match;

		// Check this header against project keywords.
		for (UINT i=0; i < data->v_projects.size(); i++)
		{
			match = true;

			ProjectKeywords project = data->v_projects[i];
			CString subject = header_iter->m_subject;

			// Test for no .p, .r, or .mp3  -- these are not meaningful posts
			if ( (StrStrI(subject, ".p")==NULL) && (StrStrI(subject, ".r")==NULL) && (StrStrI(subject, ".mp3")==NULL) )
			{
				match = false;
				break;
			}

			// Use exact keywords if we have them.
			if (project.m_query_keywords.v_exact_keywords.size() > 0)
			{
				for (UINT j=0; j < project.m_query_keywords.v_exact_keywords.size(); j++)
				{
					CString keyword = project.m_query_keywords.v_exact_keywords[j].keyword.c_str();
					if (StrStrI(subject, keyword) != NULL)
					{
						continue;
					}
					else
					{
						match = false;
						break;
					}
				}

				if (match)
				{
					strcpy( header_iter->m_project, project.m_project_name.c_str() );
					header_iter++;
					break;
				}
			}
			else if (project.m_query_keywords.v_keywords.size() > 0)
			{
				// Use the regular keywords if there are no exact keywords
				for (UINT j=0; j < project.m_query_keywords.v_keywords.size(); j++)
				{
					CString keyword = project.m_query_keywords.v_keywords[j].keyword.c_str();
					if (StrStrI(subject, keyword) != NULL)
					{
						continue;
					}
					else
					{
						match = false;
						break;
					}
				}

				if (match)
				{
					strcpy( header_iter->m_project, project.m_project_name.c_str() );
					header_iter++;
					break;
				}
			}
			else
			{
				// Do we need to use the the freakin' qrp keywords?
				bool we_have_a_small_problem = true;
			}
		}

		if (!match)
		{
			data->v_headers.erase(header_iter);
		}
	}

	if (data->v_headers.size() == 0)
	{
		msg = new char[1024];
		sprintf(msg, "** Thread exiting. No matches found. (%.3f seconds)", ((double)(GetTickCount()-tick))/1000.0);
		::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);

		msg = new char[1024];
		sprintf(msg, "");
		::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);

		delete data;
		return 0;
	}

	//
	//	Now that we have matched these posts against the projects... we should do a second check for this projects
	//	particular keywords.
	//
	header_iter = data->v_headers.begin();
	while (header_iter != data->v_headers.end())
	{
		bool match = false;
		CString this_project = header_iter->m_project;

		for (int k=0; k < (int)data->v_projects.size(); k++)
		{
			if (strcmp(data->v_projects[k].m_project_name.c_str(), this_project) == 0)
			{
				if (data->v_projects[k].m_supply_keywords.v_keywords.size() == 0)
				{
					match = true;
					break; // out of project loop, no keywords, probably a movie
				}

				for (int keyword_index=0; keyword_index < (int)data->v_projects[k].m_supply_keywords.v_keywords.size(); keyword_index++)
				{
					CString subject = header_iter->m_subject;
					CString track_name = data->v_projects[k].m_supply_keywords.v_keywords[keyword_index].m_track_name.c_str();

					if (StrStrI(subject, track_name) != NULL)
					{
						match = true;
						break;	// out of keyword loop
					}
				}

				break;	// out of project loop
			}
		}

		if (match)
			header_iter++;
		else
			data->v_headers.erase(header_iter);
	}

	//
	// Second, compare against the database to see if this is a new post, or our counterpost.
	//			if it is our counterpost, log the time we got it and the counterpost message id, then remove the header from the vector.
	//			otherwise, keep the header and send it to 
	//
	msg = new char[1024];
	sprintf(msg, "Found %d headers that match. (%.3f seconds) Checking against the counter-post database...", data->v_headers.size(), ((double)(GetTickCount()-tick))/1000.0);
	::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);

	MYSQL *conn;
	MYSQL_RES *res = NULL;
	conn=mysql_init(NULL);

	DWORD database_tick = GetTickCount();

	// Testing the interaction to the DC Master (# of queries/second)
	int query_counter = 0;
	
	//
	// Opens a connection to the DC Master database.
	// 
	if (mysql_real_connect(conn,"38.118.160.161","onsystems","ebertsux37","dcdata",0,NULL,0) == NULL)
	{
		msg = new char[1024];
		sprintf(msg,"(Failed to connect to database: %s", mysql_error(conn));
		::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);
	}
	else
	{
		header_iter = data->v_headers.begin();
		while (header_iter != data->v_headers.end())
		{
			bool this_is_our_counter_post = false;
			int ret = 0, lines = 0;
			CString from, newsgroup, subject, message_id, project, date;
			char query[3072];
			memset(query, 0, 3072);

			from = header_iter->m_from;
			newsgroup = header_iter->m_group;
			subject = header_iter->m_subject;
			message_id = header_iter->m_message_id;
			project = header_iter->m_project;
			date = header_iter->m_post_date;
			lines = header_iter->m_lines;

			ReplaceInvalidCharacters(&from);
			ReplaceInvalidCharacters(&subject);

			CString subject_no_parts = subject;
			int index = subject_no_parts.ReverseFind('(');

			if (index > 0)
				subject_no_parts = subject_no_parts.Left(index-1);
			else
			{
				data->v_headers.erase(header_iter);
				continue;
			}

			if (strstr(subject_no_parts, "'") != NULL)
				int brake = 1;

			//
			// I think we can decipher whether this is ours or not by whether or not the counter_post_time is filled in or not.
			//	Because, if counter_post_time is NULL here, then this must be the real post.
			//	But if counter_post_time is NOT NULL, then it could be the real one, OR our counterpost. (do we need the lines?)
			//
			sprintf(query, "SELECT origonal_post_time, counter_post_time FROM usenet_protection WHERE subject like '%s%%'", subject_no_parts);
			
			query_counter++;
			ret = mysql_query(conn, query);
			if (ret != 0)	// check for error
				TRACE(mysql_error(conn));

			res = mysql_store_result(conn); // allocates mem (::)
			int num_rows = (int)mysql_num_rows(res);
			mysql_free_result(res);			// free's mem (::)

			memset(query, 0, sizeof query);
			if (num_rows > 0)	// We have already counter-posted on this...
			{
				if (lines == 1)
				{
					// Update the counter-post info
					sprintf(query, "UPDATE usenet_protection SET counter_post_found = now(), counter_post_message_id = '%s' WHERE subject like '%s%%'", message_id, subject_no_parts);
					query_counter++;
					ret = mysql_query(conn, query);

					if (ret != 0)	// check for error
						TRACE(mysql_error(conn));
				}
				
				// Remove this from the posting queue (we don't want to counter-post twice)
				data->v_headers.erase(header_iter);
			}
			else
			{
				// Keep the post, extrapolate new posts and send them to the poster
				sprintf(query, "INSERT INTO usenet_protection (project, user, newsgroup, subject, origonal_post_time, message_id) VALUES ('%s','%s','%s','%s','%s','%s')",project,from,newsgroup,subject_no_parts,date,message_id);
				query_counter++;
				ret = mysql_query(conn, query);

				if (ret != 0)	// check for error
				{
					TRACE(mysql_error(conn));	// most likely a duplicate entry... so ignore this sucker 
					data->v_headers.erase(header_iter);
				}
				else
				{
					header_iter++;

					msg = new char[1024];
					sprintf(msg,"New origonal post: %s", subject);
					::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);
				}
			}
		}

		mysql_close(conn);
	}

	msg = new char[1024];
	sprintf(msg, "DC MASTER DATABASE TIME: %d querys in %.3f seconds", query_counter, ((double)(GetTickCount()-database_tick))/1000.0);
	::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);

	//
	// Last, extrapolate new posts and send any headers left in v_headers to the Usenet Poster (wherever that is)
	//

	if (data->v_headers.size() > 0)
	{
		int before_extrapolation = (int)data->v_headers.size();
		data->v_headers = ExtrapolateNewPosts(data->v_headers);
		int after_extrapolation = (int)data->v_headers.size();

		// Send that shit home...
		if ((int)data->v_headers.size() > 0)
			SendToPoster(data->v_headers, data->p_dll);

	 	msg = new char[1024];
		sprintf(msg, "** Thread exiting. Extrapolated %d posts into %d headers. (%.3f seconds)", before_extrapolation, after_extrapolation, ((double)(GetTickCount()-tick))/1000.0);
		::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);
	}
	else
	{
	 	msg = new char[1024];
		sprintf(msg, "** Thread exiting. No new posts. No extrapolation. (%.3f seconds)", ((double)(GetTickCount()-tick))/1000.0);
		::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);
	}

	msg = new char[1024];
	sprintf(msg, "");
	::PostMessage(data->m_dlg_hwnd, WM_THREAD_LOG, (WPARAM)msg,(LPARAM)0);

	delete data;
	return 0;
}

//
//
//
LRESULT UsenetSearcherDlg::ScanThread(WPARAM wparam, LPARAM lparam)
{
	// Start the thread
	AfxBeginThread(ScanThreadProc,(LPVOID)wparam,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	return 0;
}

//
//
//
LRESULT UsenetSearcherDlg::ThreadLogMsg(WPARAM wparam, LPARAM lparam)
{
	char *ptr=(char *)wparam;
	ThreadLog(ptr);
	delete [] ptr;
	return 0;
}