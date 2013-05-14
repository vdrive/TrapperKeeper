#pragma once

class OvernetProtectorDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(OvernetProtectorDll *parent);

	OvernetProtectorDll *p_parent;
};
