#include "StdAfx.h"
#include "linkedlist.h"

LinkedList::LinkedList(void)
{
	mp_head=NULL;
	mp_tail=NULL;
	m_size=0;
	mp_current=NULL;
}

LinkedList::~LinkedList(void)
{
	Clear();
}

//Appends an object to the tail of the list
void LinkedList::Add(Object* object)
{
	m_size++;
	LinkedObject *nlo=new LinkedObject(object);
	if(mp_tail==NULL){
		mp_head=nlo;
		mp_tail=nlo;
		return;
	}

	//append to the tail of the list
	mp_tail->SetNext(nlo);
	mp_tail=nlo;
}

UINT LinkedList::Size(void)
{
	return m_size;
}

//this removes an object at the current position, and advances that position.
void LinkedList::RemoveCurrentAndAdvance(void)
{
	LinkedObject* p_target=mp_current;
	if(p_target){
		m_size--;
		mp_current=p_target->mp_next;  //advance current

		if(p_target==mp_head){  //check head pointer
			mp_head=p_target->mp_next;
		}
		if(p_target==mp_tail){	//check tail pointer
			mp_tail=p_target->mp_prev;  //deleting the tail should move the tail back one
		}

		delete p_target;  //targets destructor will take care of reassigning next/prev pointers
	}
}

void LinkedList::Clear(void)
{
	mp_current=mp_head;
	while(mp_current){
		LinkedObject* next=mp_current->mp_next;
		delete mp_current;
		mp_current=next;
	}

	mp_head=NULL;
	m_size=0;
}

void LinkedList::StartIteration(void)
{
	mp_current=mp_head;
}

void LinkedList::Advance(void)
{
	if(mp_current){
		mp_current=mp_current->mp_next;
	}
}

Object* LinkedList::GetCurrent(void)
{
	if(mp_current)
		return mp_current->mp_obj;
	else
		return NULL;
}

//support for queue data structure
void LinkedList::RemoveHead(void)
{
	LinkedObject* p_target=mp_head;
	if(p_target){
		m_size--;
		if(p_target==mp_current){
			mp_current=p_target->mp_next;
		}

		mp_head=p_target->mp_next;

		if(mp_tail==p_target){
			mp_tail=NULL;
		}

		delete p_target;  //targets destructor will take care of reassigning pointers
	}	
}

//support for queue data structure
Object* LinkedList::GetHead(void)
{
	if(mp_head)
		return mp_head->mp_obj;
	else
		return NULL;
}
