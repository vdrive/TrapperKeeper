#include "StdAfx.h"
#include "BTSearcherdll.h"
#include "Resource.h"
#include "HTTPConnector.h"

#include "TorrentSpySearcher.h"
#include "IsoHuntSearcher.h"
#include "MiniNovaSearcher.h"
#include "PirateBaySearcher.h"
#include "TorrentReactorSearcher.h"
#include "MyBittorrentSearcher.h"
#include "TorrentBoxSearcher.h"
#include "TorrentPortalSearcher.h"

//
//
//
BTSearcherDll::BTSearcherDll(void)
{
}

//
//
//
BTSearcherDll::~BTSearcherDll(void)
{
}

//
//
//
void BTSearcherDll::DllInitialize()
{
	m_dlg.SetDll(this);
	m_dlg.Create(IDD_BTSEARCHERDLG,CWnd::GetDesktopWindow());
}

//
//
//
void BTSearcherDll::DllUnInitialize()
{

}

//
//
//
void BTSearcherDll::DllStart()
{
	m_dlg.Log("Started");
}

//
//
//
void BTSearcherDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTSearcherDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

void BTSearcherDll::SupplySynched(const char* source_ip)
{
}



string BTSearcherDll::GetSubstring(const string data, int * pos, string startStr, string endStr) 
{
	int cur_pos = *pos;
	const char * temp = data.c_str();
	int start = (int)data.find(startStr, cur_pos);
	if(start < 0) {
		return "";
	}
	start += (int)startStr.length();
	int len = (int)data.find(endStr, start);
	if(len < 0) len = (int)data.length() - start;
	else len -= start;
	cur_pos = start + len;

	*pos = cur_pos;
	//TRACE("%s\n", data.substr(start, len).c_str());
	return data.substr(start, len);
}

UINT WebSearchThreadProc(LPVOID p_dll)
{
	((WebSearcher*)p_dll)->Collect();
	delete p_dll;
	return 1;
}

void BTSearcherDll::Collect()
{
	ProjectManager pm;
	//string owner = "Fox";
	string owner = "Universal Music";
	//string owner = "Universal Pictures";

	string query = 
		"select search_string, search_type, id, owner, size_threashold from projects p ";
	query += 
		//"where (owner like 'Fox%' or owner like 'Universal Music%' or owner like 'Universal Pictures%' ) ";
		"where owner like '"+owner+"%' ";
	query += "and active = 'A' order by id";


	pm.SetQuery(query);
	if(v_project_data.size() == 0) {
		m_dlg.Log("Getting Project Data");
		pm.GetProjectData(&v_project_data);
		m_dlg.Log("Gathering data");
	}
	
	v_search.clear();
	// **** potential memory leak *****
	v_search.push_back(new TorrentSpySearcher());
	v_search.push_back(new IsoHuntSearcher());
	v_search.push_back(new MiniNovaSearcher());
	v_search.push_back(new PirateBaySearcher());
	v_search.push_back(new TorrentBoxSearcher());
	v_search.push_back(new TorrentPortalSearcher());
	v_search.push_back(new TorrentReactorSearcher());

	//v_search.push_back(new MyBittorrentSearcher());

	for(unsigned int i = 0; i < v_search.size(); i++) {
		v_search[i]->Init(&v_project_data, &m_dlg.m_main_list_ctrl);
		AfxBeginThread(WebSearchThreadProc,(LPVOID)v_search[i],THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	}
	for(unsigned int i = 0; i < v_search.size(); i++) {
		//delete v_search[i];
	}
	v_search.clear();
}