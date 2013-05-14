#pragma once

class AcceptedSocket
{
public:

	AcceptedSocket(SOCKET hSocket, unsigned int addr)
		: m_hSocket(hSocket), m_addr(addr)
	{
	}

	~AcceptedSocket(void)
	{
	}

public:
	SOCKET			m_hSocket;
	unsigned int	m_addr;
};
