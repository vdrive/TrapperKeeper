#pragma once
#include "resource.h"		// main symbols
#include <vector>
using namespace std;

#ifdef TRAPPER_KEEPER_DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

class CDllLoaderDlg;
class DllManager;
class DLL_API DllLoaderApp
{
public:
	DllLoaderApp(void);
	~DllLoaderApp(void);
	void UpdateDllList();
	void ReloadDlls(WPARAM wparam,LPARAM lparam);


	CDllLoaderDlg* p_dlg;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DllLoaderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(DllLoaderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
private:
	DllManager* p_dll_manager;

public:
	void ShowGUI(vector<UINT>& app_ids);
	void ShowGUI(UINT& app_id);
	void OnExit();
};

