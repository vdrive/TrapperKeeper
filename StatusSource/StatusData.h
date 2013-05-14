// StatusData.h

#ifndef STATUS_DATA_H
#define STATUS_DATA_H

class StatusData
{
public:
	// Public Member Functions
	StatusData();
	StatusData(StatusData* status_data);
	StatusData& operator=(const StatusData* status_data);	// assignment operator

	int ReadFromBuffer(char* buf);
	int WriteToBuffer(char* buf);
	int GetBufferLength();

	void Clear();

	// Public Data Members
	unsigned int m_version;						// version of the data structure
	unsigned int m_percent_processor_usage;		// 0-100%
//	unsigned int m_total_available_bandwidth;	// Usually 100 Mb/s
	unsigned int m_total_used_bandwidth;		// 0-100 Mb/s
	unsigned int m_kazaa_count;
	unsigned int m_shared_file_count;
	unsigned int m_max_kazaa;
	float m_free_bytes;	//C: free space in GB

	// list of kazaa memories
	vector<UINT> v_kazaa_mems;

	//new version
	UINT m_movies_shared;
	UINT m_music_shared;
	UINT m_swarmer_shared;

	//new new version
	UINT m_received_per_sec;
	UINT m_sent_per_sec;
	UINT m_trapper_keeper_mem_usage;
	UINT m_committed_memory;
};

#endif // STATUS_DATA_H