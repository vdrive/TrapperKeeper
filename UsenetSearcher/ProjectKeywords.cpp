// ProjectKeywords.cpp

#include "stdafx.h"
#include "ProjectKeywords.h"

//
//
//
ProjectKeywords::ProjectKeywords()
{
	Clear();
}

//
//
//
ProjectKeywords::~ProjectKeywords()
{
}

//
//
//
bool ProjectKeywords::operator < (const ProjectKeywords&  other)const
{
	if(strcmp(m_supply_keywords.m_search_string.c_str(), other.m_supply_keywords.m_search_string.c_str())<0)
		return true;
	else
		return false;
}

//
//
//
void ProjectKeywords::Clear()
{
/*
	// Public Data Members
	string m_project_name;

	bool m_bearshare_disting_enabled;
	bool m_poisoning_enabled;
	bool m_uber_disting_enabled;
	bool m_guid_spoofing_enabled;

	unsigned int m_query_multiplier;
	QueryKeywords m_query_keywords;

	SupplyKeywords m_supply_keywords;
*/
	m_project_name.erase();

	m_bearshare_disting_enabled=false;
	m_poisoning_enabled=false;
	m_uber_disting_enabled=false;
	m_guid_spoofing_enabled=false;
	m_auto_multiplier_enabled=false;
	m_project_active=true;

	m_interval = 0;
	
	m_query_multiplier=0;

	m_owner.erase();
	m_viewers.Clear();

	m_query_keywords.Clear();
	
	m_supply_keywords.Clear();

	m_qrp_keywords.Clear();

	v_poisoners.clear();

	m_version = 0x00000003; //version

	m_find_more = false;
	m_artist_name.erase();
	m_album_name.erase();
	m_search_type = ProjectKeywords::audio;
	v_tracks_hash_counts.clear();

	m_emule_spoofing_enabled=false;
	m_emule_swarming_enabled=false;
	m_id = 0;
	m_kazaa_dist_power = 0;

	m_directconnect_spoofing_enabled=false;
	m_directconnect_decoys_enabled=false;
	m_directconnect_supply_enabled=false;
	m_fasttrack_spoofing_enabled=false;
	m_fasttrack_decoys_enabled=false;
	m_fasttrack_swarming_enabled=false;
	m_fasttrack_supply_enabled=false;
	m_gnutella_spoofing_enabled=false;
	m_gnutella_decoys_enabled=false;
	m_gnutella_swarming_enabled=false;
	m_gnutella_supply_enabled=false;
	m_gnutella_demand_enabled=false;
	m_piolet_spoofing_enabled=false;
	m_piolet_supply_enabled=false;
	m_piolet_demand_enabled=false;
	m_emule_decoys_enabled=false;
	m_emule_supply_enabled=false;
	m_usenet_swarming_enabled=false;
}

//
//
//
int ProjectKeywords::GetBufferLength()
{
/*
	// Public Data Members
	string m_project_name;

	bool m_bearshare_disting_enabled;
	bool m_poisoning_enabled;
	bool m_uber_disting_enabled;
	bool m_guid_spoofing_enabled;

	unsigned int m_query_multiplier;
	QueryKeywords m_query_keywords;

	SupplyKeywords m_supply_keywords;
*/	
	int len=0;
	
	len += sizeof(UINT); //m_version

	len+=(int)m_project_name.size()+1;	// +1 for NULL

	len+=sizeof(bool);
	len+=sizeof(bool);
	len+=sizeof(bool);
	len+=sizeof(bool);
	len+=sizeof(bool);
	len+=sizeof(bool);

	len+=sizeof(unsigned int); //interval
	len+=sizeof(unsigned int); //multiplier

	len+=(int)m_owner.size()+1; //+1 for NULL

	len+=m_viewers.GetBufferLength();

	len+=m_query_keywords.GetBufferLength();
	
	len+=m_supply_keywords.GetBufferLength();

	len+=m_qrp_keywords.GetBufferLength();

	len+=sizeof(unsigned int);	// # of poisoner ips
	len+=sizeof(NetAddress)*(int)v_poisoners.size();

	len+=sizeof(bool); //find_more
	len+=(int)m_artist_name.size()+1;
	len+=(int)m_album_name.size()+1;
	len+=sizeof(ProjectKeywords::search_type);

	len+=sizeof(unsigned int);
	len+=sizeof(HashCount)*(int)v_tracks_hash_counts.size();

	if(m_version >= 0x00000002)
	{
		len += sizeof(bool);
		len += sizeof(bool);
	}
	if(m_version >= 0x00000003)
	{
		len += sizeof(UINT);
		len += sizeof(int);
		len += sizeof(bool) * 17;
	}
	return len;
}

