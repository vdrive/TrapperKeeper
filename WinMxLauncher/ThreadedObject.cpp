#include "StdAfx.h"
#include "threadedobject.h"

ThreadedObject::ThreadedObject(void)
{
	b_threadRunning=0;
	b_killThread=0;
}

ThreadedObject::~ThreadedObject(void)
{
	StopThread();
}

// Override this method to do threaded work
UINT ThreadedObject::Run()
{
	return 0;
}

UINT ServerProc( LPVOID pParam )
{
	ThreadedObject *threadObj=(ThreadedObject*)pParam;
	threadObj->b_threadRunning=1;
	UINT val=threadObj->Run();
	threadObj->b_threadRunning=0;
	return val;
}

// Call this function to start the thread
void ThreadedObject::StartThread()
{
	AfxBeginThread(ServerProc,this);
}

// Call this method to stop the thread
void ThreadedObject::StopThread(void)
{
	b_killThread=1;
	while(b_threadRunning){
		Sleep(5);
	}
	b_killThread=0;
}
