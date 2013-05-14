// VendorCount.cpp

#include "stdafx.h"
#include "VendorCount.h"

//
//
//
VendorCount::VendorCount()
{
	m_vendor.erase();
	m_count=0;
}

//
//
//
VendorCount::~VendorCount()
{
	m_vendor.erase();
}

//
//
//
//reverse sorting, bigger count at first
bool VendorCount::operator<(const VendorCount &vc)const
{
	if(m_count<vc.m_count)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//
//
//
bool VendorCount::operator==(const VendorCount &vc)const
{
	if(strcmp(m_vendor.c_str(), vc.m_vendor.c_str())==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}