#pragma once

class SLSKController;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(SLSKController *parent);

	SLSKController *p_parent;
};