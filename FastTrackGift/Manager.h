#pragma once

class FastTrackGiftDll;
class Manager
{
public:
	Manager(void);
	~Manager(void);
	virtual void InitParent(FastTrackGiftDll *parent);

	FastTrackGiftDll *p_parent;
};
