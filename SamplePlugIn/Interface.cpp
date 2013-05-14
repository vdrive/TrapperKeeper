#include "StdAfx.h"
#include "interface.h"
#include "Dll.h"

Interface::Interface()
{
	p_dll = NULL;
}

//
//
//
Interface::~Interface(void)
{
	if(p_dll != NULL) //if register was never called
		p_dll->RemoveInterface(this);
}

//
//
//Call to register this Interface with the dll for messages.
void Interface::Register(Dll* pDll)
{
	p_dll = pDll;
	p_dll->AddInterface(this);
}