//
//
//
int ProjectKeywords::WriteToBuffer(char *buf)
{
	int i;

	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((UINT*)ptr)=m_version;
	ptr+=sizeof(UINT);

	strcpy(ptr,m_project_name.c_str());
	ptr+=m_project_name.size()+1;	// +1 for NULL

	*((bool *)ptr)=m_bearshare_disting_enabled;
	ptr+=sizeof(bool);
	*((bool *)ptr)=m_poisoning_enabled;
	ptr+=sizeof(bool);
	*((bool *)ptr)=m_uber_disting_enabled;
	ptr+=sizeof(bool);
	*((bool *)ptr)=m_guid_spoofing_enabled;
	ptr+=sizeof(bool);
	*((bool *)ptr)=m_auto_multiplier_enabled;
	ptr+=sizeof(bool);
	*((bool *)ptr)=m_project_active;
	ptr+=sizeof(bool);

	*((unsigned int *)ptr)=m_interval;
	ptr+=sizeof(unsigned int);
	
	*((unsigned int *)ptr)=m_query_multiplier;
	ptr+=sizeof(unsigned int);

	strcpy(ptr, m_owner.c_str());
	ptr+=m_owner.size()+1; //+1 for NULL

	ptr+=m_viewers.WriteToBuffer(ptr);

	ptr+=m_query_keywords.WriteToBuffer(ptr);

	ptr+=m_supply_keywords.WriteToBuffer(ptr);

	ptr+=m_qrp_keywords.WriteToBuffer(ptr);

	*((unsigned int *)ptr)=(int)v_poisoners.size();
	ptr+=sizeof(unsigned int);
	
	for(i=0;i<(int)v_poisoners.size();i++)
	{
		*((NetAddress *)ptr)=v_poisoners[i];
		ptr+=sizeof(NetAddress);
	}

	*((bool*)ptr)=m_find_more;
	ptr+=sizeof(bool);
	strcpy(ptr,m_artist_name.c_str());
	ptr+=m_artist_name.size()+1;
	strcpy(ptr,m_album_name.c_str());
	ptr+=m_album_name.size()+1;
	*((ProjectKeywords::search_type *)ptr)=m_search_type;
	ptr+=sizeof(ProjectKeywords::search_type);

	*((unsigned int *)ptr)=(int)v_tracks_hash_counts.size();
	ptr+=sizeof(unsigned int);
	
	for(i=0;i<(int)v_tracks_hash_counts.size();i++)
	{
		*((HashCount *)ptr)=v_tracks_hash_counts[i];
		ptr+=sizeof(HashCount);
	}

	if(m_version >= 0x00000002)
	{
		*((bool *)ptr) = m_emule_spoofing_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_emule_swarming_enabled;
		ptr += sizeof(bool);
	}
	if(m_version >= 0x00000003)
	{
		*((UINT *)ptr) = m_id;
		ptr += sizeof(UINT);
		*((int *)ptr) = m_kazaa_dist_power;
		ptr += sizeof(int);
		*((bool *)ptr) = m_directconnect_spoofing_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_directconnect_decoys_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_directconnect_supply_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_fasttrack_spoofing_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_fasttrack_decoys_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_fasttrack_swarming_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_fasttrack_supply_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_gnutella_spoofing_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_gnutella_decoys_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_gnutella_swarming_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_gnutella_supply_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_gnutella_demand_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_piolet_spoofing_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_piolet_supply_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_piolet_demand_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_emule_decoys_enabled;
		ptr += sizeof(bool);
		*((bool *)ptr) = m_emule_supply_enabled;
		ptr += sizeof(bool);

	}
	return GetBufferLength();
}

