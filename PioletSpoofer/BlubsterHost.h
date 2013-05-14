// BlubsterHost.h

#ifndef BLUBSTER_HOST_H
#define BLUBSTER_HOST_H

class BlubsterHost
{
public:
	BlubsterHost();
	~BlubsterHost();
	void Clear();

	bool operator<(const unsigned int& ip)const;
	bool operator<(const BlubsterHost &host)const;
	bool operator==(const BlubsterHost &host)const;

	bool SortByNumConnections(BlubsterHost &host);
	void SetVendor(const char* vendor);
	void ClearVendor();

	unsigned int m_ip;
	unsigned short int m_port;
	int m_connected;	// 0 = not connected, 1-5 = connected (1 = have not seen msg from host in a while...about to give up on it, 5 = seen msg recently...strong faith in connection)
	unsigned int m_num_connections;
	char m_vendor[32];
	bool m_connected_before;
};

#endif // BLUBSTER_HOST_H