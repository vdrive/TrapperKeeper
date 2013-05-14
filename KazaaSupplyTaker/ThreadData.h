// ThreadData.h

#ifndef THREAD_DATA_H
#define THREAD_DATA_H

#include "SearchResult.h"
#include <vector>
using namespace std;

class ThreadData
{
public:
	ThreadData();
	void Clear();

	int m_worker_id;
	int m_week_number;

	HWND m_hwnd;

	CString m_table_name;
	CString m_db_name;
	CString m_db_login;
	CString m_db_pass;
	CString m_db_ip;

	bool m_swarming;

	string m_project;

	vector<SearchResult> v_results;
};

#endif // THREAD_DATA_H