#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "SamplePlugInDlg.h"
#include "BTCollectorComInterface.h"
#include "BTCollectorDlg.h"
#include "ClientData.h"


class BTDataDll :	public Dll
{
public:
	BTDataDll(void);
	~BTDataDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void TimerFired(UINT nIDEvent);
	void SendDataRequests();

	void ParseClientData(char * source_name, void * data, int data_length);
	void CalculateData();

private:

	BTCollectorComInterface * p_com;
	BTCollectorDlg m_dlg;

	vector <string> v_data_map;
	vector <double> v_time;
	
	vector<vector<ClientData> > v_client_data;

};
