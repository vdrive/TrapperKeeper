#pragma once
#include "c:\cvs\mdproj\trapperkeeper\tkcom\tkcominterface.h"

class UsenetSearcherDll;

class UsenetSearcherCom :
	public TKComInterface
{
public:
	UsenetSearcherCom(void);
	~UsenetSearcherCom(void);

	void InitParent(UsenetSearcherDll *dll);
	UsenetSearcherDll *p_dll;
};
