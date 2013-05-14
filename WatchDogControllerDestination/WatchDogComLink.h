#pragma once
#include "tkcominterface.h"

class WatchDogComLink : public TKComInterface
{
public:
	WatchDogComLink(void);
	~WatchDogComLink(void);

	virtual void DataReceived(char *source_ip, void *data, UINT data_length);

};
