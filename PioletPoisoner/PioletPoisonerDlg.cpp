// PioletPoisonerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PioletPoisoner.h"
#include "PioletPoisonerDlg.h"
#include "FTConnectionModule.h"
#include "..\PioletSpoofer\PoisonEntry.h"

// CPioletPoisonerDlg dialog

IMPLEMENT_DYNAMIC(CPioletPoisonerDlg, CDialog)
CPioletPoisonerDlg::CPioletPoisonerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPioletPoisonerDlg::IDD, pParent)
{
}

CPioletPoisonerDlg::~CPioletPoisonerDlg()
{
}

void CPioletPoisonerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_RICHEDIT, m_log_edit);
}


BEGIN_MESSAGE_MAP(CPioletPoisonerDlg, CDialog)
	ON_WM_TIMER()
	ON_MESSAGE(WM_NUM_FT_MODULE, OnNumFTModule)
	ON_MESSAGE(WM_FT_SOCKETS_STATUS, OnFTSocketsStatus)
	ON_MESSAGE(WM_REMOVE_FT_MODULE, OnRemoveFTModule)
	ON_MESSAGE(WM_FILE_REQUEST,OnReceivedFileRequest)
	ON_MESSAGE(WM_LISTENING_SOCKETS_STATUS,OnListeningSocketsStatus)
END_MESSAGE_MAP()


// CPioletPoisonerDlg message handlers

//
//
//
BOOL CPioletPoisonerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_newline=true;
	InitRunningSinceWindowText();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CPioletPoisonerDlg::OnTimer(UINT nIDEvent)
{
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CPioletPoisonerDlg::InitParent(PioletPoisoner* parent)
{
	p_parent = parent;
}

//
//
//
void CPioletPoisonerDlg::Log(const char *text,COLORREF color,bool bold,bool italic)
{
	if(m_newline)
	{
		char buf[1024];
		CTime time=CTime::GetCurrentTime();
		sprintf(buf,"%04u-%02u-%02u %02u:%02u:%02u - ",time.GetYear(),time.GetMonth(),time.GetDay(),time.GetHour(),time.GetMinute(),time.GetSecond());
		m_log_edit.SetSel(-1,-1);
		m_log_edit.ReplaceSel(buf);
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

	m_log_edit.SetSel(-1,-1);
	m_log_edit.SetSelectionCharFormat(cf);
	m_log_edit.ReplaceSel(text);

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
	while(m_log_edit.GetTextLength() > 1024*1024)	
	{
//		m_log_edit.SetSel(0,m_log_edit.LineLength(0)+1);
		m_log_edit.SetSel(0,-1);	// clear whole thing *&* temp kludge
		m_log_edit.SetReadOnly(FALSE);
		m_log_edit.Clear();
		m_log_edit.SetReadOnly(TRUE);
	}
}

//
//
//
void CPioletPoisonerDlg::InitRunningSinceWindowText()
{
	// Update the time that the values were last cleared
	CTime time;
	time=CTime::GetCurrentTime();
	CString time_str = time.Format("Running since %A %m/%d/%Y at %H:%M");

	char msg[1024];
	char date[32];
	sprintf(date,__DATE__);
	char m[16];
	unsigned int d;
	unsigned int y;
	sscanf(date,"%s %u %u",&m,&d,&y);
	sprintf(msg,"[ Built on %s %02u, %u at %s ]",m,d,y,__TIME__);

	time_str += (" - ");
	time_str += msg;
	SetWindowText(time_str);
}

//
//
//
LRESULT CPioletPoisonerDlg::OnNumFTModule(WPARAM wparam, LPARAM lparam)
{
	char msg[128];
	sprintf(msg, "Modules: %d   Sockets: %d", (int)wparam, (int)wparam * 60);
	GetDlgItem(IDC_MOD_STATIC)->SetWindowText(msg);
	return 0;
}

//
//
//
LRESULT CPioletPoisonerDlg::OnListeningSocketsStatus(WPARAM wparam, LPARAM lparam)
{
	char msg[128];
	sprintf(msg, "Listening Sockets: %u", (UINT)wparam);
	GetDlgItem(IDC_LISTENING_SOCKETS_STATIC)->SetWindowText(msg);
	return 0;
}

//
//
//
LRESULT CPioletPoisonerDlg::OnFTSocketsStatus(WPARAM wparam, LPARAM lparam)
{
	bool found = false;
	int total_idles = 0;
	FTConnectionModule* ftm = (FTConnectionModule*)lparam;
	for(UINT i=0; i<v_ft_connection_status.size(); i++)
	{
		if(v_ft_connection_status[i].m_mod == ftm)
		{
			found = true;
			v_ft_connection_status[i].m_num_idles = (int)wparam;
		}
		total_idles += v_ft_connection_status[i].m_num_idles;
	}
	if(!found)
	{
		FTConnectionStatus fts;
		fts.m_mod = ftm;
		fts.m_num_idles = (int)wparam;
		v_ft_connection_status.push_back(fts);
	}

	char msg[128];
	sprintf(msg, "Sockets Idle: %d   Sockets Transfering: %d", total_idles, ((int)v_ft_connection_status.size()*60)-total_idles);
	GetDlgItem(IDC_SOCKETS_STATUS_STATIC)->SetWindowText(msg);
	return 0;
}

//
//
//
LRESULT CPioletPoisonerDlg::OnRemoveFTModule(WPARAM wparam, LPARAM lparam)
{
	FTConnectionModule* ftm = (FTConnectionModule*)wparam;
	vector<FTConnectionStatus>::iterator iter= v_ft_connection_status.begin();
	while(iter!=v_ft_connection_status.end())
	{
		if( iter->m_mod == ftm)
		{
			v_ft_connection_status.erase(iter);
			break;
		}
		iter++;
	}
	return 0;
}

//
//
//
LRESULT CPioletPoisonerDlg::OnReceivedFileRequest(WPARAM wparam, LPARAM lparam)
{
	PoisonEntry* pe = (PoisonEntry*)wparam;
	p_parent->OnReceivedFileRequest(pe);
	return 0;
}
