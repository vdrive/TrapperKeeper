#include "StdAfx.h"
#include "dll.h"

Dll::Dll(void)
{
}

//
//
//
Dll::~Dll(void)
{
}

//
//
//
void Dll::AddInterface(Interface* pi)
{
	v_pInterfaces.push_back(pi);
}

//
//
//
void Dll::RemoveInterface(Interface* pi)
{
	vector<Interface*>::iterator iter = v_pInterfaces.begin();
	while(iter!=v_pInterfaces.end())
	{
		if(*(iter) == pi)
		{
			v_pInterfaces.erase(iter);
			break;
		}
		iter++;
	}
}

//
//
//
bool Dll::DllReceivedData(AppID& from, void* data, UINT& datasize)
{
	for(int i=0; i < (int)v_pInterfaces.size();i++)
	{
		if(v_pInterfaces[i]->InterfaceReceivedData(from,data,datasize))
			return true;
	}
	if(ReceivedDllData(from, data, datasize))
		return true;
	return false;
}

//
//
//
bool Dll::ReceivedDllData(AppID& from, void* data, UINT& datasize)
{
	return false;
}
