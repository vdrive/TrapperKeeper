#pragma once
#include "tkcominterface.h"

class SDComInterface : public  TKComInterface
{
public:
	SDComInterface(void);
	~SDComInterface(void);
protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
