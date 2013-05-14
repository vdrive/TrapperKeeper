#pragma once

class GnutellaProtectorDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(GnutellaProtectorDll *parent);

	GnutellaProtectorDll *p_parent;
};
