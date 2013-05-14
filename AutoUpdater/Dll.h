#pragma once
#include "Interface.h"
#include "../DllLoader/AppID.h"

class Dll
{
public:
	Dll(void);
	virtual ~Dll(void);

	void AddInterface(Interface* pi);
	void RemoveInterface(Interface* pi);
	bool DllReceivedData(AppID& from, void* data, UINT& datasize);
	virtual bool ReceivedDllData(AppID& from, void* data, UINT& datasize);
	virtual void DllInitialize() = 0;
	virtual void DllUnInitialize() = 0;
	virtual void DllStart() = 0;
	virtual void DllShowGUI() = 0;

private:
	vector<Interface*> v_pInterfaces;
};
