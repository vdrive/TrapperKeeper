//WatchDogControllerDestinationDll.h: Defines the initialization routines for the DLL
//
#pragma once
#include "../SamplePlugIn/Dll.h"
#include "../DllLoader/AppID.h"
#include "WatchDogControllerDestinationDlg.h"
#include "WatchDogControllerDestinationHeader.h"
#include "WatchDogComLink.h"
#include <vector>

using namespace std;

//This is my main app class name
//
class WatchDogControllerDestinationDll : public Dll
{
public:
	// Contructor
	WatchDogControllerDestinationDll(void);
	~WatchDogControllerDestinationDll(void);
	// Access Routines:
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
private:
	bool SendMessageToSource(char *dest_ip);
	// member variables:
	AppID m_app_id;		
	CWatchDogControllerDestinationDlg m_dlg;			
	WatchDogComLink *m_com_link;
public:		
	void DataReceived(char * source_name , void * data, int data_length);
	void RestartComputer(void);
};
