#pragma once
#include "stdAfx.h"
#include "../TKCom/TKComInterface.h"

class SoulSeekClientDll;

class ParentComInterface : public TKComInterface
{
public:
	ParentComInterface(void);
	~ParentComInterface(void);
	void InitParent(SoulSeekClientDll *parent);
protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
private:
	SoulSeekClientDll *p_parent;
};
