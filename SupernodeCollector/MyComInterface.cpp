#include "StdAfx.h"
#include "mycominterface.h"
#include "../SamplePlugIn/Dll.h"

MyComInterface::MyComInterface(void)
{
}

//
//
//
MyComInterface::~MyComInterface(void)
{
}

//
//
//
void MyComInterface::DataReceived(char *source_ip, void *data, UINT data_length)
{
	//received remote comm data
	p_dll->DataReceived(source_ip, data, data_length);
}