#pragma once

class WinMxDcDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(WinMxDcDll *parent);

	WinMxDcDll *p_parent;
};
