// DllLoaderDlg.cpp : implementation file
// CDllLoaderDlg dialog


#include "stdafx.h"
#include "DllLoaderDlg.h"
#include "DllLoaderApp.h"

#define WM_SHOW_GUI	WM_USER_MENU+1

//sorting columns of the dll list
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort)
{
	DWORD_PTR *params=(DWORD_PTR *)lParamSort;
	CListCtrl *list=(CListCtrl *)params[0];
	int sub_item=(int)params[1];
	bool found1=false;
	bool found2=false;
	CString cstring1,cstring2;

	switch(sub_item)
	{
		case SUB_DLL_NAME:
		{
			HWND hList = list->GetSafeHwnd();
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = lParam1;
			int index1 = ListView_FindItem(hList, -1, &lvfi);
			lvfi.lParam = lParam2;
			int index2 = ListView_FindItem(hList, -1, &lvfi);

			cstring1=list->GetItemText(index1,sub_item);
			cstring2=list->GetItemText(index2,sub_item);
			cstring1.MakeLower();
			cstring2.MakeLower();
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
		case SUB_DLL_APPID:
		case SUB_DLL_VERSION:
		{
			HWND hList = list->GetSafeHwnd();
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = lParam1;
			int index1 = ListView_FindItem(hList, -1, &lvfi);
			lvfi.lParam = lParam2;
			int index2 = ListView_FindItem(hList, -1, &lvfi);

			cstring1=list->GetItemText(index1,sub_item);
			cstring2=list->GetItemText(index2,sub_item);
			UINT val1=0xFFFFFFFF;
			UINT val2=0xFFFFFFFF;
			sscanf(cstring1,"%u",&val1);
			sscanf(cstring2,"%u",&val2);

			if(val1<val2)
			{
				return -1;
			}
			else if(val1>val2)
			{
				return 1;
			}
			return 0;
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
IMPLEMENT_DYNAMIC(CDllLoaderDlg, CDialog)
CDllLoaderDlg::CDllLoaderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDllLoaderDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CDllLoaderDlg::~CDllLoaderDlg()
{
}

void CDllLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DLL_LIST, m_dll_list);
}


BEGIN_MESSAGE_MAP(CDllLoaderDlg, CDialog)
	ON_BN_CLICKED(IDC_Show_GUI_BUTTON, OnBnClickedShowGuiButton)
//	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_DBLCLK, IDC_DLL_LIST, OnNMDblclkDllList)
	ON_NOTIFY(NM_RCLICK, IDC_DLL_LIST, OnNMRclickDllList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_DLL_LIST, OnLvnColumnclickDllList)
END_MESSAGE_MAP()


// CDllLoaderDlg message handlers
//
//
//
void CDllLoaderDlg::OnBnClickedShowGuiButton()
{
	vector<UINT>app_ids;
	
	// If none are selected, then show all
	char str_app_id[32+1];
	UINT app_id = 0;
	if(m_dll_list.GetSelectedCount()>0)
	{
		POSITION pos = m_dll_list.GetFirstSelectedItemPosition();
		if(pos != NULL)
		{
			while(pos)
			{
				int nItem = m_dll_list.GetNextSelectedItem(pos);
				m_dll_list.GetItemText(nItem,SUB_DLL_APPID,str_app_id,sizeof(str_app_id));
				sscanf(str_app_id, "%u", &app_id);
				app_ids.push_back(app_id);
			}
		}
	}
	else
	{
		for(int i=0;i<m_dll_list.GetItemCount();i++)
		{
			m_dll_list.GetItemText(i,SUB_DLL_APPID,str_app_id,sizeof(str_app_id));
			sscanf(str_app_id, "%u", &app_id);
			app_ids.push_back(app_id);
		}
	}
		
	if(app_ids.size()>0)
	{
		p_app->ShowGUI(app_ids);
	}
}

//
//
//
void CDllLoaderDlg::InitParent(DllLoaderApp* parent)
{
	p_app = parent;
}

//
//
//
void CDllLoaderDlg::OnClose()
{
	if (MessageBox("Are you sure want to close Trapper Keeper?", "Question", MB_YESNO|MB_ICONQUESTION)==IDYES)
	{
		p_app->OnExit();
		PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);
		CDialog::OnClose();
	}
}

//
//
//
void CDllLoaderDlg::OnCancel()		// user hit ESC
{
//	CDialog::OnCancel();
//	OnExit();
}

//
//
//
BOOL CDllLoaderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_dll_list.InsertColumn(SUB_DLL_NAME,"DLL Name",LVCFMT_LEFT,125);
	m_dll_list.InsertColumn(SUB_DLL_APPID,"ID",LVCFMT_CENTER,40);
	m_dll_list.InsertColumn(SUB_DLL_VERSION,"Version",LVCFMT_CENTER,80);

	CRect main_rect,list_rect;
	GetWindowRect(&main_rect);
	GetDlgItem(IDC_DLL_LIST)->GetWindowRect(&list_rect);
	m_border=list_rect.left-main_rect.left;
	m_bottom=main_rect.bottom - list_rect.bottom;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CDllLoaderDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(IsWindowVisible())
	{
		CRect main_rect,list_rect,button_rect;

		GetWindowRect(&main_rect);
		GetDlgItem(IDC_DLL_LIST)->GetWindowRect(&list_rect);

//		list_rect.bottom=main_rect.bottom-m_border-m_bottom;
		list_rect.bottom=main_rect.bottom-m_border;
		list_rect.right=main_rect.right-m_border;
/*
		button_rect.top=list_rect.bottom+15;
		button_rect.right=list_rect.right;
		button_rect.left=list_rect.left;
		button_rect.bottom=main_rect.bottom-15;
*/
		ScreenToClient(&list_rect);
//		ScreenToClient(&button_rect);

		GetDlgItem(IDC_DLL_LIST)->MoveWindow(&list_rect);
		GetDlgItem(IDC_Show_GUI_BUTTON)->MoveWindow(&button_rect);
	}
}

