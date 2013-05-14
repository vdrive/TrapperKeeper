#pragma once
#include "..\tkcom\tkcominterface.h"

class SLSKComInterface :	public TKComInterface
{
public:
	SLSKComInterface(void);
	~SLSKComInterface(void);
protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
