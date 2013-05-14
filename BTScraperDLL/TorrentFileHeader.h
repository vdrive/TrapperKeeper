#pragma once

class TorrentFileHeader
{
public:
	TorrentFileHeader(void);
	~TorrentFileHeader(void);
	int GetSize();
	int GetAnnounceLength();
	int GetNumPieces();
	void SetAnnounceLength(int len);
	void SetNumPieces(int num);

	int m_creationdate;
	int m_length;
	int m_piecelength;
	int m_numpieces;
	int info_len;
	int info_start;
	int m_size;

protected:
	int m_len; // number of announce
	int m_num; // number of pieces
};
