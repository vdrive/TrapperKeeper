#include "StdAfx.h"
#include ".\BTTorrentSourceCom.h"
#include "bttorrentsourcedll.h"

BTTorrentSourceCom::BTTorrentSourceCom(void)
{
}

BTTorrentSourceCom::~BTTorrentSourceCom(void)
{
}

void BTTorrentSourceCom::InitParent(BTTorrentSourceDll * parent)
{
	p_parent = parent;
}

void BTTorrentSourceCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	p_parent->DataReceived(source_ip,data,data_length);
}
