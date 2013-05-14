#include "StdAfx.h"
#include "nameserverinterface.h"
#include "DllInterface.h"

NameServerInterface::NameServerInterface(void)
{
}

NameServerInterface::~NameServerInterface(void)
{
}
//will fill vector will all the tag names that are in use
bool NameServerInterface::RequestAllNames(vector<string> &v_names)
{
	AppID m_app_id;
	m_app_id.m_app_id=76;
	m_app_id.m_version = 0x00000001;
	m_app_id.m_app_name = "Name Server";
	return DllInterface::SendData(m_app_id,(void*)"GETALLNAMES",&v_names);

}
//When called, will fill up the vector will all the corresponding ip addresses that match the tag
bool NameServerInterface::RequestIP(CString TAG, vector<string> &v_ips)
{
	//Send to NameServer Application

	AppID m_app_id;
	m_app_id.m_app_id=76;
	m_app_id.m_version = 0x00000001;
	m_app_id.m_app_name = "Name Server";
	return this->SendData(m_app_id,TAG,v_ips);
}

bool NameServerInterface::RequestIPeX(vector<RackInfo> &v_ipinfo)
{
	AppID m_app_id;
	m_app_id.m_app_id=76;
	m_app_id.m_version = 0x00000001;
	m_app_id.m_app_name = "Name Server";
	return DllInterface::SendData(m_app_id,(void*)"GETIPINFO",&v_ipinfo);	
}
bool NameServerInterface::SendData(AppID to_appid,const char* tag, vector<string>& copy)
{
	if(tag==NULL)	//make sure tag is not null
		return false;

	return DllInterface::SendData(to_appid,(void*)tag,&copy);	//get the ip addresses
}
//overriden function
bool NameServerInterface::InterfaceReceivedData(AppID from_app_id,void* input_data, void* output_data)
{			
	return false;
}