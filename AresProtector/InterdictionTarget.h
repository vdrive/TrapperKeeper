#pragma once
#include "..\tkcom\object.h"

class InterdictionTarget : public Object
{
public:
	public:
		unsigned int m_ip;
		unsigned int m_size;
		unsigned short m_port;
		byte m_hash[20];

		bool mb_banned;
		int m_fail_count;
		CTime m_create_time;

		string GetStrIP(){
			string str;
			char buf[64];
			sprintf( buf , "%u.%u.%u.%u" , (m_ip>>0)&0xff , (m_ip>>8)&0xff , (m_ip>>16) & 0xff ,(m_ip>>24) & 0xff );  //little endian
			str=buf;
			return str;
		}

		InterdictionTarget(unsigned int ip,unsigned short port, unsigned int size, const char* hash_str,byte* hash_byte);
		InterdictionTarget(const char* ip,unsigned short port, unsigned int size, const char* hash_str,byte* hash_byte);
		~InterdictionTarget();

		inline int CompareTo(Object* object)
		{
			InterdictionTarget *ah=(InterdictionTarget*)object;
			if(m_ip>ah->m_ip)
				return 1;
			else if(m_ip<ah->m_ip)
				return -1;
			else{
				if(m_port>ah->m_port)
					return 1;
				else if(m_port<ah->m_port)
					return -1;
				else  //ip and port are equal, this is indeed the same client
					return 0;
			}
		}
};
