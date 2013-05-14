// TabSelection.cpp : implementation file
//

#include "stdafx.h"
#include "TabSelection.h"
#include "LogDlg.h"


// CTabSelection

IMPLEMENT_DYNAMIC(CTabSelection, CTabCtrl)

//
//
//
CTabSelection::CTabSelection()
{
	m_anDialogID[0] = IDD_LOG_DLG;
	m_apDialog[0] = new CLogDlg();
}

//
//
//
CTabSelection::~CTabSelection()
{
	for( size_t i = 0; i < CTABSELECT_TAB_COUNT; i++ )
		delete m_apDialog[i];
}

BEGIN_MESSAGE_MAP(CTabSelection, CTabCtrl)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange)
END_MESSAGE_MAP()

// CTabSelection message handlers

//
//
//
void CTabSelection::InitDialogs()
{
	for( size_t i = 0; i < CTABSELECT_TAB_COUNT; i++ )
		m_apDialog[i]->Create( m_anDialogID[i], GetParent() );
}

//
//
//
void CTabSelection::OnSelchange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	ActivateTabDialogs();
	*pResult = 0;
}

//
//
//
void CTabSelection::ActivateTabDialogs()
{
	int nSel = GetCurSel();
	if( m_apDialog[nSel]->m_hWnd )
		m_apDialog[nSel]->ShowWindow( SW_HIDE );

	CRect l_rectClient;
	CRect l_rectWnd;

	GetClientRect( l_rectClient );
	AdjustRect( FALSE, l_rectClient );
	GetWindowRect( l_rectWnd );
	GetParent()->ScreenToClient( l_rectWnd );
	l_rectClient.OffsetRect( l_rectWnd.left, l_rectWnd.top );
	for( int nCount = 0; nCount < CTABSELECT_TAB_COUNT; nCount++ )
	{
		m_apDialog[nCount]->SetWindowPos( &wndTop, l_rectClient.left , l_rectClient.top,
			l_rectClient.Width(), l_rectClient.Height() , SWP_HIDEWINDOW );
	}
	m_apDialog[nSel]->SetWindowPos( &wndTop, l_rectClient.left, l_rectClient.top,
		l_rectClient.Width(), l_rectClient.Height(), SWP_SHOWWINDOW );

	m_apDialog[nSel]->ShowWindow(SW_SHOW);
}
void CTabSelection::WriteToLog(int warningLevel, int object,LPCTSTR strToWrite)
{
	((CLogDlg*)(m_apDialog[0]))->WriteToLog(warningLevel,object,strToWrite);
}