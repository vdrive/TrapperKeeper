#pragma once
#include "..\tkcom\tkcominterface.h"

class SwarmerSourceDll;
class SwarmerSourceComInterface :	public TKComInterface
{
public:
	SwarmerSourceComInterface(void);
	~SwarmerSourceComInterface(void);

	void InitParent(SwarmerSourceDll * parent);

	protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);

	private:

	SwarmerSourceDll * p_parent;
};


