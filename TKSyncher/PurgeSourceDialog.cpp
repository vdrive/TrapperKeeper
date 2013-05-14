// PurgeSourceDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PurgeSourceDialog.h"
#include "TKSyncherInterface.h"
#include "DllInterface.h"


// CPurgeSourceDialog dialog

IMPLEMENT_DYNCREATE(CPurgeSourceDialog, CDialog)

CPurgeSourceDialog::CPurgeSourceDialog(CWnd* pParent /*=NULL*/)
: CDialog(CPurgeSourceDialog::IDD,pParent)
{
}

CPurgeSourceDialog::~CPurgeSourceDialog()
{
}

void CPurgeSourceDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPSTART, m_ip_start);
	DDX_Control(pDX, IDC_IPEND, m_ip_end);
	DDX_Control(pDX, IDC_SOURCE_NAME, m_source_name);
}

BOOL CPurgeSourceDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	//this->Navigate("http://www.microsoft.com");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CPurgeSourceDialog, CDialog)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_Purge, OnPurge)
	ON_BN_CLICKED(IDC_TOTAL_ANNIHILATION, OnTotalAnnihilation)
	ON_BN_CLICKED(IDC_CLEAN_DIRECTORY, OnCleanDirectory)
	ON_BN_CLICKED(IDC_DELETE_FILE, OnDeleteFile)
	ON_BN_CLICKED(IDC_RUN_PROGRAM, OnBnClickedRunProgram)
END_MESSAGE_MAP()

// CPurgeSourceDialog message handlers

void CPurgeSourceDialog::OnCancel()
{
	CDialog::OnCancel();
}

void CPurgeSourceDialog::OnPurge()
{
	DWORD start_ip;
	m_ip_start.GetAddress(start_ip);
	DWORD end_ip;
	m_ip_end.GetAddress(end_ip);

	if(end_ip<start_ip){
		MessageBox("The upper boundary must actually be greater than or equal to the lower boundry.  You will have to think harder and/or type better.","Wait right there.",MB_OK);
		return;
	}

	
	CString address;
	TKSyncherInterface::IPInterface syncher_talk;
	CString source_name;
	m_source_name.GetWindowText(source_name);  
	syncher_talk.v_strings.push_back(string((LPCSTR)source_name));  //push our sourcename onto the stack of the ip_interface
	syncher_talk.m_type=syncher_talk.PURGESOURCE;  //set the type of message to a purge message

	byte buf[1024];
	UINT buf_len=syncher_talk.WriteToBuffer(buf);  //put our message into a raw byte array

	//enumerate the ips and broadcast the message to purge to each of them
	for(UINT i=start_ip;i<=end_ip;i++){
		address.Format("%d.%d.%d.%d",((i>>24)&255),((i>>16)&255),((i>>8)&255),((i)&255));	
		m_com.SendReliableData((char*)(LPCSTR)address,buf,buf_len);
	}
}

void CPurgeSourceDialog::SetDLL(Dll* p_dll)
{
	m_com.Register(p_dll,DllInterface::GetAppID().m_app_id);
}

void CPurgeSourceDialog::OnTotalAnnihilation()
{
	DWORD start_ip;
	m_ip_start.GetAddress(start_ip);
	DWORD end_ip;
	m_ip_end.GetAddress(end_ip);

	if(end_ip<start_ip){
		MessageBox("The upper boundary must actually be greater than or equal to the lower boundry.  You will have to think harder and/or type better.","Wait right there.",MB_OK);
		return;
	}

	
	CString address;
	TKSyncherInterface::IPInterface syncher_talk;
	CString source_name;
	m_source_name.GetWindowText(source_name);  
	syncher_talk.v_strings.push_back(string((LPCSTR)source_name));  //push our sourcename onto the stack of the ip_interface
	syncher_talk.m_type=syncher_talk.TOTALANNIHILATION;  //set the type of message to a purge message

	byte buf[1024];
	UINT buf_len=syncher_talk.WriteToBuffer(buf);  //put our message into a raw byte array

	//enumerate the ips and broadcast the message to purge to each of them
	for(UINT i=start_ip;i<=end_ip;i++){
		address.Format("%d.%d.%d.%d",((i>>24)&255),((i>>16)&255),((i>>8)&255),((i)&255));	
		m_com.SendReliableData((char*)(LPCSTR)address,buf,buf_len);
	}
}

