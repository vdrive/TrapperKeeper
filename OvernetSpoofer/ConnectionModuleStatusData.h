// ConnectionModuleStatusData.h
#pragma once

class ConnectionModuleStatusData
{
public:
	// Public Member Functions
	ConnectionModuleStatusData();
	~ConnectionModuleStatusData();
	void Clear();
	UINT m_num_peers;
	UINT m_num_supplies;
	UINT m_num_projects;
	UINT m_hash_search_index;
	UINT m_publicize_ack_received;
	UINT m_userhash_to_publicize;
	UINT m_queries_received;
	UINT m_publish_received;
	UINT m_spoofs_sent;
};