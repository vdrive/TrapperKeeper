#pragma once
#include "..\tkcom\tkcominterface.h"

class KLComInterface :	public TKComInterface
{
public:
	KLComInterface(void);
	~KLComInterface(void);
protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
