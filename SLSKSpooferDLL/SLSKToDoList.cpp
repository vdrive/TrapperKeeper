
#include "stdafx.h"
#include "SLSKToDoList.h"

SLSKToDoList::SLSKToDoList()
{
	ctp = false;
	ctm = false;
	connecttohosts = false;
    tasksforserver = false;
}

SLSKToDoList::~SLSKToDoList()
{
}

SLSKToDoList* SLSKToDoList::sm_instance=NULL;