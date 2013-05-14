// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoulSeekServerDll.h"
#include "LogDlg.h"


// LogDlg dialog

IMPLEMENT_DYNAMIC(CLogDlg, CDialog)
CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
{
	objectNames[0] = "Root";
	objectNames[1] = "Server";
	objectNames[2] = "Parent";
	objectNames[3] = "Search";
	objectNames[4] = "Client";
	isPaused = false;
}

CLogDlg::~CLogDlg()
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_richLog);
	DDX_Control(pDX, IDC_COMBO1, m_filter_object);
	DDX_Control(pDX, IDC_COMBO2, m_filter_message_level);
	DDX_Control(pDX, IDC_BUTTON2, m_refresh);
	DDX_Control(pDX, IDC_BUTTON1, m_pause);
}

BOOL CLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_filter_object.InsertString(0,"All Threads");
	m_filter_object.InsertString(1,"Root Thread");
	m_filter_object.InsertString(2,"Server Thread");
	m_filter_object.InsertString(3,"Parent Thread");
	m_filter_object.InsertString(4,"Search Thread");
	m_filter_object.InsertString(5,"Client Thread");
	m_filter_object.SetCurSel(0);

	m_filter_message_level.InsertString(0,"View All Messages");
	m_filter_message_level.InsertString(1,"View Normal Messages");
	m_filter_message_level.InsertString(2,"View Warning Messages");
	m_filter_message_level.InsertString(3,"View Error Messages");
	m_filter_message_level.SetCurSel(0);

	return true;
}

BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedPause)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedRefresh)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeObject)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnCbnSelchangeWarningLevel)
END_MESSAGE_MAP()

void CLogDlg::OnCbnSelchangeObject()
{
	if(isPaused == false)
	{
		RefreshLog();
	}
	else
	{
		isPaused = false;
		RefreshLog();
		isPaused = true;
	}
}

void CLogDlg::OnCbnSelchangeWarningLevel()
{
	if(isPaused == false)
	{
		RefreshLog();
	}
	else
	{
		isPaused = false;
		RefreshLog();
		isPaused = true;
	}
}
void CLogDlg::OnBnClickedRefresh()
{
	if(isPaused == false)
	{
		RefreshLog();
	}
	else
	{
		isPaused = false;
		RefreshLog();
		isPaused = true;
	}
}

void CLogDlg::OnBnClickedPause()
{

	if(isPaused == false)
	{
		isPaused = true;
		m_pause.SetWindowText("Resume");
	}
	else
	{
		isPaused = false;
		RefreshLog();
		m_pause.SetWindowText("Pause");
	}
}

void CLogDlg::RefreshLog()
{
	m_richLog.SetSel(0,m_richLog.LineLength(0)+1);
    m_richLog.SetSel(0,-1);

	m_richLog.SetReadOnly(false);
	m_richLog.Clear();
	m_richLog.SetReadOnly(true);
	for(int i = 0; i < (int)logEntry.size(); i++)
	{
		WriteEntry(i);
	}
}

void CLogDlg::WriteToLog(int warningLevel, int object, LPCTSTR strToWrite)
{
	CTime currentTime = CTime::GetCurrentTime();
	logEntry.push_back(LogEntry(currentTime.Format("%m/%d %H:%M:%S").GetString(),warningLevel,object, string(strToWrite)));
	if(logEntry.size() > MAX_LOG_SIZE)
	{
		logEntry.erase(&logEntry.front()); //pop the front if the log gets too big
		if(m_richLog.GetTextLength() > 1024*1024) // if text box gets too big
		{
			RefreshLog();
			return;
		}
	}
	WriteEntry((int)logEntry.size() - 1);
}

void CLogDlg::WriteEntry(int i)
{
	if(isPaused == true)
		return;

	if(m_filter_object.GetCurSel() != 0 
			&& m_filter_object.GetCurSel() != (logEntry[i].object + 1)
		)
		return;
	if(m_filter_message_level.GetCurSel() != 0 
			&& m_filter_message_level.GetCurSel() != (logEntry[i].level + 1)
		)
		return;

	int red;
	int green;
	int blue;

	switch(logEntry[i].object)
	{
		case 1: //Blue
			{
				red = 0;
				green = 0;
				blue = 150;
				break;
			}
		case 2:
			{  //green
				red = 0;
				green = 150;
				blue = 0;
				break;
			}
		case 3:
			{  //purple
				red = 100;
				green = 0;
				blue = 100;
				break;
			}
		case 4:
			{  // Tanish Orange
				red = 255;
				green = 153;
				blue = 102;
				break;
			}
		default:
			{
				red = 0;
				green = 0;
				blue = 0;
				break;
			}
	}
	AppendString(logEntry[i].time.c_str(),&m_richLog, red, green, blue, false);
	if(logEntry[i].object < 5)
	{
		AppendString(" - ",&m_richLog, red, green, blue, false);
		AppendString(objectNames[logEntry[i].object].c_str(),&m_richLog, red, green, blue, false);
	}
	AppendString(": ",&m_richLog, red, green, blue, false);

	switch(logEntry[i].level)
	{
		case 0:
			{
				red = 0;
				green = 100;
				blue = 0;
				break;
			}
		case 1:
			{
				red = 255;
				green = 102;
				blue = 0;
				break;
			}
		case 2:
			{
				red = 255;
				green = 0;
				blue = 0;
				break;
			}
		default:
			{
				red = 0;
				green = 0;
				blue = 0;
				break;
			}
	}
	AppendString(logEntry[i].msg.c_str(),&m_richLog, red, green, blue, false);
	AppendString("\n",&m_richLog, red, green, blue, false);
}

void CLogDlg::AppendString(LPCTSTR tempString, CRichEditCtrl *rEBox, int red, int green, int blue, bool bold)
{
	rEBox->SetSel(-1,-1);  //if you wanted to append text to the end of the edit control
    CHARFORMAT cf;

    rEBox->GetSelectionCharFormat(cf);
    cf.dwMask |= CFM_COLOR;
    if(bold) cf.dwMask |= CFM_BOLD; 
	//cf.dwEffects = 0; 
	if(bold) cf.dwEffects |=CFE_BOLD;

	if(cf.dwEffects&CFE_AUTOCOLOR)//CFE_AUTOCOLOR=0x00000100
		cf.dwEffects^=CFE_AUTOCOLOR;

    cf.crTextColor = RGB(red,green,blue);                 //some color
    rEBox->SetSelectionCharFormat(cf);
    rEBox->ReplaceSel(tempString);
}
