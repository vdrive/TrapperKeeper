#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class UsenetPosterDll;

class UsenetPosterCom :
	public TKComInterface
{
public:
	UsenetPosterCom(void);
	~UsenetPosterCom(void);

	void InitParent(UsenetPosterDll *dll);
	UsenetPosterDll *p_dll;

protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);
};
