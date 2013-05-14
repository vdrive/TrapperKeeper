#pragma once
#include "AppID.h"

class DllInfo
{
public:
	DllInfo(void);
	~DllInfo(void);

	HINSTANCE m_hinstance;
	AppID m_appID;
	string m_module_path;
};
