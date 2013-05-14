// VendorCount.h
#pragma once

class VendorCount
{
public:
	VendorCount();
	~VendorCount();

	bool operator<(const VendorCount &vc)const;

	string m_vendor;
	unsigned int m_count;
};