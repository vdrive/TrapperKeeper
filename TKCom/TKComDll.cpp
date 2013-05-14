#include "StdAfx.h"
#include "TKComDll.h"
#include "ComDialog.h"

#include "ComService.h"

using namespace comspace;


//
//
//
TKComDll::TKComDll(void)
{
	m_system=NULL;
}

//
//
//
TKComDll::~TKComDll(void)
{
}

//
//
//
void TKComDll::DllInitialize()
{
	TRACE("TKComDll::DllInitialize() BEGIN\n");
	TRACE("TKComDll::DllInitialize() PART 1\n");
	m_system=new ComService();
	TRACE("TKComDll::DllInitialize() PART 2\n");
	g_com_dialog.Create(IDD_COM_DIALOG,CWnd::GetDesktopWindow());
	TRACE("TKComDll::DllInitialize() END\n");
}

//
//
//
void TKComDll::DllUnInitialize()
{
	if(m_system!=NULL){
		m_system->StopComService();
		delete m_system;
		m_system=NULL;
	}
	g_com_dialog.DestroyWindow();
}

//
//
//
void TKComDll::DllStart()
{
	TRACE("TKComDll::DllStart() BEGIN\n");
	m_system->StartComService(); //start all the threads.
	TRACE("TKComDll::DllStart() END\n");
}

//
//
//
void TKComDll::DllShowGUI()
{
	g_com_dialog.ShowWindow(SW_SHOWNORMAL);
	g_com_dialog.BringWindowToTop();
}

bool TKComDll::ReceivedDllData(AppID from_app_id,void* input_data,void* output_data)
{
//	_ASSERTE( _CrtCheckMemory( ) );
	if(m_system!=NULL){
		bool stat=m_system->ReceivedAppData(from_app_id.m_app_id,(byte*)input_data);
		//_ASSERTE( _CrtCheckMemory( ) );
		return stat;
	}
	else{
		return false;
	}
}