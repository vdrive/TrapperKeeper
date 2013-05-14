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
	if(b_threadRunning)
		return;
	b_killThread=0;
	m_thread=AfxBeginThread(ServerProc,this);
}

// Call this function to start the thread
void ThreadedObject::StartThreadLowPriority()
{

	if(b_threadRunning)
		return;
	b_killThread=0;
	m_thread=AfxBeginThread(ServerProc,this,THREAD_PRIORITY_BELOW_NORMAL);
}


// Call this method to stop the thread
void ThreadedObject::StopThread()
{
	int count=0;
	if(b_threadRunning){
		b_killThread=1;

		//while(b_threadRunning){
		//	Sleep(5);
		//}
		if(WaitForSingleObject(m_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0){
			//ASSERT(0);
		}
		Sleep(1);
	}
	b_killThread=0;	
}