void CPurgeSourceDialog::OnCleanDirectory()
{
	DWORD start_ip;
	m_ip_start.GetAddress(start_ip);
	DWORD end_ip;
	m_ip_end.GetAddress(end_ip);

	if(end_ip<start_ip){
		MessageBox("The upper boundary must actually be greater than or equal to the lower boundry.  You will have to think harder and/or type better.","Wait right there.",MB_OK);
		return;
	}

	
	CString address;
	TKSyncherInterface::IPInterface syncher_talk;
	CString source_name;
	m_source_name.GetWindowText(source_name);  
	syncher_talk.v_strings.push_back(string((LPCSTR)source_name));  //push our sourcename onto the stack of the ip_interface
	syncher_talk.m_type=TKSyncherInterface::IPInterface::CLEANDIRECTORY;  //set the type of message to a purge message

	byte buf[1024];
	UINT buf_len=syncher_talk.WriteToBuffer(buf);  //put our message into a raw byte array

	//enumerate the ips and broadcast the message to purge to each of them
	for(UINT i=start_ip;i<=end_ip;i++){
		address.Format("%d.%d.%d.%d",((i>>24)&255),((i>>16)&255),((i>>8)&255),((i)&255));	
		m_com.SendReliableData((char*)(LPCSTR)address,buf,buf_len);
	}
}

void CPurgeSourceDialog::OnDeleteFile()
{
	DWORD start_ip;
	m_ip_start.GetAddress(start_ip);
	DWORD end_ip;
	m_ip_end.GetAddress(end_ip);

	if(end_ip<start_ip){
		MessageBox("The upper boundary must actually be greater than or equal to the lower boundry.  You will have to think harder and/or type better.","Wait right there.",MB_OK);
		return;
	}

	
	CString address;
	TKSyncherInterface::IPInterface syncher_talk;
	CString source_name;
	m_source_name.GetWindowText(source_name);  
	syncher_talk.v_strings.push_back(string((LPCSTR)source_name));  //push our sourcename onto the stack of the ip_interface
	syncher_talk.m_type=TKSyncherInterface::IPInterface::DELETEFILE;  //set the type of message to a purge message

	byte buf[1024];
	UINT buf_len=syncher_talk.WriteToBuffer(buf);  //put our message into a raw byte array

	//enumerate the ips and broadcast the message to purge to each of them
	for(UINT i=start_ip;i<=end_ip;i++){
		address.Format("%d.%d.%d.%d",((i>>24)&255),((i>>16)&255),((i>>8)&255),((i)&255));	
		m_com.SendReliableData((char*)(LPCSTR)address,buf,buf_len);
	}
}

void CPurgeSourceDialog::OnBnClickedRunProgram()
{
	DWORD start_ip;
	m_ip_start.GetAddress(start_ip);
	DWORD end_ip;
	m_ip_end.GetAddress(end_ip);

	if(end_ip<start_ip){
		MessageBox("The upper boundary must actually be greater than or equal to the lower boundry.  You will have to think harder and/or type better.","Wait right there.",MB_OK);
		return;
	}

	
	CString address;
	TKSyncherInterface::IPInterface syncher_talk;
	CString source_name;
	m_source_name.GetWindowText(source_name);  
	syncher_talk.v_strings.push_back(string((LPCSTR)source_name));  //push our sourcename onto the stack of the ip_interface
	syncher_talk.m_type=TKSyncherInterface::IPInterface::RUNPROGRAM;  //set the type of message to a purge message

	byte buf[1024];
	UINT buf_len=syncher_talk.WriteToBuffer(buf);  //put our message into a raw byte array

	//enumerate the ips and broadcast the message to purge to each of them
	for(UINT i=start_ip;i<=end_ip;i++){
		address.Format("%d.%d.%d.%d",((i>>24)&255),((i>>16)&255),((i>>8)&255),((i)&255));	
		m_com.SendReliableData((char*)(LPCSTR)address,buf,buf_len);
	}
}
