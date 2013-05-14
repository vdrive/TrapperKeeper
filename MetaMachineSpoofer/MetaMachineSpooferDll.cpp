#include "StdAfx.h"
#include "MetaMachineSpooferDll.h"


CMetaMachineSpooferDlg MetaMachineSpooferDll::sm_meta_dialog;
//
//
//
MetaMachineSpooferDll::MetaMachineSpooferDll(void)
{
//	mp_tcp=NULL;
	mp_system=NULL;
}

//
//
//
MetaMachineSpooferDll::~MetaMachineSpooferDll(void)
{
}

//
//
//
void MetaMachineSpooferDll::DllInitialize()
{

	//if(mp_tcp)	delete mp_tcp;
	if(mp_system) delete mp_system;
	//mp_tcp=NULL;
	mp_system=NULL;
	

	//mp_tcp=new SpooferTCPReference();  //bring tcp to life
	mp_system=new MetaSpooferReference();  //bring the system online
	sm_meta_dialog.Create(IDD_METASPOOFERDIALOG,CWnd::GetDesktopWindow());  //create the heartbeat for tcp
}

//
//
//
void MetaMachineSpooferDll::DllUnInitialize()
{
	sm_meta_dialog.DestroyWindow();  //get rid of that timer
	
	if(mp_system){
		mp_system->System()->Shutdown();
		Sleep(100);
		delete mp_system;
	}

	//if(mp_tcp) delete mp_tcp;

	//mp_tcp=NULL;
	mp_system=NULL;
}

//
//
//
void MetaMachineSpooferDll::DllStart()
{
	m_syncher_interface.Register(this,"Edonkey");
	if(mp_system){
		/*
		if(mp_tcp){

			for(int i=0;i<MULTI_CONNECTION_COUNT;i++){
				unsigned short port=BASEPORT+i;
				//we want to listen on a variety of ports
				mp_tcp->TCP()->m_servers[i]=mp_tcp->TCP()->Listen(port);
				mp_tcp->TCP()->m_server_ports[i]=port;
			}
			//mp_tcp->TCP()->server3 = mp_tcp->TCP()->Listen(4663);  //tell the one to listen
			//mp_tcp->TCP()->server4 = mp_tcp->TCP()->Listen(4664);  //tell the one to listen
			//mp_tcp->TCP()->server5 = mp_tcp->TCP()->Listen(4665);  //tell the one to listen
		}*/

		mp_system->System()->Init(this);
	}
}

//
//
//
void MetaMachineSpooferDll::DllShowGUI()
{
	sm_meta_dialog.ShowWindow(SW_NORMAL);
	sm_meta_dialog.BringWindowToTop();
}

//
//
//
void MetaMachineSpooferDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}