// NetworkBandwidth.h

#ifndef NETWORK_BANDWIDTH_H
#define NETWORK_BANDWIDTH_H

#include "NetworkBandwidthHost.h"
#include "NetworkDlg.h"

class StatusDestDll;
class NetworkBandwidth
{
public:
	NetworkBandwidth(StatusDestDll* parent);
	~NetworkBandwidth();
	void Clear();
	void ShowNetworkGUI();

	string m_network;
	vector<NetworkBandwidthHost> v_hosts;
	CNetworkDlg m_dlg;
};

#endif // NETWORK_BANDWIDTH_H