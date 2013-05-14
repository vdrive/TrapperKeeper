#pragma once

class AppID
{
public:
	AppID(void);
	~AppID(void);

	UINT m_app_id;
	UINT m_version;
	string m_app_name;
	int IsEqual(AppID& app_id);
};
