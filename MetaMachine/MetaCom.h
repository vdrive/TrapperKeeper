#pragma once
#include "..\tkcom\tkcominterface.h"

class MetaCom :	public  TKComInterface
{
public:
	MetaCom(void);
	~MetaCom(void);
	void DataReceived(char *source_ip, void *data, UINT data_length);

};
