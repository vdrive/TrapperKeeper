#pragma once

#include "ProjectKeywords.h"
#include "Header.h"

class UsenetSearcherDll;

class ThreadData
{
public:
	ThreadData(void);
	~ThreadData(void);

	vector<ProjectKeywords> v_projects;
	vector<Header> v_headers;

	HWND m_dlg_hwnd;
	UsenetSearcherDll *p_dll;
};
