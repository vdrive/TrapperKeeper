#include "StdAfx.h"
#include "swarmdata.h"
#include "..\tkcom\tinysql.h"
#include "DonkeyFile.h"
#include "Swarm.h"
#include "SpoofingSystem.h"

SwarmData::SwarmData(void)
{
	mb_inited=false;
}

SwarmData::~SwarmData(void)
{
}


byte SwarmData::ConvertCharToInt(char ch)
{
	switch(ch){
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A': return 10;
		case 'a': return 10;
		case 'B': return 11;
		case 'b': return 11;
		case 'C': return 12;
		case 'c': return 12;
		case 'D': return 13;
		case 'd': return 13;
		case 'E': return 14;
		case 'e': return 14;
		case 'F': return 15;
		case 'f': return 15;
	}
	return 0;
}


void SwarmData::Update(void)
{
	if(mb_inited && !m_last_update.HasTimedOut(1*60*60)){
		return;
	}
	
	mb_inited=true;
	m_last_update.Refresh();
	
	static const int GETSWARMS				= 1;
	static const int ADDSOURCE				= 2;
	static const int REMOVESOURCE			= 3;
	static const int GETINTERDICTIONTARGETS	= 4;
	static const int GETSOURCEHASHES		= 5;
	static const int GETFALSEDECOYS			= 6;
	static const int GETSPOOFS				= 7;

	byte key=0xe4;

	Buffer2000 response_packet;
	response_packet.WriteByte(1);  //version
	response_packet.WriteByte(1);  //its an question for the server
	response_packet.WriteByte(GETSWARMS);  //swarms
	response_packet.WriteByte(key);  //swarms

	MetaSpooferReference ref;
	ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());
	ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet.GetBufferPtr(),response_packet.GetLength());

	/*
	CSingleLock lock(&m_data_lock,TRUE);

	m_last_update.Refresh();

	mv_swarms.Clear();
	
	CString query;
	query.Format("select project,file_name,size,hash,hash_set from swarm_table order by rand() limit 20000");

	TinySQL sql;
	sql.Init("38.119.64.66","onsystems","tacobell","metamachine",3306);
	sql.Query(query,true);	

	vector <string> v_projects;  //only add so many swarms per project
	vector <int> v_counts;

	int add_hash_that_needs_sources_count=0;

	for(int i=(int)sql.m_num_rows-1;i>=0;i--){
		CString str_hash=sql.mpp_results[i][3].c_str();

		if(str_hash.GetLength()<32)
			continue;  //not valid data
		byte byte_hash[16];
		byte *byte_hash_set=NULL;
		
		for(int j=0;j<16;j++){
			char ch1=str_hash[j*2];
			char ch2=str_hash[j*2+1];
			byte val1=ConvertCharToInt(ch1);
			byte val2=ConvertCharToInt(ch2);
			byte hash_val=((val1&0xf)<<4)|(val2&0xf);
			byte_hash[j]=hash_val;
		}
		int the_size=atoi(sql.mpp_results[i][2].c_str());

		int num_parts=the_size/PARTSIZE;
		if(num_parts>0){
			num_parts++;
		}

		if(num_parts>0 && the_size>=PARTSIZE){  //try to set the hash set data
			CString str_hash_set=sql.mpp_results[i][4].c_str();
			if(str_hash_set.GetLength()!=num_parts*32){

				if(add_hash_that_needs_sources_count++<10){
					MetaSpooferReference ref;
					ref.System()->AddHashThatNeedsSources(str_hash);
				}
				continue;  //we don't want to add this swarm, since we don't have all the data we need
			}
			byte_hash_set=new byte[num_parts*16];
			for(int part_index=0;part_index<num_parts;part_index++){
				for(int j=0;j<16;j++){
					char ch1=str_hash_set[part_index*32+j*2];
					char ch2=str_hash_set[part_index*32+j*2+1];
					byte val1=ConvertCharToInt(ch1);
					byte val2=ConvertCharToInt(ch2);
					byte hash_val=((val1&0xf)<<4)|(val2&0xf);
					byte_hash_set[part_index*16+j]=hash_val;
				}				
			}				
		}

		const char* project=sql.mpp_results[i][0].c_str();
		bool b_too_many=false;
		bool b_found=false;
		for(int j=0;j<(int)v_projects.size();j++){
			if(stricmp(v_projects[j].c_str(),project)==0){
				v_counts[j]++;  
				b_found=true;
				if(v_counts[j]>400){  //only allow 400 swarms per project, so some projects don't completely drown out others
					b_too_many=true;
				}
				break;
			}
		}
		if(!b_found){  //if we don't have a count going for this project, then add one
			v_projects.push_back(string(project));
			v_counts.push_back(1);
		}

		if(!b_too_many){
			mv_swarms.Add(new Swarm(sql.mpp_results[i][1].c_str(),the_size,byte_hash,byte_hash_set));
		}
		else{
			if(byte_hash_set){
				delete []byte_hash_set;
			}
		}
	}*/
}

void SwarmData::Reset(void)
{
	mv_swarms.Clear();
}

void SwarmData::AddSwarm(const char* project, const char *hash, const char* hash_set,const char* file_name, UINT file_size)
{
	CString str_hash=hash;

	if(MUSIC_ONLY && file_size>100000000)  //if we are supposedly only a music only client, lets disregard any movie swarms
		return;

	if(str_hash.GetLength()<32)
		return;  //not valid data
	byte byte_hash[16];
	byte *byte_hash_set=NULL;
	
	for(int j=0;j<16;j++){
		char ch1=str_hash[j*2];
		char ch2=str_hash[j*2+1];
		byte val1=ConvertCharToInt(ch1);
		byte val2=ConvertCharToInt(ch2);
		byte hash_val=((val1&0xf)<<4)|(val2&0xf);
		byte_hash[j]=hash_val;
	}
	UINT the_size=file_size;

	int num_parts=the_size/PARTSIZE;
	if(num_parts>0){
		num_parts++;
	}

	if(num_parts>0 && the_size>=PARTSIZE){  //try to set the hash set data
		CString str_hash_set=hash_set;
		if(str_hash_set.GetLength()!=num_parts*32){
			return;  //we don't want to add this swarm, since we don't have all the data we need (hashset)
		}
		byte_hash_set=new byte[num_parts*16];
		for(int part_index=0;part_index<num_parts;part_index++){
			for(int j=0;j<16;j++){
				char ch1=str_hash_set[part_index*32+j*2];
				char ch2=str_hash_set[part_index*32+j*2+1];
				byte val1=ConvertCharToInt(ch1);
				byte val2=ConvertCharToInt(ch2);
				byte hash_val=((val1&0xf)<<4)|(val2&0xf);
				byte_hash_set[part_index*16+j]=hash_val;
			}				
		}				
	}

	for(int i=0;i<(int)mv_swarms.Size();i++){  //check if we have already added this swarm
		Swarm *sm=(Swarm*)mv_swarms.Get(i);
		if(memcmp(sm->m_hash,byte_hash,16)==0){
			delete []byte_hash_set;
			return;
		}
	}

	mv_swarms.Add(new Swarm(file_name,the_size,byte_hash,byte_hash_set));
}

