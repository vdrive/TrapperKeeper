#pragma once

class AppID
{
public:
	AppID(void);
	~AppID(void);
	AppID(UINT id, UINT version=1, const char* name=NULL);

	UINT m_app_id;
	UINT m_version;
	string m_app_name;
	int IsEqual(AppID& app_id);
};
