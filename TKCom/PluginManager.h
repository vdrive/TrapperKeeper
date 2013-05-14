#pragma once
#include "object.h"
#include "DataBuffer.h"
#include <mmsystem.h>
#include "Vector.h"
#include "ComTCP.h"
#include "LinkedList.h"

class PluginManager :
	public Object
{
private:
	class Destination : public Object {
	public:
		string m_dest;
		int m_attempts;
		UINT m_last_attempt;
		Destination(const char* dest){
			m_dest=dest;
			m_last_attempt=GetTickCount();
			if(m_last_attempt<2000)
				m_last_attempt=2000;
			m_last_attempt-=2000;
			m_attempts=0;
		}
	};

	//this is a private class used by the plugin manager for data storage.
	class SendJob : public Object{
	public:
		DataBuffer *data;
		LinkedList ml_destinations;
		UINT from_app;
	public:
		~SendJob();

		SendJob( DataBuffer *_data,UINT _from_app);

		inline void AddDest(const char* dest);

		inline bool IsFromApp(UINT app);

		inline UINT GetDataSize();

		inline DataBuffer* GetData();

		inline bool IsDone();
	};
private:
	UINT m_queued_data;
	CCriticalSection m_job_lock;
	LinkedList ml_jobs;
	//int GetHighestPriorityJob(void);

public:
	PluginManager(void);
	~PluginManager(void);
	bool QueueSend(DataBuffer* buffer, vector <string> &v_dests,UINT from_app);

	
	void Process(ComTCP* tcp);
};
