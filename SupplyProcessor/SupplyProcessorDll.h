#pragma once
#include "Resource.h"
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "SupplyData.h"
#include "DatabaseHelper.h"
#include "SupplyProcessorTestDlg.h"
#include "afxmt.h"


class SupplyProcessorDLL :	public Dll
{
public:
	SupplyProcessorDLL(void);
	~SupplyProcessorDLL(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();


	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	void Log(string message);
	void Log(const char *message);

	bool Write(void *buf, void*supply_data); // queues up data to the database;
	bool Read(char * buf, void*output_data); // gets requested information
	bool Clear(char * buf, void*output_data); // clear DB of any items > given days
	bool ReceivedDllData(AppID from_app_id,void* input_data,void* output_data);

	bool ClearDB();
	bool Send();

	HWND m_dlg_hwnd;
protected:
	bool Write(vector<SupplyData>* supply_data); // writes to the db

	vector<SupplyData> v_supply_queue;
private:
	SupplyProcessorTestDlg m_dlg;

	DatabaseHelper m_database_helper;
	int m_clear_interval; // in days
	string m_source; // 

	CCriticalSection m_lock;
};
