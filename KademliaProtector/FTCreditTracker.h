#pragma once
#include "..\tkcom\object.h"

class FTCreditTracker : public Object
{
public:
	string m_ip;
	int m_credits;

	FTCreditTracker(const char* peer);
	~FTCreditTracker(void);

	void UseCredit(void)
	{
		m_credits++;
	}


	void GainCredit(void)
	{
		if(m_credits>0)
			m_credits--;
	}
};
