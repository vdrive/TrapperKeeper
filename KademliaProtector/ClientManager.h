#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include "KadFTConnection.h"

class ClientManager : public ThreadedObject
{
	Vector mv_clients;
	Vector mv_terminal_clients;
	Vector mv_client_credits[NUMCONBINS];
	

protected:
	inline int GetBinFromPeer(const char* peer)
	{
		UINT ip=inet_addr(peer);
		return (int)(ip%NUMCONBINS);
	}

public:
	ClientManager(void);
	~ClientManager(void);
	void AddClient(KadFTConnection* con);
	UINT Run(void);
	void Shutdown(void);
	UINT GetClientCount(void);
	void GuiClearTerminalClients(void);
	void UseCredit(const char* peer);
	bool HasEnoughCreditToConnect(const char* peer);
};
