#pragma once
#include "..\tkcom\tksocketsystem.h"

class AresSNTCP : public TKSocketSystem
{
public:
	AresSNTCP(void);
	~AresSNTCP(void);

	//override this to be notified when data has come in for one of your connections
	void OnReceive(UINT con_id , byte* data , UINT length,const char* peer);

	//override this to be notified when data has been successfully sent to the destination
	//if a connection is functioning properly, then you will get 1 OnSend for each send request you made.
	void OnSend(UINT con_id,const char* peer){}

	void OnServerError(UINT server_id,UINT error){}

	//Override this to be notified when a particular connection has been closed for any number of reasons.  This will not be called if you explicitly called CloseConnection().
	void OnClose(UINT con_id,UINT reason, const char* peer);

	//override this to keep new connections from being automatically closed.
	//handle is how you reference your new connection.
	void OnAcceptedConnection(UINT server_handle,UINT con_id,const char* peer,unsigned short port);

	//notification that a OpenConnection call has completed.
	void OnConnect(UINT con_id, const char* peer);
};
