#pragma once
#include "..\tkcom\tkcominterface.h"
class MetaSpooferCom : public TKComInterface
{
public:
	MetaSpooferCom(void);
	~MetaSpooferCom(void);

	void DataReceived(char *source_ip, void *data, UINT data_length);
};
