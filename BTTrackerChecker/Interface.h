#pragma once
#include "../DllLoader/AppID.h"

class Dll;
class Interface
{
public:
	Interface();
	virtual ~Interface(void);
	virtual bool InterfaceReceivedData(AppID from_app_id, void* input_data, void* output_data) = 0;
protected:
	Dll* p_dll;
public:
	virtual void Register(Dll* pDll);
};
