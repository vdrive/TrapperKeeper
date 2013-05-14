#pragma once
#include "Hash20Byte.h"

//typedef struct 

class BTPacketParser
{
public:
	enum BTPacketType
	{
		TYPE_UNINIT,				// uninitialized
		TYPE_GET_ANNOUNCE,			// announce
		TYPE_GET_SCRAPE,			// scrape
		TYPE_INVALID_URL,			// url does not end with "/announce" or "/scrape"
		TYPE_MISSING_INFOHASH,		// did not include infohash
		TYPE_NON_COMPACT,			// client does not support compact format
		TYPE_PARSE_ERROR			// generic parse error
	};
	enum TorrentEvents { STARTED, COMPLETED, STOPPED, EMPTY };

public:
	BTPacketParser(const char *pData, const size_t nSize);
	~BTPacketParser(void);

	inline BTPacketType GetPacketType(void) { return m_ePacketType; }

	inline Hash20Byte GetInfoHash(void) { return m_infoHash; }
	inline Hash20Byte GetPeerID(void) { return m_peerID; }

	inline DWORD GetIP(void) { return (DWORD)m_nIP; }
	inline USHORT GetPort(void) { return (USHORT)m_nPort; }
	inline int GetUploaded(void) { return m_nUploaded; }
	inline int GetDownloaded(void) { return m_nDownloaded; }
	inline int GetLeft(void) { return m_nLeft; }

	inline TorrentEvents GetEvent(void) { return m_eEvent; }

	//	inline bool IsComplete(void) { return (m_nLeft == 0); }

	inline bool IsCompact(void) { return m_bCompact; }

private:
	BTPacketType	m_ePacketType;

	Hash20Byte		m_infoHash;
	Hash20Byte		m_peerID;

	int				m_nIP;
	int				m_nPort;
	int				m_nUploaded;
	int				m_nDownloaded;
	int				m_nLeft;

	TorrentEvents	m_eEvent;

	bool			m_bNoPeerID;
	int				m_nNumWant;

	char			m_aKey[6];

	Hash20Byte		m_trackerID;

	bool			m_bCompact;
};
