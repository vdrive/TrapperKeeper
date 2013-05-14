// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoulSeekClientDll.h"
#include "LogDlg.h"
#include "PacketIO.h"


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

	/*unsigned char data[] = {
0x78, 0xda, 0xe3, 0x64, 0x60, 0x60, 0xc8, 0x4d, 
0x2d, 0x29, 0xca, 0x4f, 0xc9, 0x2f, 0x4a, 0x14, 
0x00, 0x72, 0x18, 0x41, 0xd8, 0x13, 0x48, 0xa4, 
0x58, 0xc5, 0x80, 0x44, 0x73, 0xd2, 0xf2, 0x63, 
0x72, 0x4b, 0x8b, 0x33, 0x93, 0x13, 0x63, 0x72, 
0xf3, 0xf3, 0x52, 0xf2, 0x15, 0x72, 0x0b, 0x8c, 
0x63, 0xbc, 0x12, 0x2b, 0x75, 0xa3, 0x14, 0x1c, 
0xf3, 0x52, 0x14, 0x7c, 0x32, 0xf3, 0xb2, 0x33, 
0xf3, 0x14, 0x02, 0x12, 0x8b, 0xb2, 0x15, 0x74, 
0x15, 0x3c, 0xab, 0xaa, 0xf2, 0x41, 0x54, 0x9e, 
0x42, 0x48, 0x46, 0xaa, 0x82, 0x6b, 0x5e, 0x8a, 
0x1e, 0x50, 0xf5, 0x96, 0x30, 0x0d, 0x06, 0x10, 
0x60, 0x06, 0x59, 0x55, 0x60, 0xcc, 0xcc, 0x00, 
0x01, 0x0d, 0x50, 0xcb, 0x96, 0x02, 0x31, 0x13, 
0x54, 0x8c, 0xb1, 0xd5, 0x94, 0x81, 0x81, 0x15, 
0xca, 0x01, 0x00, 0x3b, 0xd2, 0x21, 0x38 };

	unsigned char a[1000000];
	int uncompressedSize = 1000000;
	int status;

	// compress( returned data, returned length, original data, original length )
	status = uncompress( (Bytef *)a, (uLongf *)&uncompressedSize, (const Bytef *)data, (uLong)sizeof(data));
	
	if( status != Z_OK )
		TRACE( "Compress: Compression error(%i)\n", status );

	PacketReader pr((char *)a);

	this->WriteToLog(0,0,pr.GetNextString());

	char tempInt[10+1];
	
	itoa(pr.GetNextInt(), tempInt, 10);
	this->WriteToLog(0,0,tempInt);

	int numOfFiles = pr.GetNextInt();
	itoa(numOfFiles, tempInt, 10);
	this->WriteToLog(0,0,tempInt);

	for(int i = 0; i < numOfFiles; i++)
	{
		char tInt[10];
		itoa((int)pr.GetNextByte(), tInt, 10);   // Code
		WriteToLog(0,0,tInt);

		WriteToLog(0,0,pr.GetNextString()); // File Name

		itoa(pr.GetNextInt(), tInt, 10); // Size 1
		WriteToLog(0,0,tInt);

		itoa(pr.GetNextInt(), tInt, 10); // Size 2
		WriteToLog(0,0,tInt);

		WriteToLog(0,0,pr.GetNextString()); // Extension

		int numOfAtt = pr.GetNextInt();
		itoa(numOfAtt, tInt, 10); //Num of attributes
		WriteToLog(0,0,tInt);
		for(int j = 0; j < numOfAtt; j++)
		{
			itoa(pr.GetNextInt(), tInt, 10); // Type
			WriteToLog(0,0,tInt);

			itoa(pr.GetNextInt(), tInt, 10); // Value
			WriteToLog(0,0,tInt);
		}
	}

	itoa(pr.GetNextByte(), tempInt, 10);
	this->WriteToLog(0,0,tempInt);
	itoa(pr.GetNextInt(), tempInt, 10);
	this->WriteToLog(0,0,tempInt);
	itoa(pr.GetNextInt(), tempInt, 10);
	this->WriteToLog(0,0,tempInt);
	*/

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
