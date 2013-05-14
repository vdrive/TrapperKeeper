//DllInterface.h
///////////////////////////////////////////////////////////////////////////
/* 
//	Purpose: This is the base class serves as the interface between DllLoader
//			 and your application.
//
//////////////////////////////////////////////////////////////////////////*/
#pragma once
#include "../DllLoader/AppID.h"
#include "DCScannerDll.h"	//This is your main app class

class DllInterface
{
public:
	DllInterface(void);
	~DllInterface(void);

	static HINSTANCE m_hinst;	//stores the HINSTANCE of DllLoader.dll
	static AppID m_app_id;			//your application ID and info

///////////////////////////////////////////////////////////////////////////////
/*
//	The following functions are exported from DllLoader
//
//////////////////////////////////////////////////////////////////////////////*/

	//Send the app data to app_id
	static bool	SendData(AppID to_app_id, void* input_data = NULL,void* output_data = NULL);

	//Asks DllLoader to reload DLLs
	static void ReloadDlls(vector<AppID>& apps);
///////////////////////////////////////////////////////////////////////////////////////
/*  
//	Must have the following functions exported explicitly in .def file
//  and DllLoader will call these functions when it needs to
//
//////////////////////////////////////////////////////////////////////////////////////*/

	//This is the first function DllLoader will call when DllLoader is starting up
	//The first parameter is the path of DllLoader.dll so we can load the dll up
	static bool InitDll(string path);

	//DllLoader calls this function when it is safe to detach DllLoader.dll
	static void UnInitialize();

	//DllLoader calls this function when it received the app data from the app
	static bool ReceivedData(AppID from_app_id, void* input_data, void* output_data);

	//Returns the AppID of this application
	static AppID GetAppID();

	//DllLoader calls this main function to start running the this app
	static void Start();

	//DllLoaders calls this function to show the GUI of this app
	static void ShowGUI();

protected:
	static DCScannerDll m_the_app; //This is the main class of your application
								   //Change it to your main app class instead of SamplePlugIn class here
};
