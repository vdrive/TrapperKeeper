#pragma once
#include "ByteBuffer.h"
#include "Hash20Byte.h"
#include "Peer.h"

// Predefined HTTP server messages
#define HTTP_200							"HTTP/1.0 200 OK\r\n\r\n"
#define HTTP_404							"HTTP/1.0 404 Not Found\r\n\r\n"


// Predefined tracker warning/error messages
#define FAILURE_UNRECOGNIZED_HOST_START		"Unrecognized host ("
#define FAILURE_UNRECOGNIZED_HOST_MIDDLE	"). Please go to "
#define FAILURE_UNRECOGNIZED_HOST_END		" to sign-up or login."
#define FAILURE_UNAUTHORIZED_TORRENT		"Torrent is not authorized for use on this tracker."
#define FAILURE_MISSING_INFOHASH			"Info hash missing, please try again."
#define FAILURE_UNSUPPORTED_PROTOCOL		"Unsupported tracker protocol, please upgrade your client."
#define FAILURE_INVALID_PACKET				"You sent me garbage, please try again."

//
//
//
class TorrentIndexResult;
class BTPacketGenerator
{
public:
	BTPacketGenerator(void);
	BTPacketGenerator(TorrentIndexResult *pTorrentIndexResult);
	~BTPacketGenerator(void);

	void SetFailureReason(const char *pStr);
	void SetWarningMessage(const char *pStr);

private:
	inline void SetInterval(int nInterval) { m_nInterval = nInterval; }
	inline void SetMinInterval(int nMinInterval) { m_nMinInterval = nMinInterval; }
	inline void SetTrackerID(const Hash20Byte &rTrackerID) { m_trackerID = rTrackerID; }
	inline void SetComplete(int nComplete) { m_nComplete = nComplete; }
	inline void SetIncomplete(int nIncomplete) { m_nIncomplete = nIncomplete; }
	inline void SetHaveCompleted(int nHaveCompleted) { m_nHaveCompleted = nHaveCompleted; }
	inline void SetScrapeHash(const Hash20Byte &rScrapeHash) { m_scrapeHash = rScrapeHash; }
	inline void SetPeers(const set<Peer> &rsPeers) { m_sPeers = rsPeers; }

public:
	void GetBencodedPacket(ByteBuffer &rBuf, bool bSupportsCompact);

private:
	char *			m_pFailureReason;
	char *			m_pWarningMessage;
	int				m_nInterval;
	int				m_nMinInterval;
	Hash20Byte		m_trackerID;
	int				m_nComplete;
	int				m_nIncomplete;
	int				m_nHaveCompleted;
	set<Peer>		m_sPeers;
	Hash20Byte		m_scrapeHash;

	bool			m_bAnnounce;
};
