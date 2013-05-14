#pragma once
#include "threadedobject.h"
#include "Vector.h"
#include "DataBuffer.h"

//AUTHOR:  Ty Heath
//DATE:  3/6/2003
//PURPOSE:
//This class is responsible for sending received com data to plugins.
//It is essentially a threaded dispatcher that makes it so the COM accepting/sending/receiving won't pause if a plugin takes its sweet time handling a DataReceived call.

//NOTES:
//1.	If a plugin does take time to handle messages, it can and will block other plugins from receiving data from COM during that time.  Just not the entire COM system.
//		This is a serious weakness, but I am hesitant to add a thread for each message that has to go out or even 1 thread per registered plugin.  The later solution is plausible, but will result in an extra X threads running all the time.

//TODO:
//Nothing, this class is perfect.

class MessageDispatcher :
	public Object
{
public:
	CCriticalSection m_subscriber_vector_lock;
	Vector mv_subscribers;
	Vector mv_messages;
	MessageDispatcher(void);
	~MessageDispatcher(void);
	void DispatchMessage(DataBuffer* db);
	void Process(void);
};