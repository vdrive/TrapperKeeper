#pragma once

class GnutellaDecoyerDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(GnutellaDecoyerDll *parent);

	GnutellaDecoyerDll *p_parent;
};
