#pragma once
#include "..\tkcom\tkcominterface.h"

class KazaaSupplyTakerDll;

class KazaaSupplySupernodeCom :
	public TKComInterface
{
public:
	KazaaSupplySupernodeCom(void);
	~KazaaSupplySupernodeCom(void);

	void InitParent(KazaaSupplyTakerDll *dll);

	KazaaSupplyTakerDll *p_dll;

protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
