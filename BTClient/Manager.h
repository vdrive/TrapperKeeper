#pragma once

class BTClientDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(BTClientDll *parent);

	BTClientDll*p_parent;
};
