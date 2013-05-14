#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class BTIPGathererDll;

class BTIPGathererCom :
	public TKComInterface
{
public:
	BTIPGathererCom(void);
	~BTIPGathererCom(void);

	void InitParent(BTIPGathererDll * parent);

	protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);

	private:

	BTIPGathererDll * p_parent;
};
