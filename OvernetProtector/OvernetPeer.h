#pragma once

class OvernetPeer
{
	friend class OvernetPeerHash;
public:
	OvernetPeer(void);
	~OvernetPeer(void);

	UINT m_ip;
	short m_port;
	unsigned char m_id[16];
	unsigned int m_checksum;
	/*
	bool m_connected;
	bool m_publicized_to;
	bool m_publicized_from;
	CTime m_last_publish_time;
	*/

	bool operator < (const OvernetPeer&  other)const;
	bool operator ==(OvernetPeer &other);
	void Clear();
	void SetID(byte* id, UINT ip, short port);
	bool IsHashCloseEnough(const byte hash);


private:
	void SetHashChecksum();
};

class OvernetPeerHash : public hash_compare <OvernetPeer>
{
public:
	using hash_compare<OvernetPeer>::operator();
	size_t operator()(const OvernetPeer& key)const
	{
		return key.m_checksum;
	}
};