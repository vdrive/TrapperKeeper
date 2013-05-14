#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class BTInflatorDll;

class BTInflatorCom :
	public TKComInterface
{
public:
	BTInflatorCom(void);
	~BTInflatorCom(void);

	void InitParent(BTInflatorDll * parent);

	protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);

	private:

	BTInflatorDll * p_parent;
};
