#pragma once

class GnutellaSupplyDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(GnutellaSupplyDll *parent);

	GnutellaSupplyDll *p_parent;
};
