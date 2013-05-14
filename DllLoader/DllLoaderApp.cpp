#include "StdAfx.h"
#include "dllloaderapp.h"
#include "DllLoaderDlg.h"
#include "DllManager.h"
//#include "AppIDs.h"
DllLoaderApp::DllLoaderApp(void)
{
	p_dlg = new CDllLoaderDlg;
	p_dlg->Create(IDD_TRAPPER_KEEPER_DIALOG,CWnd::GetDesktopWindow());
	p_dll_manager = new DllManager;
	p_dll_manager->InitParent(this);
}

//
//
//
DllLoaderApp::~DllLoaderApp(void)
{
	if(p_dlg!=NULL)
	{
		p_dlg->DestroyWindow();
		delete p_dlg;
	}
	delete p_dll_manager;
}

//
//
//
BOOL DllLoaderApp::InitInstance()
{
	AfxInitRichEdit();	// to support Rich Edit Controls in Dialog boxes

	p_dlg->InitParent(this);
	p_dlg->ShowWindow(SW_NORMAL);
	p_dll_manager->LoadAllDlls();
	p_dlg->UpdateDllList(p_dll_manager->v_dlls);
	return TRUE;
}

//
//
//
void DllLoaderApp::ShowGUI(vector<UINT>& app_ids)
{
	p_dll_manager->ShowGUI(app_ids);
}

//
//
//
void DllLoaderApp::ShowGUI(UINT& app_id)
{
	p_dll_manager->ShowGUI(app_id);
}

//
//
//
void DllLoaderApp::OnExit()
{
	p_dll_manager->FreeAllDlls();
}

//
//
//
void DllLoaderApp::UpdateDllList()
{
	p_dlg->UpdateDllList(p_dll_manager->v_dlls);
}

//
//
//
void DllLoaderApp::ReloadDlls(WPARAM wparam,LPARAM lparam)
{
	//AppIDs* app_ids = (AppIDs*)wparam;

	//vector<AppID> apps = app_ids->v_app_ids;
	//delete app_ids; //free memory
	p_dll_manager->ReloadDlls();
}

