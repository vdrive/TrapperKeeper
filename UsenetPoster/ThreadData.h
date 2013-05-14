#pragma once

#include "..\UsenetSearcher\Header.h"

class UsenetPosterDll;

class ThreadData
{
public:
	ThreadData(void);
	~ThreadData(void);

	vector<Header> v_headers;

	HWND m_dlg_hwnd;
	UsenetPosterDll *p_dll;
};
