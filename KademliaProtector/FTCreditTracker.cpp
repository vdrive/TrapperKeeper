#include "StdAfx.h"
#include "ftcredittracker.h"

FTCreditTracker::FTCreditTracker(const char* peer)
{
	m_ip=peer;
	m_credits=0;
}

FTCreditTracker::~FTCreditTracker(void)
{
}
