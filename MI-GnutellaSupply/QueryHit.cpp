// QueryHit.cpp

#include "stdafx.h"
#include "QueryHit.h"

//
//
//
QueryHit::QueryHit()
{
	//p_filename=NULL;
//	p_info=NULL;

	Clear();
}

//
//
//
QueryHit::QueryHit(const QueryHit &hit)
{
	//p_filename=NULL;
//	p_info=NULL;

//	m_guid=hit.m_guid;
	//m_port=hit.m_port;
	m_ip=hit.m_ip;
	//m_speed=hit.m_speed;
	//m_file_index=hit.m_file_index;
	//m_file_size=hit.m_file_size;
	m_track=hit.m_track;
	m_timestamp = hit.m_timestamp;


//	m_is_searcher_query_hit=hit.m_is_searcher_query_hit;

	//m_is_spoof_dist_poison=hit.m_is_spoof_dist_poison;

	//p_filename=new char[strlen(hit.p_filename)+1];
	//strcpy(p_filename,hit.p_filename);

//	p_info=new char[strlen(hit.p_info)+1];
//	strcpy(p_info,hit.p_info);

	strcpy(m_sha1, hit.m_sha1);
	m_project = hit.m_project;
	m_project_id = hit.m_project_id;
}


//
//
//
QueryHit::~QueryHit()
{
	//if(p_filename!=NULL)
	//{
	//	delete [] p_filename;
	//	p_filename=NULL;
	//}
/*
	if(p_info!=NULL)
	{
		delete [] p_info;
		p_info=NULL;
	}
*/
}

//
//
//
QueryHit& QueryHit::operator=(const QueryHit &hit)
{
	Clear();

//	m_guid=hit.m_guid;
	//m_port=hit.m_port;
	m_ip=hit.m_ip;
	//m_speed=hit.m_speed;
	//m_file_index=hit.m_file_index;
	//m_file_size=hit.m_file_size;
	m_track=hit.m_track;
	m_timestamp = hit.m_timestamp;

//	m_is_searcher_query_hit=hit.m_is_searcher_query_hit;

	//m_is_spoof_dist_poison=hit.m_is_spoof_dist_poison;

	//p_filename=new char[strlen(hit.p_filename)+1];
	//strcpy(p_filename,hit.p_filename);

//	p_info=new char[strlen(hit.p_info)+1];
//	strcpy(p_info,hit.p_info);
	strcpy(m_sha1, hit.m_sha1);
	m_project = hit.m_project;
	m_project_id = hit.m_project_id;
	return *this;
}

//
//
//
void QueryHit::Clear()
{
//	memset(&m_guid,0,sizeof(GUID));
	memset(&m_sha1,0,sizeof(m_sha1));
	//m_port=0;
	m_ip=0;
	//m_speed=0;
	//m_file_index=0;
	//m_file_size=0;
	m_track=0;
	m_timestamp = CTime::GetCurrentTime();
	
//	m_is_searcher_query_hit=false;

	//m_is_spoof_dist_poison=false;

	//if(p_filename!=NULL)
	//{
	//	delete [] p_filename;
	//	p_filename=NULL;
	//}
	m_project_id=0;
/*
	if(p_info!=NULL)
	{
		delete [] p_info;
		p_info=NULL;
	}
*/
	m_project.Empty();
}

//
//
//
//char *QueryHit::Filename()
//{
//	return p_filename;
//}

//
//
//
//void QueryHit::Filename(char *filename)
//{
//	if(p_filename!=NULL)
//	{
//		delete [] p_filename;
//		p_filename=NULL;
//	}
//
//	p_filename=new char[strlen(filename)+1];
//	strcpy(p_filename,filename);
//}

//
//
//
/*
char *QueryHit::Info()
{
	return p_info;
}

//
//
//
void QueryHit::Info(char *info)
{
	if(p_info!=NULL)
	{
		delete [] p_info;
		p_info=NULL;
	}

	p_info=new char[strlen(info)+1];
	strcpy(p_info,info);
}
*/
//
// The entire query hit message is what is passed in as char *data (static function)
//

