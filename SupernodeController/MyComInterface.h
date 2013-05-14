#pragma once
#include "..\tkcom\tkcominterface.h"

class MyComInterface :	public  TKComInterface
{
public:
	MyComInterface(void);
	~MyComInterface(void);

protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
