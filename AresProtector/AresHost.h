#pragma once
#include "..\tkcom\object.h"

class AresHost : public Object
{
	unsigned int m_ip;
	string m_sip;
	unsigned short m_port;

	
public:
	AresHost(unsigned int ip,unsigned short port);
	AresHost(const char* ip,unsigned short port);
	~AresHost(void);

	const char* GetIP(){return m_sip.c_str();}
	unsigned int GetIntIP(){return m_ip;}
	unsigned short GetPort(){return m_port;}

	int m_fail_count;  //a counter to keep track of how many times we have failed our connection attempt to this host

	// Compares this object to another.  Classes should override this function if they want sorting capability etc.
	inline int CompareTo(Object* object)
	{
		AresHost *ah=(AresHost*)object;
		if(m_ip>ah->m_ip)
			return 1;
		else if(m_ip<ah->m_ip)
			return -1;
		else return 0;
	}
	void WriteToFile(HANDLE file);
};