//
// Returns buffer length on read
//
int ProjectKeywords::ReadFromBuffer(char *buf)
{
	int i;

	Clear();

	char *ptr=buf;

	m_version = *((UINT *)ptr);
	ptr+=sizeof(UINT);

	m_project_name=ptr;
	ptr+=m_project_name.size()+1;

	m_bearshare_disting_enabled=*((bool *)ptr);
	ptr+=sizeof(bool);
	m_poisoning_enabled=*((bool *)ptr);
	ptr+=sizeof(bool);
	m_uber_disting_enabled=*((bool *)ptr);
	ptr+=sizeof(bool);
	m_guid_spoofing_enabled=*((bool *)ptr);
	ptr+=sizeof(bool);
	m_auto_multiplier_enabled=*((bool *)ptr);
	ptr+=sizeof(bool);
	m_project_active=*((bool *)ptr);
	ptr+=sizeof(bool);

	m_interval=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_query_multiplier=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_owner = ptr;
	ptr+=m_owner.size()+1; //+1 for NULL

	ptr+=m_viewers.ReadFromBuffer(ptr);

	ptr+=m_query_keywords.ReadFromBuffer(ptr);

	ptr+=m_supply_keywords.ReadFromBuffer(ptr);

	ptr+=m_qrp_keywords.ReadFromBuffer(ptr);

	int len=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	for(i=0;i<len;i++)
	{
		v_poisoners.push_back(*((NetAddress *)ptr));
		ptr+=sizeof(NetAddress);
	}

	m_find_more = *((bool*)ptr);
	ptr+=sizeof(bool);
	m_artist_name = ptr;
	ptr+=m_artist_name.size()+1;
	m_album_name = ptr;
	ptr+=m_album_name.size()+1;
	m_search_type = *((ProjectKeywords::search_type *)ptr);
	ptr += sizeof(ProjectKeywords::search_type);

	len=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	for(i=0;i<len;i++)
	{
		v_tracks_hash_counts.push_back(*((HashCount *)ptr));
		ptr+=sizeof(HashCount);
	}

	if(m_version >= 0x00000002)
	{
		m_emule_spoofing_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_emule_swarming_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
	}

	if(m_version >= 0x00000003)
	{
		m_id=*((UINT *)ptr);
		ptr+=sizeof(UINT);
		m_kazaa_dist_power=*((int *)ptr);
		ptr+=sizeof(int);
		m_directconnect_spoofing_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_directconnect_decoys_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_directconnect_supply_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_fasttrack_spoofing_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_fasttrack_decoys_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_fasttrack_swarming_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_fasttrack_supply_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_gnutella_spoofing_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_gnutella_decoys_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_gnutella_swarming_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_gnutella_supply_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_gnutella_demand_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_piolet_spoofing_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_piolet_supply_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_piolet_demand_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_emule_decoys_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
		m_emule_supply_enabled=*((bool *)ptr);
		ptr+=sizeof(bool);
	}
	return GetBufferLength();
}

/*
//
//
//
unsigned int ProjectKeywords::CalculateChecksum()
{
	int i;
	
	unsigned int checksum=0;

	char *buf=new char[GetBufferLength()];
	WriteToBuffer(buf);

	for(i=0;i<GetBufferLength();i++)
	{
		checksum+=buf[i];
	}

	return checksum;
}
*/