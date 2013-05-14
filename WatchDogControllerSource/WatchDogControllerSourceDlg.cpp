// WatchDogControllerSourceDlg.cpp : implementation file
//
#include "stdafx.h"
//#include "afxcmn.h"
#include "WatchDogControllerSource.h"
#include "WatchDogControllerSourceDll.h"
#include "WatchDogControllerSourceDlg.h"
#include "Rack.h"


// CWatchDogControllerSourceDlg dialog

IMPLEMENT_DYNAMIC(CWatchDogControllerSourceDlg, CDialog)
CWatchDogControllerSourceDlg::CWatchDogControllerSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWatchDogControllerSourceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//
//
//
CWatchDogControllerSourceDlg::~CWatchDogControllerSourceDlg()
{
}

//
//
//
void CWatchDogControllerSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CONTROL, m_list_control);
}


BEGIN_MESSAGE_MAP(CWatchDogControllerSourceDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_CONTROL, OnLvnColumnclickListControl)
END_MESSAGE_MAP()


// CWatchDogControllerSourceDlg message handlers
//
//

// Return -1 if lParam1 < lParam2, 1 if lParam1 > lParam2, 0 if lParam1 == lParam 2
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// lParamSort is the list control pointer and column
	// lParam1 and lParam2 is the index
	DWORD_PTR *params=(DWORD_PTR *)lParamSort;
	CListCtrl *p_list=(CListCtrl *)params[0];
	int sub_item=(int)params[1];
	bool found1=false;
	bool found2=false;
	CString cstring1, cstring2;

	switch(sub_item)
	{		
		case SUB_WDC_NAME:
		{
			HWND hList = p_list->GetSafeHwnd();
			LVFINDINFO lvfi;						
			lvfi.flags = LVFI_PARAM;

			lvfi.lParam = lParam1;
			int index1 = ListView_FindItem(hList, -1, &lvfi);
			lvfi.lParam = lParam2;
			int index2 = ListView_FindItem(hList, -1, &lvfi);

			cstring1=p_list->GetItemText(index1,sub_item);
			cstring2=p_list->GetItemText(index2,sub_item);
	
			// Put empty strings at the bottom
			if(cstring1.IsEmpty())
			{
				return 1;
			}
			else if(cstring2.IsEmpty())
			{
				return -1;
			}
			else
			{
				return strcmp(cstring1,cstring2);
			}
			break;
		}
		default:
		{
			return 0;
		}
	}
}

//
//
//
void CWatchDogControllerSourceDlg::OnLvnColumnclickListControl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
	params[0]=(DWORD_PTR)&m_list_control;
	params[1]=pNMLV->iSubItem;
	m_list_control.SortItems(CompareFunc,(DWORD_PTR)params);
	*pResult = 0;
}
//
//
//
void CWatchDogControllerSourceDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	CRect rect;
	// can resize the window:
	//if(m_list_control.GetSafeHwnd())
	//{
	//	CRect newRect(5,5,cx-10,cy-50);
	//	m_list_control.MoveWindow(&newRect,TRUE);
	//}
}

//
//
//
void CWatchDogControllerSourceDlg::InitParent(WatchDogControllerSourceDll * parent)
{
	p_parent = parent;
}

