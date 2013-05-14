
#pragma once
#include "SLSKtask.h"
#include "SupernodeHost.h"


class SLSKToDoList
{
public:
	SLSKToDoList(void);
	~SLSKToDoList(void);
	static SLSKToDoList* sm_instance;
	static SLSKToDoList* GetInstance()
	{
		return sm_instance;
	}

	vector<CString> connections;
	int numofconnections;
	vector<buffer*> stasks;
	vector<CString> peerInfo;
	vector<SLSKtask*> connectingtome;
	vector<SLSKtask*> connectingtopeer;
	vector<SupernodeHost> hosts;
    bool connecttohosts;
	bool tasksforserver;
	bool ctm;
	bool ctp;


};
