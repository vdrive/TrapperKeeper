#pragma once
#include "tasyncsocket.h"

class SoulSeekServerDll;

class Listener :	public TAsyncSocket
{
public:
	Listener(void);
	~Listener(void);

	void InitParent(SoulSeekServerDll *parent);

	void OnAccept(int error_code);

private:
	SoulSeekServerDll *p_parent;
};
