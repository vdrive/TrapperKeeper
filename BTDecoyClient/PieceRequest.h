#pragma once

class PieceRequest
{
public:

	PieceRequest(unsigned int nIndex, unsigned int nBegin, unsigned int nLength)
		: m_nIndex(nIndex), m_nBegin(nBegin), m_nLength(nLength), m_bChoke(false)
	{
	}

	PieceRequest(bool bChoke)
		: m_nIndex(0), m_nBegin(0), m_nLength(0), m_bChoke(true)
	{
	}

	~PieceRequest(void) {}

	inline bool operator==(const PieceRequest &rRight) const { return ( m_nIndex == rRight.m_nIndex && m_nBegin == rRight.m_nBegin ); }
	inline bool operator!=(const PieceRequest &rRight) const { return ( m_nIndex != rRight.m_nIndex || m_nBegin != rRight.m_nBegin );  }
	friend inline bool operator<(const PieceRequest &rLeft, const PieceRequest &rRight) { return ( (rLeft.m_nIndex < rRight.m_nIndex) || (rLeft.m_nIndex == rRight.m_nIndex && rLeft.m_nBegin < rRight.m_nBegin) ); }
	friend inline bool operator>(const PieceRequest &rLeft, const PieceRequest &rRight) { return ( (rLeft.m_nIndex > rRight.m_nIndex) || (rLeft.m_nIndex == rRight.m_nIndex && rLeft.m_nBegin > rRight.m_nBegin) ); }

public:
	unsigned int m_nIndex;
	unsigned int m_nBegin;
	unsigned int m_nLength;

	bool m_bChoke;
};
