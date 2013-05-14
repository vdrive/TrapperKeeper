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
bool VendorCount::operator<(const VendorCount &vc)const
{
	if(m_count<vc.m_count)
	{
		return true;
	}
	else
	{
		return false;
	}
}