bool QueryHit::IsSpoofDistPoison(char *data,int len)
{
	int i;

	// If the private data is 409, then this is a spoof/dist/poison
	char *ptr=data;

	// Extract the query hit header items
	unsigned char *num_hits=(unsigned char *)ptr;
	ptr++;
	unsigned short int *port=(unsigned short int *)ptr;
	ptr+=sizeof(unsigned short int);
	unsigned int *ip=(unsigned int *)ptr;
	ptr+=sizeof(unsigned int);
	unsigned int *speed=(unsigned int *)ptr;
	ptr+=sizeof(unsigned int);

	// Extract the result sets
	for(i=0;i<*num_hits;i++)
	{
		unsigned int *file_index=(unsigned int *)ptr;
		ptr+=sizeof(unsigned int);
		unsigned int *file_size=(unsigned int *)ptr;
		ptr+=sizeof(unsigned int);

		// Check that we haven't gone beyond the bounds of the data
		if((int)(ptr-data)>len){return false;}		
		char *filename=ptr;
		// Check to see that the string is valid
		//if(IsStringValid(filename)==false){return false;}		
		ptr+=strlen(filename)+1;

		// Check that we haven't gone beyond the bounds of the data
		if((int)(ptr-data)>len){return false;}		
		char *info=ptr;
		// Check to see that the string is valid
		//if(IsStringValid(info)==false){return false;}		
		ptr+=strlen(info)+1;
	}

	// Now extract the trailer
	char *vendor=ptr;
	ptr+=4;

	// Check that we haven't gone beyond the bounds of the data
	if((int)(ptr-data)>len){return false;}		

	unsigned char *open_data_size=(unsigned char *)ptr;
	ptr++;
	ptr+=*open_data_size;	// skip over open data

	// Check that we haven't gone beyond the bounds of the data
	if((int)(ptr-data)>len){return false;}		

	// Read the next 2 bytes ... which may be private data, and see if it is 409
	unsigned short int *private_data=(unsigned short int *)ptr;
	if(*private_data==409)
	{
		return true;
	}

	return false;
}

//
//
//
int QueryHit::GetBufferLength()
{
	int len=0;
	
	//len+=sizeof(m_file_index);
	//len+=sizeof(m_file_size);
//	len+=sizeof(m_guid);
	len+=sizeof(m_ip);
	//len+=sizeof(m_is_spoof_dist_poison);
	//len+=sizeof(m_port);
	len+=sizeof(m_sha1);
	//len+=sizeof(m_speed);
	len+=sizeof(m_timestamp);
	len+=sizeof(m_track);
	len+=m_project.GetLength()+1;
	//if(p_filename != NULL)
	//	len += (int)strlen(p_filename)+1;
	len+=sizeof(m_project_id);
	
	return len;
}

//
//
//
int QueryHit::WriteToBuffer(char *buf)
{
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	//*((UINT*)ptr)=m_file_index;
	//ptr+=sizeof(UINT);

	//*((UINT*)ptr)=m_file_size;
	//ptr+=sizeof(UINT);

	//*((GUID*)ptr)=m_guid;
	//ptr+=sizeof(GUID);

	*((int*)ptr)=m_ip;
	ptr+=sizeof(int);

	//*((bool*)ptr)=m_is_spoof_dist_poison;
	//ptr+=sizeof(bool);

	//*((unsigned short int*)ptr)=m_port;
	//ptr+=sizeof(unsigned short int);

	memcpy(ptr, m_sha1, sizeof(m_sha1));
	ptr+=sizeof(m_sha1);

	//*((UINT*)ptr)=m_speed;
	//ptr+=sizeof(UINT);

	*((CTime*)ptr)=m_timestamp;
	ptr+=sizeof(CTime);

	*((int*)ptr)=m_track;
	ptr+=sizeof(int);

	strcpy(ptr, m_project);
	ptr+=m_project.GetLength()+1; //+1 for NULL

	//if(p_filename != NULL)
	//	strcpy(ptr, p_filename);
	//ptr+=(int)strlen(p_filename)+1;

	*((UINT*)ptr)=m_project_id;
	ptr+=sizeof(UINT);


	return GetBufferLength();
}

//
// Returns buffer length on read
//
int QueryHit::ReadFromBuffer(char *buf)
{
	Clear();

	char *ptr=buf;

	//m_file_index = *((UINT *)ptr);
	//ptr+=sizeof(UINT);

	//m_file_size = *((UINT *)ptr);
	//ptr+=sizeof(UINT);

	//m_guid = *((GUID *)ptr);
	//ptr+=sizeof(GUID);

	m_ip = *((int *)ptr);
	ptr+=sizeof(int);

	//m_is_spoof_dist_poison = *((bool *)ptr);
	//ptr+=sizeof(bool);

	//m_port = *((unsigned short int *)ptr);
	//ptr+=sizeof(unsigned short int);

	memcpy(m_sha1, ptr, sizeof(m_sha1));
	ptr+=sizeof(m_sha1);

	//m_speed = *((UINT *)ptr);
	//ptr+=sizeof(UINT);


	m_timestamp = *((CTime *)ptr);
	ptr+=sizeof(CTime);

	m_track = *((int *)ptr);
	ptr+=sizeof(int);

	m_project=ptr;
	ptr+=m_project.GetLength()+1;

	//Filename(ptr);

	//if(p_filename!=NULL)
	//	ptr+=(int)strlen(p_filename)+1;

	m_project_id = *((UINT *)ptr);
	ptr+=sizeof(UINT);

	return GetBufferLength();
}

//bool QueryHit::IsSwarm()
//{
//	//see if this is swarming
//	UINT ip1,ip2,ip3,ip4;
//	ip1=(m_ip>>0)&0xFF;
//	ip2=(m_ip>>8)&0xFF;
//	ip3=(m_ip>>16)&0xFF;
//	ip4=(m_ip>>24)&0xFF;
//	if( (ip1==38 && ip2==118) ||
//		((ip1==66 && ip2==54) && (ip3==72 || ip3==73 ||ip3==74)) )
//		return true;
//	return false;
//}