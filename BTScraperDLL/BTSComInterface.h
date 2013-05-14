#pragma once
#include "..\tkcom\tkcominterface.h"

class BTSComInterface :	public TKComInterface
{
public:
	BTSComInterface(void);
	~BTSComInterface(void);
protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
