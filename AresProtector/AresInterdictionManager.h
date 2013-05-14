#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\Vector.h"
#include "interdictiontarget.h"
#include <afxmt.h>

class AresInterdictionManager : public ThreadedObject
{
private:

	CCriticalSection m_lock;
	SOCKET m_rcv_socket;
	SOCKET m_send_socket;
	int m_index;
	Vector mv_interdiction_targets;

	float m_banned_client_ratio;

public:
	AresInterdictionManager(void);
	~AresInterdictionManager(void);
	void StopSystem(void);
	void StartSystem(void);
	bool GetInterdictionTarget(Vector &v_tmp);
	UINT Run(void);
	void Update(void);
	UINT GetInterdictionTargetCount(void);
	void RequestMoreTargets(void);
	float GetBannedClientRatio(void);
	void UpdateBannedClientRatio(void);
	void CleanTargets(void);
};
