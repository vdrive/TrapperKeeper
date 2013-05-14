#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class BTTorrentSourceDll;

class BTTorrentSourceCom :
	public TKComInterface
{
public:
	BTTorrentSourceCom(void);
	~BTTorrentSourceCom(void);

	void InitParent(BTTorrentSourceDll * parent);

	protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);

	private:

	BTTorrentSourceDll * p_parent;
};
