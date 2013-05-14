#pragma once
#include "..\tkcom\vector.h"
#include "..\tkcom\threadedobject.h"

class SupernodeServerSystemInterface : public ThreadedObject
{
private:
	CAsyncSocket m_rcv_socket;
	CAsyncSocket m_send_socket;
public:
	SupernodeServerSystemInterface(void);
	~SupernodeServerSystemInterface(void);
	UINT Run(void);
	void StartSystem(void);
	void StopSystem(void);
	virtual void ReceivedSupernodes(vector<string> &v_ips, vector<unsigned short> &v_ports);
	void RequestSupernodes(void);
	void ReportNewSupernodesToServer(vector<string> &v_ips,vector<unsigned short> &v_ports);
};
