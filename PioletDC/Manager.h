#pragma once

class PioletDCDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(PioletDCDll *parent);

	PioletDCDll *p_parent;
};
