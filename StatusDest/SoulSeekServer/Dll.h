#pragma once
#include "Interface.h"
#include "../DllLoader/AppID.h"
#include <vector>
using namespace std;

class Dll
{
public:
	Dll(void);
	virtual ~Dll(void);

	void AddInterface(Interface* pi);
	void RemoveInterface(Interface* pi);

	//DllInterface calls this function when it received data from other dlls
	bool DllReceivedData(AppID from_app_id, void* input_data, void* output_data);

	//override this function if you are expecting data from other dlls locally
	virtual bool ReceivedDllData(AppID from_app_id, void* input_data, void* output_data);

	//First function Trapper Keeper calls when loading this dll
	virtual void DllInitialize() = 0;

	//When Trapper Keeper is exiting, it calls this function
	virtual void DllUnInitialize() = 0;
	
	//After checking the validity of this dll, Trapper Keeper calls this function
	virtual void DllStart() = 0;

	//Show me you GUI
	virtual void DllShowGUI() = 0;
	
	//override this in order to receive data from the COM service.
	virtual void DataReceived(char *source_name, void *data, int data_length){}

private:
	vector<Interface*> v_pInterfaces;
};
