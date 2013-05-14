#include "StdAfx.h"
#include "BTRemoverdll.h"
#include "Resource.h"
#include "HTTPConnector.h"

#include "TorrentSpyRemover.h"

//
//
//
BTRemoverDll::BTRemoverDll(void)
{
}

//
//
//
BTRemoverDll::~BTRemoverDll(void)
{
}

//
//
//
void BTRemoverDll::DllInitialize()
{
	m_dlg.SetDll(this);
	m_dlg.Create(IDD_BTREMOVERDLG,CWnd::GetDesktopWindow());
}

//
//
//
void BTRemoverDll::DllUnInitialize()
{

}

//
//
//
void BTRemoverDll::DllStart()
{
	m_dlg.Log("Started");
}

//
//
//
void BTRemoverDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTRemoverDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

void BTRemoverDll::SupplySynched(const char* source_ip)
{
}



string BTRemoverDll::GetSubstring(const string data, int * pos, string startStr, string endStr) 
{
	int cur_pos = *pos;
	const char * temp = data.c_str();
	int start = -1;
	if(startStr.length() == 0) start = cur_pos;
	else start = (int)data.find(startStr, cur_pos);
	if(start < 0) {
		return "";
	}
	start += (int)startStr.length();
	int len = (int)data.find(endStr, start);
	if(len < 0 || endStr.length() == 0) len = (int)data.length() - start;
	else len -= start;
	cur_pos = start + len;

	*pos = cur_pos;
	//TRACE("%s\n", data.substr(start, len).c_str());
	return data.substr(start, len);
}

UINT WebRemoveThreadProc(LPVOID p_dll)
{
	((WebRemover*)p_dll)->Remove();
	delete p_dll;
	return 1;
}

void BTRemoverDll::Collect()
{

	string query = 
		"select url from supply_data where source = 'torrentspy' and deleted = 'n'";


	m_dlg.Log("Gathering data");

	v_ids.clear();

	SQLInterface sql;
	MYSQL conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	sql.OpenDB(&conn, "206.161.141.35", "bittorrent_data");
	res = sql.Query(&conn, query);

	if(res != NULL)
	{
		int col_count =  (int) mysql_num_fields(res);
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{	
				int pos = 0;
				string url = row[0];
				string id = GetSubstring(url, &pos, "=", "");
				v_ids.push_back(id);
				TRACE("id:%s\n", id.c_str());
			}
		}
		mysql_free_result(res);			// free's mem (::)
	} 

	v_search.clear();
	// **** potential memory leak *****
	v_search.push_back(new TorrentSpyRemover());


	for(unsigned int i = 0; i < v_search.size(); i++) {
		v_search[i]->Init(&v_ids, &m_dlg.m_main_list_ctrl);
		AfxBeginThread(WebRemoveThreadProc,(LPVOID)v_search[i],THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	}
	v_search.clear();
}