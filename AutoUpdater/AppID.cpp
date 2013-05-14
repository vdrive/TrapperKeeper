#include "StdAfx.h"
#include "appid.h"

AppID::AppID(void)
{
	m_app_id = 0;
	m_version = 0;
	m_app_name = "";
}

//
//
//
AppID::~AppID(void)
{
}

//
//
//
//returns 1 if same app id and same version
//returns 0 if different app id
//returns -1 if same app id but different version
int AppID::IsEqual(AppID& app_id)
{
	if(m_app_id == app_id.m_app_id && m_version == app_id.m_version)
		return 1;
	if(m_app_id == app_id.m_app_id)
		return -1;
	return 0;
}
