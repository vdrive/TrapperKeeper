// Supernode.cpp

#include "stdafx.h"
#include "Supernode.h"

//
//
//
Supernode::Supernode()
{
	Clear();
}

//
//
//
void Supernode::Clear()
{
	m_ip=0;
	m_port=0;
	m_load_percentage=0;
	m_availability=0;
	m_creation_time=0;
}