//
//
//
void CDllLoaderDlg::OnPaint()
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

//
//
//
HCURSOR CDllLoaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//
//
//
void CDllLoaderDlg::UpdateDllList(vector<DllInfo>& dlls)
{
	m_dll_list.DeleteAllItems();
	for(unsigned int i=0; i< dlls.size(); i++)
	{
		int item = m_dll_list.InsertItem(m_dll_list.GetItemCount(),dlls[i].m_appID.m_app_name.c_str());
		m_dll_list.SetItemData(item,dlls[i].m_appID.m_app_id);
		char appid[32+1];
		char app_version[32+1];
		sprintf(appid,"%u",dlls[i].m_appID.m_app_id);
		//print the app version in x.x.x.x format
		sprintf(app_version,"%u.%u.%u.%u",(dlls[i].m_appID.m_version>>24)&0xFF,(dlls[i].m_appID.m_version>>16)&0xFF,
			(dlls[i].m_appID.m_version>>8)&0xFF,(dlls[i].m_appID.m_version>>0)&0xFF);
		m_dll_list.SetItemText(item, SUB_DLL_APPID, appid);
		m_dll_list.SetItemText(item,SUB_DLL_VERSION,app_version);
	}
}

//
//
//
void CDllLoaderDlg::OnNMDblclkDllList(NMHDR *pNMHDR, LRESULT *pResult)
{
	POSITION pos = m_dll_list.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		int nItem = m_dll_list.GetNextSelectedItem(pos);
		UINT app_id = 0;
		char str_app_id[32+1];
		m_dll_list.GetItemText(nItem,SUB_DLL_APPID,str_app_id,sizeof(str_app_id));
		sscanf(str_app_id, "%u", &app_id);
		p_app->ShowGUI(app_id);		
	}
	
	*pResult = 0;
}

//
//
//
void CDllLoaderDlg::OnNMRclickDllList(NMHDR *pNMHDR, LRESULT *pResult)
{
	POINT point;
	GetCursorPos(&point);

	CMenu menu;
	menu.CreatePopupMenu();
	if(m_dll_list.GetSelectedCount()==0)
	{
		menu.AppendMenu(MF_STRING,WM_SHOW_GUI,"Show All GUIs");
	}
	else if(m_dll_list.GetSelectedCount()==1)
	{
		menu.AppendMenu(MF_STRING,WM_SHOW_GUI,"Show GUI");
	}
	else
	{
		menu.AppendMenu(MF_STRING,WM_SHOW_GUI,"Show GUIs");
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this,0);

	*pResult = 0;
}

//
//
//
BOOL CDllLoaderDlg::OnCommand(WPARAM wParam,LPARAM lParam)
{
	// Check for right-click popup menu items
	if(wParam>WM_USER_MENU)
	{
		switch(wParam)
		{
			case WM_SHOW_GUI:
			{
				OnBnClickedShowGuiButton();
				break;
			}
		}
	}
	
	return CDialog::OnCommand(wParam,lParam);
}

//
//
//
void CDllLoaderDlg::OnLvnColumnclickDllList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
	params[0]=(DWORD_PTR)&m_dll_list;
	params[1]=pNMLV->iSubItem;
	m_dll_list.SortItems(CompareFunc,(DWORD_PTR)params);
	*pResult = 0;
}
