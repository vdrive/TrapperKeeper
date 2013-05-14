#pragma once
#include "object.h"

class ThreadedObject :
	public Object
{
public:
	volatile int b_threadRunning;  //though these are public, don't fuck with them
	volatile int b_killThread;
public:
	ThreadedObject(void);
	virtual ~ThreadedObject(void);
	// Override this method to do threaded work
	virtual UINT Run();
	// Call this function to start the thread
	void StartThread();
	// Call this method to stop the thread
	void StopThread(void);
};

