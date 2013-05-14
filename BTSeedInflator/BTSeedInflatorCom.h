#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class BTSeedInflatorDll;

class BTSeedInflatorCom :
	public TKComInterface
{
public:
	BTSeedInflatorCom(void);
	~BTSeedInflatorCom(void);

	void InitParent(BTSeedInflatorDll * parent);

	protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);

	private:

	BTSeedInflatorDll * p_parent;
};
