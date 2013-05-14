#pragma once
#include "tkcominterface.h"
#include "SyncherService.h"

class SyncherService;
class SyncherComLink :
	public TKComInterface
{
public:
	SyncherComLink(void);
	~SyncherComLink(void);
	void DataReceived(char *source_ip, void *data, UINT data_length);
	SyncherService *p_service;
	void SetService(SyncherService* service);
};