//
//
//
BOOL CWatchDogControllerSourceDlg::OnInitDialog(void)
{
	CDialog::OnInitDialog();
	m_count = 0;
	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Retrieve the current time when app exe:
	m_time = CTime::GetCurrentTime();	
	RunningSince();

	// Add Header Columns
	m_list_control.SetExtendedStyle(m_list_control.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list_control.InsertColumn(SUB_WDC_NAME,	 "Rack",	LVCFMT_CENTER, 103);			
	m_list_control.InsertColumn(SUB_WDC_STATUS, "Time Since Ping",	LVCFMT_CENTER, 102);	

	return FALSE;  
}

//
//
//
void CWatchDogControllerSourceDlg::RunningSince()
{
	char buf[1024];

	int hour = m_time.GetHour();
	if (hour < 12) // 0-11 A.M
	{
		if (hour == 0) // midnight
		{
			hour = 12;
		}
		sprintf(buf, "The Watch Dog  %u/%02u - %u:%02u AM", m_time.GetMonth(), m_time.GetDay(), hour, m_time.GetMinute());
	}
	else // if PM
	{
		if (hour != 12)
		{
			hour -= 12;
		}
		sprintf(buf, "The Watch Dog  %u/%02u - %u:%02u PM", m_time.GetMonth(), m_time.GetDay(), hour, m_time.GetMinute());
	}
	SetWindowText(buf);
}

//
//
//
void CWatchDogControllerSourceDlg::OnTimer(UINT nIDEvent)
{
	CDialog::OnTimer(nIDEvent);	
	switch(nIDEvent)
	{	
		// 1st event is for duration timer to resend a ping message to desination:
		case 1:
		{
			// call from main app: WatchDogControllerSourceDll.cpp
			p_parent->OnPingTimer();
			break;
		}
	}
}

//
//
// Add ip into list
void CWatchDogControllerSourceDlg::AddDestIP(char * ip)
{
	m_list_control.InsertItem(m_list_control.GetItemCount(), ip);
}

//
//
//
void CWatchDogControllerSourceDlg::DeleteAllRacks(void)
{
	m_list_control.DeleteAllItems();
	UpdateData(TRUE);
}

//
//
//
void CWatchDogControllerSourceDlg::TotalUpRacks(int n_racks_up)
{
	char ip[32];
	_itoa(n_racks_up,ip, 10);
	sprintf(ip, "%d", n_racks_up);
	SetDlgItemText(IDC_STATIC_TEXT2, ip);
}

//
//
//
void CWatchDogControllerSourceDlg::TotalDownRacks(int n_racks_down)
{
	char ip[32];
	_itoa(n_racks_down,ip, 10);
	sprintf(ip, "%d", n_racks_down);
	SetDlgItemText(IDC_STATIC_TEXT3, ip);

	int total_racks = m_list_control.GetItemCount();
	int n_racks_up = total_racks - n_racks_down;
	TotalUpRacks(n_racks_up);
}

//
//
//
void CWatchDogControllerSourceDlg::TotalWatchRacks()
{
	char ip[32];
	int total = m_list_control.GetItemCount();
	_itoa(total, ip, 10);
	sprintf(ip, "%d", total);
	SetDlgItemText(IDC_STATIC_TEXT1, ip);
}

//
//
//
void CWatchDogControllerSourceDlg::TimeToPing(vector<Rack> &v_racks)
{
	bool found = false;
	int count = 0;
	int selected_count = m_list_control.GetItemCount();
	CString total_elasped_time;
	CTimeSpan elasped_time;
	CTime cur_time = CTime::GetCurrentTime();

	for (UINT i=0; i<v_racks.size(); i++)
	{
		for (int j=0; j<selected_count; j++)
		{
			char ip[20];
			m_list_control.GetItemText(j, SUB_WDC_NAME, ip, sizeof(ip));
			if (strcmp(v_racks[i].m_rack_name.c_str(), ip) == 0)
			{
				CTime ping_time = v_racks[i].m_last_ping_time;
				elasped_time = cur_time - ping_time;

				//char buf[1024];
				if((elasped_time.GetTotalMinutes() >= 60))	// if it has been over an hour, format it h:mm:ss
				{
					//sprintf(buf,"%u:%02u:%02u",elasped_time.GetTotalHours(),elasped_time.GetMinutes(),elasped_time.GetSeconds());
					total_elasped_time = elasped_time.Format("%H:%M:%S");
				}
				else	// else it has not been an hour yet, so format it m:ss
				{
					//sprintf(buf,"%u:%02u",elasped_time.GetMinutes(),elasped_time.GetSeconds());
					total_elasped_time = elasped_time.Format("%M:%S");
				}
				//total_elasped_time=buf;				
				m_list_control.SetItemText(j, SUB_WDC_STATUS, total_elasped_time);
				
				if (elasped_time.GetTotalMinutes() >= 5)
				{
					count++;
				}
			}
		}
	}
	TotalDownRacks(count);
}

//
//
//
void CWatchDogControllerSourceDlg::TimeFromPong(vector<Rack> &v_racks, char *new_ip)
{
	CTime curr_time = CTime::GetCurrentTime();
	for (UINT i=0; i<v_racks.size(); i++)
	{
		if (strcmp(new_ip, v_racks[i].m_rack_name.c_str()) == 0)
		{
			// update m_last_ping_time to current time
			v_racks[i].m_last_ping_time = curr_time;
			break;
		}
	}
}

//
//
//
//void CWatchDogControllerSourceDlg::SetSortArrow(const int i_sort_column, const BOOL b_sort_ascending)
//{
//	m_iSortColumn = i_sort_column;
//	m_bSortAscending = b_sort_ascending;
//
//	// change the item to owner drawn.
//	//HD_ITEM hditem;
//
//	//hditem.mask = HDI_FORMAT;
//	//VERIFY( m_list_control.GetItem( i_sort_column, &hditem ) );
//	//hditem.fmt |= HDF_OWNERDRAW;
//	//VERIFY( m_list_control.SetItem( i_sort_column, &hditem ) );
//	
//	// invalidate the header control so it gets redrawn
//	Invalidate();
//
//}
//
////
////
////
//void CWatchDogControllerSourceDlg::Sort(int i_column, bool b_ascending)
//{
//}
