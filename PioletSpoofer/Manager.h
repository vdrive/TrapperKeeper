#pragma once

class PioletSpooferDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(PioletSpooferDll *parent);

	PioletSpooferDll *p_parent;
};
