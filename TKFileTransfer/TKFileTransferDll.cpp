#include "StdAfx.h"
#include "TKFileTransferDll.h"
#include "DllInterface.h"
#include "FileTransferDialog.h"
using namespace filetransferspace;
//
//
//
TKFileTransferDll::TKFileTransferDll(void)
{
	//m_system=NULL;
	//mp_tcp=NULL;
	
}

//
//
//
TKFileTransferDll::~TKFileTransferDll(void)
{

	
}

//
//
//
void TKFileTransferDll::DllInitialize()
{

//	if(mp_tcp)	delete mp_tcp;
//	mp_tcp=NULL;

//	mp_tcp=new FileTransferTCPReference();  //bring tcp to life
}

//
//
//
void TKFileTransferDll::DllUnInitialize()
{
	if(g_file_transfer_dialog.GetSafeHwnd())
		g_file_transfer_dialog.DestroyWindow();

//	if(mp_tcp){ 
//		delete mp_tcp;
//		mp_tcp=NULL;
//	}
}

//
//
//
void TKFileTransferDll::DllStart()
{
	AfxInitRichEdit();
	g_file_transfer_dialog.SetDll(this);
	g_file_transfer_dialog.Create(IDD_FILETRANSFERDIALOG,CWnd::GetDesktopWindow());

//	if(mp_tcp){
//		mp_tcp->TCP()->Listen(4662);  //tell the one to listen
//	}

}

//
//
//
void TKFileTransferDll::DllShowGUI()
{
	g_file_transfer_dialog.ShowWindow(SW_SHOWNORMAL);
	g_file_transfer_dialog.BringWindowToTop();
}

bool TKFileTransferDll::ReceivedDllData(AppID from_app_id,void* input_data,void* output_data)
{
	g_file_transfer_dialog.WriteToLog("TKFileTransferDll::ReceivedDllData() BEGIN ReceivedAppData.");
	bool stat= g_file_transfer_dialog.ReceivedAppData(from_app_id.m_app_id,(byte*)input_data);
	g_file_transfer_dialog.WriteToLog("TKFileTransferDll::ReceivedDllData() END ReceivedAppData.");
	return stat;
}