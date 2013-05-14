#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class BTDataDll;

class BTCollectorComInterface :
	public TKComInterface
{
public:
	BTCollectorComInterface(void);
	~BTCollectorComInterface(void);

	void InitParent(BTDataDll * parent);

	protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);

	private:

	BTDataDll * p_parent;
};
