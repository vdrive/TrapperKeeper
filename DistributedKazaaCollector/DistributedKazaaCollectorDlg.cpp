// DistributedKazaaCollectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DistributedKazaaCollectorDll.h"
#include "DistributedKazaaCollectorDlg.h"


// CDistributedKazaaCollectorDlg dialog

IMPLEMENT_DYNAMIC(CDistributedKazaaCollectorDlg, CDialog)
CDistributedKazaaCollectorDlg::CDistributedKazaaCollectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDistributedKazaaCollectorDlg::IDD, pParent)
{
}

CDistributedKazaaCollectorDlg::~CDistributedKazaaCollectorDlg()
{
}

void CDistributedKazaaCollectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list_box);
	DDX_Control(pDX, IDC_PROJECT_LIST, m_project_list);
	DDX_Control(pDX, IDC_SEARCH_INTERVAL, m_search_more_interval);
}

BEGIN_MESSAGE_MAP(CDistributedKazaaCollectorDlg, CDialog)
	ON_BN_CLICKED(IDGO, OnGo)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ON_TOP_CHECK, OnBnClickedOnTopCheck)
END_MESSAGE_MAP()


// CDistributedKazaaCollectorDlg message handlers


// ****
//
//	 -=-=-=- THE REAL CODE -=-=-=-
//
// ****

//
//	Go.
//
void CDistributedKazaaCollectorDlg::OnGo()
{
	// For a manual search just use pid = 0, will get any available kazaa window.
	p_dll->Search(0);
}

//
//	Log it.
//
void CDistributedKazaaCollectorDlg::Log(const char *buf)
{
	char msg[4096];
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());

	strcat(msg,buf);
	m_list_box.InsertString(0,msg);
 
	while(m_list_box.GetCount()>1000)
	{
		m_list_box.DeleteString(m_list_box.GetCount()-1);
	}
}

void CDistributedKazaaCollectorDlg::InitParent(DistributedKazaaCollectorDll *parent)
{
	p_dll = parent;
}

void CDistributedKazaaCollectorDlg::OnBnClickedClear()
{

}


