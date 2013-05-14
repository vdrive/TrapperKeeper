#pragma once

class OvernetSpooferDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(OvernetSpooferDll *parent);

	OvernetSpooferDll *p_parent;
};
