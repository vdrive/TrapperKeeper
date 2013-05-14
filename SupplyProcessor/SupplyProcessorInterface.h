//SupplyProcessorInteface.h
#pragma once

#include "..\SamplePlugIn\Interface.h"
#include "SupplyData.h"

class SupplyProcessorInterface :
	public Interface
{
public:
	SupplyProcessorInterface();
	~SupplyProcessorInterface(void);
	bool SubmitSupplyData(SupplyData& supply);
	bool SubmitSupplyData(vector<SupplyData> * v_supply);
	bool GetSupplyData(unsigned int project_id, int track_num, const char* network, int num_want, vector<SupplyData> * data);
	bool CleanDatabase(const char * network_name, int days);

protected:
	bool SendData(const char* buf, void * result);
	bool InterfaceReceivedData(AppID to_appid,void* input_data, void* output_data);
	AppID m_app_id;
};

