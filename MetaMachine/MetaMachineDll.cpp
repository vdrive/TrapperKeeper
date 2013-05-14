#include "StdAfx.h"
#include "metamachinedll.h"

#include "MetaMachineDlg.h"

CMetaMachineDlg MetaMachineDll::sm_meta_dialog;

//
//
//
MetaMachineDll::MetaMachineDll(void)
{
	mp_tcp=NULL;
	mp_system=NULL;
}

//
//
//
MetaMachineDll::~MetaMachineDll(void)
{
}

//
//
//
void MetaMachineDll::DllInitialize()
{
	if(mp_tcp)	delete mp_tcp;
	if(mp_system) delete mp_system;
	mp_tcp=NULL;
	mp_system=NULL;
	mp_tcp=new TCPReference();  //bring tcp to life
	mp_system=new MetaSystemReference();  //bring the system online
	mp_tcp->TCP()->Listen(4661);  //tell the one to listen

	sm_meta_dialog.Create(IDD_METAMACHINEDIALOG,CWnd::GetDesktopWindow());  //create the heartbeat for tcp
}

//
//
//
void MetaMachineDll::DllUnInitialize()
{
	if(mp_system){
		mp_system->System()->CleanUp();
	}
	sm_meta_dialog.DestroyWindow();  //get rid of that timer
	if(mp_tcp)	delete mp_tcp;  //kill tcp
	if(mp_system) delete mp_system;  //kill the system
	mp_tcp=NULL;
	mp_system=NULL;
}

//
//
//
void MetaMachineDll::DllStart()
{
	m_com.Register(this,35);
	if(mp_system){
		mp_system->System()->StartThread();
	}
}

//
//
//
void MetaMachineDll::DllShowGUI()
{
	sm_meta_dialog.ShowWindow(SW_NORMAL);
	sm_meta_dialog.BringWindowToTop();
}

//
//
//
void MetaMachineDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

CMetaMachineDlg* MetaMachineDll::GetDlg(void)
{
	return &sm_meta_dialog;
}
