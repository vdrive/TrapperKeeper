// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LogDlg.h"

#define URL_MENU_COMMAND_OFFSET 50000	// look to resource.h for good starting point for this
#define MENU_CLEAR				URL_MENU_COMMAND_OFFSET+1

// CLogDlg dialog

IMPLEMENT_DYNAMIC(CLogDlg, CDialog)
CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
{
}

CLogDlg::~CLogDlg()
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Rich_Edit, m_edit);
}


BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
END_MESSAGE_MAP()

//
//
//
BOOL CLogDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_newline=true;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// CLogDlg message handlers
//
// User hit ESC
//
void CLogDlg::OnCancel()
{
	// Hide window
	ShowWindow(SW_HIDE);
}

//
//
//
void CLogDlg::Log(const char *text,COLORREF color,bool bold,bool italic)
{
	// Write out the current date and time if the last line is finished
	if(m_newline)
	{
		char buf[1024];
		CTime time=CTime::GetCurrentTime();
		sprintf(buf,"%04u-%02u-%02u %02u:%02u:%02u - ",time.GetYear(),time.GetMonth(),time.GetDay(),time.GetHour(),time.GetMinute(),time.GetSecond());
		m_edit.SetSel(-1,-1);
		m_edit.ReplaceSel(buf);
	}

	CHARFORMAT cf;
	memset(&cf,0,sizeof(CHARFORMAT));
	cf.cbSize=sizeof(CHARFORMAT);

	cf.dwMask|=CFM_COLOR;
	cf.crTextColor=color;

	cf.dwMask|=CFM_BOLD;
	if(bold)
	{
		cf.dwEffects|=CFE_BOLD;
	}

	cf.dwMask|=CFM_ITALIC;
	if(italic)
	{
		cf.dwEffects|=CFE_ITALIC;
	}

	m_edit.SetSel(-1,-1);
	m_edit.SetSelectionCharFormat(cf);
	m_edit.ReplaceSel(text);

	// If the text ended with a new line, then this is the end of this line, and the next time we log something, it will be on a new line
	m_newline=false;
	if(strrchr(text,'\n')!=NULL)
	{
		if(strcmp(strrchr(text,'\n'),"\n")==0)
		{
			m_newline=true;
		}
	}

	// Check to see if we need to free some memory if the edit's text is larger than 1 MB
	while(m_edit.GetTextLength() > 1024*1024)	
	{
//		m_edit.SetSel(0,m_edit.LineLength(0)+1);
		m_edit.SetSel(0,-1);	// clear whole thing *&* temp kludge
		m_edit.SetReadOnly(FALSE);
		m_edit.Clear();
		m_edit.SetReadOnly(TRUE);
	}
}

//
//
//
void CLogDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// Get where I right-clicked
	POINT thispoint;
	::GetCursorPos(&thispoint);

	CMenu url_menu;
	url_menu.CreatePopupMenu();

	url_menu.AppendMenu(MF_STRING,MENU_CLEAR,"Clear");

	url_menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,thispoint.x,thispoint.y,this);
	url_menu.DestroyMenu();	

	CDialog::OnRButtonUp(nFlags, point);
}

//
//
//
BOOL CLogDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD notification_code=HIWORD(wParam);
	WORD id=LOWORD(wParam);
	DWORD control=(DWORD)lParam;
	
	// Check for menu selection
	if((notification_code==0) && (control==0))
	{
		if(id>URL_MENU_COMMAND_OFFSET)
		{
			switch(id)
			{
				case MENU_CLEAR:
				{
					// Clear the edit
					m_edit.SetSel(0,-1);
					m_edit.SetReadOnly(FALSE);
					m_edit.Clear();
					m_edit.SetReadOnly(TRUE);
					break;
				}
				default:
				{
					// Do nothing
					break;
				}
			}
		}
	}
	
	return CDialog::OnCommand(wParam, lParam);
}