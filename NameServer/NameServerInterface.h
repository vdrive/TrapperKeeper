#pragma once
#include "..\sampleplugin\interface.h"
#include "RackInfo.h"

//interface class used to get ip addresses that correspond to a tag and get all the unique tag names
//just create a NameServerInterface in your own class and call the Request functions

class NameServerInterface :
	public Interface
{
public:
	NameServerInterface(void);
	~NameServerInterface(void);
	//looks up tag and returns a vector of ips
	bool RequestIP(CString TAG, vector<string> &v_ips);	//function to get all the ip addresses that are associated with the tag
	bool RequestAllNames(vector<string> &v_names);	//function to get all the names of the tags
	bool RequestIPeX(vector<RackInfo> &v_ipinfo);	//function to get all info each ip address in database

private:
	bool SendData(AppID to_appid,const char* tag, vector<string>& copy);				//sends to name server app
	bool InterfaceReceivedData(AppID from_app_id, void* input_data,void* output_data); //need to override
};
