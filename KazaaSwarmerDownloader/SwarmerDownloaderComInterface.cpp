#include "StdAfx.h"
#include "swarmerdownloadercominterface.h"
#include "../SamplePlugIn/Dll.h"
#include "SwarmerDownloader.h"

SwarmerDownloaderComInterface::SwarmerDownloaderComInterface(void)
{
}

SwarmerDownloaderComInterface::~SwarmerDownloaderComInterface(void)
{
}

void SwarmerDownloaderComInterface::InitParent(SwarmerDownloader * parent)
{
	p_parent = parent;
}

void SwarmerDownloaderComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	p_parent->DataReceived(source_ip,data,data_length);
}