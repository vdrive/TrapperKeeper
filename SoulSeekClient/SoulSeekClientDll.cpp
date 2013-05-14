#include "StdAfx.h"
#include "SoulSeekClientDll.h"
//
//
//
SoulSeekClientDll::SoulSeekClientDll(void)
{
}

//
//
//
SoulSeekClientDll::~SoulSeekClientDll(void)
{
	if(p_parentCom != NULL)
	{
		delete p_parentCom;
		p_parentCom = NULL;
	}
}

//
//
//
void SoulSeekClientDll::DllInitialize()
{
	m_pa.InitParent(this);
}

//
//
//
void SoulSeekClientDll::DllUnInitialize()
{

}

//
//
//
void SoulSeekClientDll::DllStart()
{
	WSocket::Startup();  // Start winsocket
	m_dlg.Create(IDD_DIALOG1,CWnd::GetDesktopWindow());

	m_dlg.InitParent(this);
	m_dlg.EnableLoginButton();
	m_connectionManager.InitParent( this );
	
	m_pa.Run();

	p_parentCom = new ParentComInterface;
	p_parentCom->InitParent(this);
	p_parentCom->Register(this,5153);
}

//
//
//
void SoulSeekClientDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void SoulSeekClientDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

//
//
//
void SoulSeekClientDll::WriteToTextFile(string s)
{
	CFile errorLog;
	CFileException fileException;

	if ( !errorLog.Open( "C:\\SoulSeekLog.txt", CFile::modeCreate |   
			CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyWrite, &fileException ) )
	{
		TRACE( "Can't open file %s, error = %u\n",
			"C:\\SoulSeekLog.txt", fileException.m_cause );
	}

	errorLog.SeekToEnd();
	CTime currentTime = CTime::GetCurrentTime();
	char buf[512];
	sprintf(buf,"%s %s",currentTime.Format("%m/%d %H:%M:%S").GetString(),s.c_str());
	errorLog.Write(buf, (UINT)strlen(buf));

	errorLog.Close();
}
