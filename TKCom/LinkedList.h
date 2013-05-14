#pragma once
#include "Object.h"

//This class is not threadsafe
#include <afxmt.h>

class LinkedList : public Object
{
private:
	class LinkedObject : public Object
	{
	public:
		Object *mp_obj;
		LinkedObject *mp_next;
		LinkedObject *mp_prev;
		LinkedObject(Object *o){
			mp_obj=o;
			mp_obj->ref();  //tell the smart pointer that something is referencing it
			mp_next=NULL;
			mp_prev=NULL;
		}

		virtual ~LinkedObject(void){
			mp_obj->deref();  //let the object delete itself when all references are gone
			if(mp_next){
				mp_next->mp_prev=mp_prev;
			}
			if(mp_prev){
				mp_prev->mp_next=mp_next;
			}
		}

		void SetPrev(LinkedObject* prev){
			mp_prev=prev;
			if(mp_prev)
				mp_prev->mp_next=this;
		}

		void SetNext(LinkedObject* next){
			mp_next=next;
			if(mp_next)
				mp_next->mp_prev=this;
		}
	};

	
	UINT m_size;
	LinkedObject *mp_current;
	LinkedObject *mp_head;
	LinkedObject *mp_tail;
public:
	LinkedList(void);
	~LinkedList(void);
	void Add(Object* object);
	UINT Size(void);
	void RemoveCurrentAndAdvance(void);
	void Clear(void);
	void StartIteration(void);
	void Advance(void);
	Object* GetCurrent(void);
	void RemoveHead(void);
	Object* GetHead(void);

	CCriticalSection m_list_lock;
};
