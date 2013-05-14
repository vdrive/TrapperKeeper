// ProjectKeywords.h

#ifndef PROJECT_KEYWORDS_H
#define PROJECT_KEYWORDS_H

#include "ProjectDataStructure.h"
#include "QueryKeywords.h"
#include "SupplyKeywords.h"
#include "QRPKeywords.h"
#include "Strings.h"
#include "NetAddress.h"
#include "HashCount.h"

class ProjectKeywords : public ProjectDataStructure
{
public:
	// Public Member Functions
	ProjectKeywords();
	~ProjectKeywords();
	bool operator < (const ProjectKeywords&  other)const;

	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

//	unsigned int CalculateChecksum();

	// Public Data Members
	string m_project_name;

	bool m_bearshare_disting_enabled;
	bool m_poisoning_enabled;
	bool m_uber_disting_enabled;
	bool m_guid_spoofing_enabled;
	bool m_auto_multiplier_enabled;
	bool m_project_active;

	unsigned int m_query_multiplier;

	unsigned int m_interval;

	string m_owner;
	Strings m_viewers;
	
	QueryKeywords m_query_keywords;

	SupplyKeywords m_supply_keywords;

	QRPKeywords m_qrp_keywords;

	vector<NetAddress> v_poisoners;

	UINT m_version;

	bool m_find_more; //find more option for kazaa
	string m_artist_name;
	string m_album_name;
	
	enum search_type
	{
		everything,
		audio,
		video,
		images,
		documents,
		software,
		playlists,
		none,
	};
	ProjectKeywords::search_type m_search_type;
	vector<HashCount> v_tracks_hash_counts;
	bool m_emule_spoofing_enabled;
	bool m_emule_swarming_enabled;
	UINT m_id;
	int m_kazaa_dist_power;
	bool m_directconnect_spoofing_enabled;
	bool m_directconnect_decoys_enabled;
	bool m_directconnect_supply_enabled;
	bool m_fasttrack_spoofing_enabled;
	bool m_fasttrack_decoys_enabled;
	bool m_fasttrack_swarming_enabled;
	bool m_fasttrack_supply_enabled;
	bool m_gnutella_spoofing_enabled;
	bool m_gnutella_decoys_enabled;
	bool m_gnutella_swarming_enabled;
	bool m_gnutella_supply_enabled;
	bool m_gnutella_demand_enabled;
	bool m_piolet_spoofing_enabled;
	bool m_piolet_supply_enabled;
	bool m_piolet_demand_enabled;
	bool m_emule_decoys_enabled;
	bool m_emule_supply_enabled;
	bool m_usenet_swarming_enabled;
};

#endif // PROJECT_KEYWORDS_H