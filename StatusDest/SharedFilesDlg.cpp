// SharedFilesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SharedFilesDlg.h"

#define SUB_PATH			0
#define SUB_FILENAME		1
#define SUB_FILE_SIZE		2
#define SUB_DATE_CREATED	3

// CSharedFilesDlg dialog

IMPLEMENT_DYNAMIC(CSharedFilesDlg, CDialog)
CSharedFilesDlg::CSharedFilesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSharedFilesDlg::IDD, pParent)
{
	m_parent_hwnd = NULL;
}

CSharedFilesDlg::~CSharedFilesDlg()
{
}

void CSharedFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILES_LIST, m_files_list);
}


BEGIN_MESSAGE_MAP(CSharedFilesDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_FILES_LIST, OnLvnColumnclickFilesList)
END_MESSAGE_MAP()


// CSharedFilesDlg message handlers

void CSharedFilesDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	::PostMessage(m_parent_hwnd, WM_SHARED_FILES_DLG_QUIT, (WPARAM)GetSafeHwnd(),0);
	//CDialog::OnCancel();
}

//
//
//
BOOL CSharedFilesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_files_list.SetExtendedStyle(m_files_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	CRect rect;
	m_files_list.GetWindowRect(&rect);
	
	int nWidth = rect.Width()-20;

	m_files_list.InsertColumn(SUB_PATH,"Path",LVCFMT_LEFT, nWidth * 45/100);
	m_files_list.InsertColumn(SUB_FILENAME,"Filename",LVCFMT_LEFT, nWidth * 35/100);
	m_files_list.InsertColumn(SUB_FILE_SIZE,"Size",LVCFMT_LEFT, nWidth * 7/100);
	m_files_list.InsertColumn(SUB_DATE_CREATED,"Date Created",LVCFMT_LEFT, nWidth * 13/100);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CSharedFilesDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(IsWindowVisible())
	{
		CRect main_rect,list_rect;
		GetWindowRect(&main_rect);
		GetDlgItem(IDC_FILES_LIST)->GetWindowRect(&list_rect);
		
		int border=list_rect.left-main_rect.left;

		list_rect.bottom=main_rect.bottom-border;
		list_rect.right=main_rect.right-border;
		ScreenToClient(&list_rect);
		GetDlgItem(IDC_IP_LIST)->MoveWindow(&list_rect);
		
		CRect rect;
		m_files_list.GetWindowRect(&rect);
		int nWidth = rect.Width() - 20;

		m_files_list.SetColumnWidth(SUB_PATH, nWidth * 45/100);
		m_files_list.SetColumnWidth(SUB_FILENAME, nWidth * 35/100);
		m_files_list.SetColumnWidth(SUB_FILE_SIZE, nWidth * 10/100);
		m_files_list.SetColumnWidth(SUB_DATE_CREATED, nWidth * 10/100);
	}
}

//
//
//
void CSharedFilesDlg::ReceivedSharedFilesReply(FileInfos& reply)
{
	m_files_list.DeleteAllItems();
	for(UINT i=0; i<reply.v_files.size(); i++)
	{
		int index = m_files_list.InsertItem(m_files_list.GetItemCount(), reply.v_files[i].m_path);
		m_files_list.SetItemData(index, index);
		m_files_list.SetItemText(index, SUB_FILENAME, reply.v_files[i].m_filename);
		char size[32];
//		UINT filesize = reply.v_files[i].m_file_size;
		sprintf(size, "%d KB", reply.v_files[i].m_file_size / 1024);
		m_files_list.SetItemText(index, SUB_FILE_SIZE, size);
		CString date = reply.v_files[i].m_creation_date.Format("%m/%d/%Y %H:%M");
		m_files_list.SetItemText(index, SUB_DATE_CREATED, date);
	}
}

//
//
//
int CALLBACK CompareSharedFilesFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort)
{
	// lParamSort is the list control pointer and column
	// lParam1 and lParam2 is the index

	DWORD_PTR *params=(DWORD_PTR *)lParamSort;
	CListCtrl *list=(CListCtrl *)params[0];
	int sub_item=(int)params[1];

	bool found1=false;
	bool found2=false;
	CString cstring1,cstring2;

	HWND hList = list->GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = lParam1;
	int index1 = ListView_FindItem(hList, -1, &lvfi);
	lvfi.lParam = lParam2;
	int index2 = ListView_FindItem(hList, -1, &lvfi);
	cstring1=list->GetItemText(index1,sub_item);
	cstring2=list->GetItemText(index2,sub_item);

	// Make then both lowercase
	cstring1.MakeLower();
	cstring2.MakeLower();

	switch(sub_item)
	{
		case SUB_PATH:
		case SUB_FILENAME:
		case SUB_DATE_CREATED:
		{
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
		}
		case SUB_FILE_SIZE:
		{
			float val1=0x00FFFFFF;	// very big
			float val2=0x00FFFFFF;	// very big
			sscanf(cstring1,"%f",&val1);
			sscanf(cstring2,"%f",&val2);

			if(val1<val2)
			{
				return -1;
			}
			else if(val1>val2)
			{
				return 1;
			}

			return 0;
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
void CSharedFilesDlg::OnLvnColumnclickFilesList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
	params[0]=(DWORD_PTR)&m_files_list;
	params[1]=pNMLV->iSubItem;
	m_files_list.SortItems(CompareSharedFilesFunc,(DWORD_PTR)params);
	*pResult = 0;
}
