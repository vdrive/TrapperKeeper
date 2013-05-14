#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "BTTrackerCheckerDlg.h"
#include "ScrapeData.h"
#include "SQLInterface.h"

class BTTrackerCheckerDll :	public Dll
{
public:
	BTTrackerCheckerDll(void);
	~BTTrackerCheckerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void SupplySynched(const char* source_ip);

	bool RecheckTracker();
	bool CheckTracker();
	bool CheckTracker(string query);
	void UpdateTrackerStatus(vector<ScrapeData> * v_scrape_data, vector<string> * v_zero_seed);
	string GetScrapeURL(string tracker, string hash);
	string GetInsertString(string value);

	int Find(string target, vector<string> * vect, int start=0);

	bool OpenDB(MYSQL *conn); 
	void CloseDB(MYSQL *rs); 
	bool Execute(MYSQL * conn, string query);
	MYSQL_RES * Query(MYSQL * conn, string query);

	void UpdateFileSize(vector<string> *v_hash_id, MYSQL *conn);


private:
	BTTrackerCheckerDlg m_dlg;
};
