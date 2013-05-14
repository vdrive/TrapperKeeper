//Author:  Ty Heath
//Last Modified:  4.8.2003

#pragma once
#include "tkcominterface.h"
#include "FileTransferService.h"
class FileTransferService;

#define TKFILENOTFOUND 5
#define TKFILEINCOMING 6


//Class that overrides the TKComInterface to use COM and get DataReceived notifications.
class ComLink :
	public TKComInterface
{
private:
	//Passes DataReceived notifications to FileTransferService
	void DataReceived(char *source_ip, void *data, UINT data_length);

	//Pointer to FileTransferService so that it can give it com messages.
	FileTransferService *p_service;

public:
	ComLink(void);
	~ComLink(void);

	//Call to set the pointer to the service when this object is first created
	void SetService(FileTransferService* service);
};
