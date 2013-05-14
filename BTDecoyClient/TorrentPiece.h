#pragma once
class TorrentPiece
{
public:

	TorrentPiece(unsigned int nPiece, unsigned int nTimeInterval)
		: m_nPiece(nPiece), m_nTimeInterval(nTimeInterval)
	{
	}

	~TorrentPiece(void)
	{
	}

	friend inline bool operator<(const TorrentPiece &rLeft, const TorrentPiece &rRight) { return rLeft.m_nPiece < rRight.m_nPiece; }
	friend inline bool operator>(const TorrentPiece &rLeft, const TorrentPiece &rRight) { return rLeft.m_nPiece > rRight.m_nPiece; }

public:
	unsigned int	m_nPiece;
	unsigned int	m_nTimeInterval;
};
