// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainDlg.h"


// CMainDlg

IMPLEMENT_DYNAMIC(CMainDlg, CPropertySheet)
CMainDlg::CMainDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
	,m_bNeedInit(TRUE)
	,m_nMinCX(0)
	,m_nMinCY(0)

{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_KAZAA_ICON);
	AddPage(&m_supernode_distributer_page);
	AddPage(&m_racks_supernodes_page);
}

//
//
//
CMainDlg::CMainDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
	,m_bNeedInit(TRUE)
	,m_nMinCX(0)
	,m_nMinCY(0)

{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_KAZAA_ICON);
	AddPage(&m_supernode_distributer_page);
	AddPage(&m_racks_supernodes_page);
}

//
//
//
CMainDlg::~CMainDlg()
{
}


BEGIN_MESSAGE_MAP(CMainDlg, CPropertySheet)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CMainDlg message handlers
void CMainDlg::InitParent(SupernodeDistributerDll* parent)
{
	p_parent = parent;
	m_supernode_distributer_page.InitParent(parent);
	m_racks_supernodes_page.InitParent(parent);
}

//
//
//
void CMainDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}

//
//
//
void CMainDlg::OnExit(void)
{
	CPropertySheet::OnClose();
}

//
//
//
void CMainDlg::OnSize(UINT nType, int cx, int cy)
{
	CPropertySheet::OnSize(nType, cx, cy);
	CRect r1;
	if (m_bNeedInit)
		return;

	CTabCtrl *pTab = GetTabControl();
	ASSERT(NULL != pTab && IsWindow(pTab->m_hWnd));

	int dx = cx - m_rCrt.Width();
	int dy = cy - m_rCrt.Height();
	GetClientRect(&m_rCrt);

	HDWP hDWP = ::BeginDeferWindowPos(5);

	pTab->GetClientRect(&r1); 
	r1.right += dx; r1.bottom += dy;
	::DeferWindowPos(hDWP, pTab->m_hWnd, NULL,
		0, 0, r1.Width(), r1.Height(),
		SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);

	// Move all buttons with the lower right sides
	for (CWnd *pChild = GetWindow(GW_CHILD);
		pChild != NULL;
		pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		if (pChild->SendMessage(WM_GETDLGCODE) & DLGC_BUTTON)
		{
			pChild->GetWindowRect(&r1); ScreenToClient(&r1); 
			r1.top += dy; r1.bottom += dy; r1.left+= dx; r1.right += dx;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL,
				r1.left, r1.top, 0, 0,
				SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
		}
		// Resize everything else...
		else
		{
			pChild->GetClientRect(&r1); 
			r1.right += dx; r1.bottom += dy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, 0, 0, r1.Width(), r1.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		}

	}

	::EndDeferWindowPos(hDWP);

}

//
//
//
BOOL CMainDlg::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	int page_count = GetPageCount();
	//Let the sheet create each page by activing each page at the beginning
	for(int i=0; i<page_count; i++)
	{
		SetActivePage(i);
	}
	//set the active page back to the first one
	SetActivePage(0);
	
	CRect r;  GetWindowRect(&r);
	r.bottom += GetSystemMetrics(SM_CYMENU);
	MoveWindow(r);

	// Init m_nMinCX/Y
	m_nMinCX = r.Width();
	m_nMinCY = r.Height();
	// After this point we allow resize code to kick in
	m_bNeedInit = FALSE;
	GetClientRect(&m_rCrt);

	return bResult;
}

//
//
//
void CMainDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
   CPropertySheet::OnGetMinMaxInfo(lpMMI);
   lpMMI->ptMinTrackSize.x = m_nMinCX;
   lpMMI->ptMinTrackSize.y = m_nMinCY;

   CPropertySheet::OnGetMinMaxInfo(lpMMI);
}
