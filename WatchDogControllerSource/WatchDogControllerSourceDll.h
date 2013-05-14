//WatchDogControllerSourceDll.h: Defines the initialization routines for the DLL
//
#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "WatchDogControllerSourceDlg.h"
#include "WatchDogControllerSourceHeader.h"
#include "WatchDogComLink.h"
//#include "WatchDogNameServerInterface.h"
#include "../NameServer/NameServerInterface.h"
#include "Rack.h"

//class WatchDogNameServerInterface;
using namespace std;

//This is my main app class name
//
class WatchDogControllerSourceDll :	public Dll
{
public:
	// Contructor
	WatchDogControllerSourceDll(void);
	~WatchDogControllerSourceDll(void);
	// Access Routines:
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
private:	
	// member variables:
	AppID m_app_id;		
	int m_count;
	vector<string> v_temp_ip_list;
	CWatchDogControllerSourceDlg m_dlg;			
	WatchDogComLink *m_com_link;
	NameServerInterface m_name_server;
	vector<Rack> v_racks;
	int m_ping_interval;
	//WatchDogNameServerInterface m_name_server;

public:	
	vector<string> v_ip_list;	
	bool SendMessageToDest(char * ip);
	void OnPingTimer();
	void DataReceived(char *source_name, void *data, int data_length);	
	bool SendControllerMessage(char * dest, WatchDogControllerSourceHeader::op_code new_op_code);
	void AddNewIP(char * ip);
	void AddNewIPS(char * new_ip);
	void ReceivedIPs(vector<string> & ips);	
};
