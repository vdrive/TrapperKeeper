#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class AutoUpdaterDll;

class AutoUpdaterCom :
	public TKComInterface
{
public:
	AutoUpdaterCom(void);
	~AutoUpdaterCom(void);

	void InitParent(AutoUpdaterDll *parent);

	AutoUpdaterDll *p_parent;

protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
