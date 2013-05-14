#pragma once


struct PeerInfo
{
	string m_peer_id;
	CTime m_last_connect;
	CTime m_last_attempt;
	CTime m_last_failure;
	bool m_is_seed;
	int m_port;
	bool m_has_connected;
	string m_key;
};

class Torrent
{
public:
	Torrent(void);
	~Torrent(void);

	void InitTorrent(string name, string announce_url, string info_hash, int numseeds, int numpeers, int port);
	//void GetTorrentInfo(string * announce_url, string * info_hash);
	string CreatePeerId();
	void UpdatePeerConnect(string peer_id);
	void UpdatePeerFailure(string peer_id);

	string GetAnnounceUrl();
	string GetInfoHash();
	string GetName();
	string CreateKeyID();


	int GetNumConnections();
	void GetPeerInfo(vector<PeerInfo> * info);
	void SetAttemptTime();

	bool HasConnected();
	bool IsReady();
	void SetHasConnected();

	bool operator==(const Torrent & tor);

private:

	CTime m_last_attempt;
	string m_announce_url;
	string m_info_hash;
	string m_torrent_name;
	vector<PeerInfo> v_peer_ids;
	bool m_has_connected;
};
