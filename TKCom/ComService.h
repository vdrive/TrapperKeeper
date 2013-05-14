//AUTHOR:  Ty Heath
//LAST MODIFIED:  3.31.2003

//Name Server functionality added on 3.18.2003.

//Current Functionality
//1.  Allows X amount of data to be queued from each plugin at any given time.
//2.  Limits the number of parallel connections between this source and any given destinations to X connections.
//3.  It compresses data.  If the compressed data is larger than the original data, it just uses the original data, and flags this with a byte in the header.
//4.  After compression, it encrypts data via a randomly generated key stored in the header.  Everyone can see the key, but you have to know how to use it to decrypt the data.

#pragma once
#include "object.h"
//#include "TKCoreSocket.h"
#include "threadedobject.h"
#include "MessageDispatcher.h"
#include "..\NameServer\NameServerInterface.h"
//#include "SocketPool.h"
#include "PluginManager.h"
#include "ComTCP.h"


//This class provides a central control for all elements of the Com Service.
class ComService :	protected ThreadedObject
{
private:
	MessageDispatcher m_message_dispatcher; //responsible for sending data received messages to the various plugins that are using the com service
	NameServerInterface p_name_server;  //the interface to the name server
//	SocketPool m_socket_pool;  //manages the collection of all the sockets.
	PluginManager m_plugin_manager; //manages queuing data from various plugins.
	ComTCP m_tcp;
public:
	//Vector mv_connections;

	ComService(void);
	~ComService(void);
	virtual UINT Run();
	bool ReceivedAppData(UINT from , byte* data );
	void StopComService(void);
	void StartComService(void);
};
