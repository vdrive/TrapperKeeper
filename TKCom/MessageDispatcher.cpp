#include "StdAfx.h"
#include "messagedispatcher.h"
#include "ComSubscriber.h"
#include "AppID.h"
#include "DllInterface.h"

MessageDispatcher::MessageDispatcher(void)
{
}

MessageDispatcher::~MessageDispatcher(void)
{
}

void MessageDispatcher::DispatchMessage(DataBuffer* db)
{
	mv_messages.Add(db);
}

void MessageDispatcher::Process(void)
{
	while(mv_messages.Size()>0){  //dispatch all messages
		DataBuffer *msg=(DataBuffer*)mv_messages.Get(0);

		byte *data=msg->GetBuffer();

		for(UINT i=0;i<mv_subscribers.Size();i++){
			ComSubscriber *ss=(ComSubscriber*)mv_subscribers.Get(i);
			if(ss->m_op_code==msg->GetOpCode()){ //if a subscriber has an op_code matching this message, then lets route it to it.
				AppID tid;
				tid.m_app_id=ss->m_from_app;
				DllInterface::SendData(tid,data);
			}
		}

		mv_messages.Remove(0);  //we will remove this since we have dispatched it to anyone that registered for its type.  If anything else is referencing this memory, it won't be deleted until the last reference is gone.
	}
}
