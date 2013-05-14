#pragma once
#include "../NameServer/nameserverinterface.h"

class WatchDogControllerSourceDll;
class WatchDogNameServerInterface :public NameServerInterface
{
public:
	WatchDogNameServerInterface(void);
	~WatchDogNameServerInterface(void);
	void InitParent(WatchDogControllerSourceDll *parent);
	//virtual void ReceivedIPs(vector<string> &v_ips);
	void ReceivedIPs(vector<string> &v_ips);

private:
	WatchDogControllerSourceDll *p_parent;
};
