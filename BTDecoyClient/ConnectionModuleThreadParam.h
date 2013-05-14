#pragma once

class ConnectionModuleThreadParam
{
public:

	ConnectionModuleThreadParam(HWND hwnd, const unsigned char *pData, unsigned int nDataSize,
		int nClientType)
		: m_hwnd(hwnd), m_pData(pData), m_nDataSize(nDataSize), m_nClientType(nClientType)
	{
	}

	~ConnectionModuleThreadParam(void)
	{
		m_pData = NULL;
		m_nDataSize = 0;
	}

public:
	HWND					m_hwnd;
	const unsigned char *	m_pData;
	unsigned int			m_nDataSize;
	int						m_nClientType;
};
