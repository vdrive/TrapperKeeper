#pragma once

#include <string>
using namespace std;

class AppID
{
public:
	AppID(void);
	~AppID(void);
	AppID(unsigned int id,unsigned int version=1,const char* name=NULL);

	unsigned int m_app_id;
	unsigned int m_version;
	string m_app_name;
	int IsEqual(AppID& app_id);
};
