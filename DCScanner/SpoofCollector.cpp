#include "StdAfx.h"
#include "spoofcollector.h"

SpoofCollector::SpoofCollector(void)
{
	//initialization
	this->m_spoof_count = 0;
	this->m_decoy_count = 0;
	this->m_requests_count = 0;
}

SpoofCollector::~SpoofCollector(void)
{
}

//add spoof count to m_spoof_count
void SpoofCollector::AddSpoof(int add_spoof)
{
	this->m_spoof_count += add_spoof;
}

//add decoy count to m_decoy_count
void SpoofCollector::AddDecoy(int add_decoy)
{
	this->m_decoy_count += add_decoy;
}

//add request to m_request
void SpoofCollector::AddRequest(int add_request)
{
	this->m_requests_count += add_request;
}