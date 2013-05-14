#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include <afxmt.h>
#include "AresInterdictionTarget.h"

class InterdictionTargetManager : public ThreadedObject
{
private:
	CCriticalSection m_lock;
	//vector<unsigned int> mv_ips;
	//vector<unsigned short> mv_ports;
	//vector<string> mv_hashes;
	//vector<unsigned int> mv_sizes;
	Vector mv_targets;
	int m_target_index;
public:
	InterdictionTargetManager(void);
	~InterdictionTargetManager(void);
	UINT Run(void);
	void GetSomeTargets(Vector &v_tmp,int count);
	int GetInterdictionTargetCount(void);
};
