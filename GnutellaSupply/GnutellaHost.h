#pragma once

class GnutellaHost
{
public:
	GnutellaHost(void);
	~GnutellaHost(void);
	void Clear();

	bool operator ==(GnutellaHost &host);

	unsigned int IP();
	string Host();
	unsigned int Port();

	void Host(char *host);
	void Port(unsigned int port);

	void IP(unsigned int ip);
	
private:
	string m_host;
	unsigned int m_ip;
	unsigned int m_port;
};
