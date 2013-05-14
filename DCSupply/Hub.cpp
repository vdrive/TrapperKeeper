#include "StdAfx.h"
#include "hub.h"

Hub::Hub(void)
{
	this->m_connected=false;
	this->m_users=0;
	this->m_name="";
	this->m_description="";
	this->m_name="";
	this->m_address="";
	this->m_port=0;
	
}

Hub::~Hub(void)
{
}
