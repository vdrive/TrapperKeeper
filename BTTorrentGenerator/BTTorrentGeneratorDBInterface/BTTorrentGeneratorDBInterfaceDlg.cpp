// BTTorrentGeneratorDBInterfaceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTTorrentGeneratorDBInterface.h"
#include "BTTorrentGeneratorDBInterfaceDlg.h"
#include <set>
#include <string>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CBTTorrentGeneratorDBInterfaceDlg dialog

CBTTorrentGeneratorDBInterfaceDlg::CBTTorrentGeneratorDBInterfaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBTTorrentGeneratorDBInterfaceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//
//
//
void CBTTorrentGeneratorDBInterfaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MAINFRAME_TAB, m_mainTab);
}

BEGIN_MESSAGE_MAP(CBTTorrentGeneratorDBInterfaceDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//
//
BOOL CBTTorrentGeneratorDBInterfaceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_mainTab.InitDialogs();

	CRect rect;
	m_mainTab.GetClientRect( &rect );
	m_mainTab.SetMinTabWidth( rect.Width() / 5 - 1);

	m_mainTab.InsertItem( 0, "       Decoy Generator" );
	m_mainTab.InsertItem( 1, "          Cross-Namer" );
	m_mainTab.InsertItem( 2, "         Stats Modifier" );
	m_mainTab.InsertItem( 3, "       Tracker Modifier" );
	m_mainTab.InsertItem( 4, "         Hash Identifier" );

	m_mainTab.ActivateTabDialogs();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBTTorrentGeneratorDBInterfaceDlg::OnPaint() 
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
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
//
HCURSOR CBTTorrentGeneratorDBInterfaceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

