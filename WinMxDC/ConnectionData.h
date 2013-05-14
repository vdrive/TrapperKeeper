// ConnectionData.h

#ifndef CONNECTION_DATA_H
#define CONNECTION_DATA_H

class HttpSocket;

class ConnectionData
{
public:
	ConnectionData();
	bool operator ==(ConnectionData &data);

	unsigned int m_ip;
	string m_filename;
	HttpSocket *p_socket;
};

#endif // CONNECTION_DATA_H