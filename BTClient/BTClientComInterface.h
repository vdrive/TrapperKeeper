#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class BTClientDll;

class BTClientComInterface :
	public TKComInterface
{
public:
	BTClientComInterface(void);
	~BTClientComInterface(void);

	void InitParent(BTClientDll * parent);

	protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);

	private:

	BTClientDll * p_parent;
};
