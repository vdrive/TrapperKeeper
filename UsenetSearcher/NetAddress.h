//NetAddress.h
#ifndef NET_ADDRESS_H
#define NET_ADDRESS_H

#include <string>
using namespace std;

class NetAddress
{
	unsigned int m_ip;
	unsigned int m_port;

public:
	NetAddress();
	~NetAddress(){};
	void SetIP(unsigned int ip);
	void SetIP(string ip);
	void SetPort(unsigned int port);
	string GetIPString();
	string GetReversedIPString();
	unsigned int GetIPInt();
	unsigned int GetPort();
	bool IsNull();
};

#endif //NET_ADDRESS_H