void CDistributedKazaaCollectorDlg::OnTimer(UINT nIDEvent)
{
	KillTimer(nIDEvent);

	switch (nIDEvent)
	{
	case 3:	// The trigger
		{
			// If we are reading projects, wait untill we are done.
			if (p_dll->m_com.m_reading_new_projects == false)
			{
				if (p_dll->v_worker_guy_ptrs.size() > 0)
				{
					if (m_worker_iter >= p_dll->v_worker_guy_ptrs.size())
						m_worker_iter = 0;

//					CheckForErrorBoxes();

					p_dll->v_worker_guy_ptrs[m_worker_iter++]->Trigger();
				}
			}

			SetTimer(nIDEvent, 300, NULL);
			break;
		}

	case 1:		// Update the last updated time
		{
			CTimeSpan ts;
			ts = CTime::GetCurrentTime() - m_last_update;
			CString	str = ts.Format("%H:%M:%S");

			GetDlgItem(IDC_STATIC_LAST_UPDATE)->SetWindowText(str);
			SetTimer(nIDEvent, 1000, NULL);
			break;
		}
	case 2:		// Send Results To Database:
		{
			int count = 0;
			CStdioFile file;
			CString query;
			char msg[1024];

			// Connect to the database, then open the file just long enough to do the inserts...
			MYSQL *conn;
			MYSQL_RES *res = NULL;
			MYSQL_ROW row;
			conn=mysql_init(NULL);
			
			sprintf(msg,"Dumping raw results to database");
			Log(msg);

			DWORD start=GetTickCount();	// ms since the system was started

			// Read from the file, replace the "X4XX1X" with the appropriate day number IN EACH QUERY
			if (file.Open("query_file.txt", CFile::typeText|CFile::modeRead|CFile::shareDenyWrite, NULL) != 0)
			{
				ULONGLONG size = file.GetLength();
				if (size <= 0)
				{		
					sprintf(msg,"Data Dump Timer Fired.  No queries, skipping...");
					Log(msg);
					break;
				}

				// Open the connection to the hub
				if (!mysql_real_connect(conn,"38.119.64.69","onsystems","ebertsux37","mediadefender",0,NULL,0))
				{
					char msg[1024];
					sprintf(msg,"(Failed to connect to database... skipping...");
					Log(msg);

					CString errmsg = "Failed to connect to database: Error: ";
					errmsg += mysql_error(conn);
					TRACE(errmsg);
				}
				else
				{
					// Get the table name (at least the important part, the day specific part)
					mysql_query(conn, "SELECT to_days(now())");
					res = mysql_store_result(conn);
					row=mysql_fetch_row(res);	// yearg
					CString days = row[0];
					mysql_free_result(res);		// yearg


					file.SeekToBegin();
					while (file.ReadString(query))
					{
						count++;
						query.Replace("X4XX1X", days);		// put the proper day in there.. :)

						int ret=mysql_query(conn, query);
						if(mysql_error(conn)[0] != '\0')
						{
							/*
							char log[2048];
							sprintf(log, "%s (DUMPING ERROR) %s\nError: %s\n\n", CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"), query, mysql_error(conn));
							*/
							
							/* Ivan edited */
							CString log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
							log += " (DUMPING ERROR) ";
							log += query;
							log += "\nError: ";
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
								MessageBox("File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
							}

							TRACE(log);
							if(log.GetLength() <= 4000)
								Log(log);
						}
					}

					file.Close();
					DeleteFile("query_file.txt");
				}
			}
			else
			{
				DWORD err = GetLastError();
				TRACE("Error opening 'query_file': %d\n", err);
			}

			mysql_close(conn); // Close connection to the database

			DWORD end=GetTickCount();	// ms since the system was started
			sprintf(msg,"%d Query's Dumped to Database in %.3f sec", count/2,((double)(end-start))/1000.0);
			Log(msg);

			SetTimer(nIDEvent, 1000*60*30, NULL);	// 30 minutes
			break;
		}
	default:
		{
			// should not get here, or we have a rouge timer
			break;
		}
	}
}

void CDistributedKazaaCollectorDlg::OnBnClickedOnTopCheck()
{
	CButton *check=(CButton *)GetDlgItem(IDC_ON_TOP_CHECK);
	if(check->GetCheck()==BST_CHECKED)
	{
		::SetWindowPos(GetSafeHwnd(),HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	}
	else	// BST_UNCHECKED
	{
		::SetWindowPos(GetSafeHwnd(),HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	}
}

void CDistributedKazaaCollectorDlg::CheckForErrorBoxes()
{
	// Check for those annoying kazaa lite not connnected message boxes.
	HWND box_hwnd = NULL;
	box_hwnd = ::FindWindow(NULL, "Kazaa.kpp - Application Error");

	int count = 0;
	while (box_hwnd != NULL)
	{
		if (count > 50)
			break;	// just in case...

		p_dll->m_com.ComLog("Found an application error message box. Pressing OK button...");

		HWND ok_button = ::GetWindow(box_hwnd, GW_CHILD);
		::PostMessage(ok_button, WM_LBUTTONDOWN, (WPARAM)MK_LBUTTON, (LPARAM)0x00050005);
		::PostMessage(ok_button, WM_LBUTTONUP, (WPARAM)0, (LPARAM)0x00050005);
		Log("Clicked OK on 'Application Error' messsage box");
		TRACE("Closing 'Clicked OK on 'Application Error' messsage box\n");

		p_dll->m_com.ComLog("Pressed the OK button.");

		Sleep(5000);  // wait for the message box to close
		box_hwnd = ::FindWindow(NULL, "Kazaa.kpp - Application Error");  // is there another stupid message box?

		count++;
	}
}