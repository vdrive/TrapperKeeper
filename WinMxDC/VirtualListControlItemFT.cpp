// VirtualListControlItemFT.cpp

#include "stdafx.h"
#include "VirtualListControlItemFT.h"

//
//
//
VirtualListControlItemFT::VirtualListControlItemFT()
{
//	memset(this,0,sizeof(VirtualListControlItemFT));

	m_mod=0;
	m_sock=0;

	m_remote_ip=0;
	m_filename.erase();

	m_start=0;
	m_end=0;
	m_total_size=0;

	m_num_sent=0;
	m_time_elapsed=0;
}

//
//
//
bool VirtualListControlItemFT::operator<(VirtualListControlItemFT &item)
{
	// Make big endian
	unsigned int ip01=(m_remote_ip>>24)&0xFF;
	unsigned int ip02=(m_remote_ip>>16)&0xFF;
	unsigned int ip03=(m_remote_ip>>8)&0xFF;
	unsigned int ip04=(m_remote_ip>>0)&0xFF;
	unsigned int ip0=(ip01<<0)+(ip02<<8)+(ip03<<16)+(ip04<<24);
	
	unsigned int ip11=(item.m_remote_ip>>24)&0xFF;
	unsigned int ip12=(item.m_remote_ip>>16)&0xFF;
	unsigned int ip13=(item.m_remote_ip>>8)&0xFF;
	unsigned int ip14=(item.m_remote_ip>>0)&0xFF;
	unsigned int ip1=(ip11<<0)+(ip12<<8)+(ip13<<16)+(ip14<<24);

	if(ip0<ip1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
