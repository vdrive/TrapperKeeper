#include "StdAfx.h"
#include "overnetudpsocket.h"
#include "iphlpapi.h"
#include "..\OvernetSpoofer\PacketBuffer.h"
#include "ConnectionModule.h"
#include "..\SupplyProcessor\SupplyData.h"

//#include "RegistryKeys.h"
//#define NUM_DECOYS_PER_TRACK	200
//#define NUM_DECOYS_PER_MOVIE	500
#define IP_HDRINCL 2	// for ip spoofing

OvernetUDPSocket::OvernetUDPSocket(void)
{
	m_last_round_start_time=CTime::GetCurrentTime();
	m_hash_search_index=m_seconds=m_my_ip=m_publish_rate=m_publish_ack_received=0;
	m_round=1;
	m_my_tcp_port=4662;
	m_publish_file_hashes_completed=false;
	GetMyIP();
	srand(m_my_ip);
	for(int i=0;i<sizeof(m_my_id);i++) //randomly create my user id hash
		m_my_id[i]=rand()%256;
	srand((unsigned)time(NULL));
	//TRACE("My user ID hash:%s\n",m_df.EncodeBase16(m_my_id,16));
	m_my_udp_port=8730;
	m_publicize_sent=m_publicize_received=m_publicize_acks_received=m_files_published=m_keywords_published=0;

	for(int i=0;i<=0xFF;i++)
	{
		UserGroupHashes group(i);
		v_group_hashes.push_back(group);
	}
//	ReadInHashes();

	int ret=0;
	ret = WSocket::Startup();
	ret = Create(m_my_udp_port);

	SendConnect("127.0.0.1",3739);
	SendConnect("127.0.0.1",3739);
}

//
//
//
OvernetUDPSocket::~OvernetUDPSocket(void)
{
}

//
//
//
void OvernetUDPSocket::GetMyIP()
{
	char iai_buf[2048];
	DWORD iai_buf_len=2048;
	IP_ADAPTER_INFO *iai=(IP_ADAPTER_INFO *)iai_buf;
	GetAdaptersInfo(iai,&iai_buf_len);
	if(strlen( iai->IpAddressList.IpAddress.String) > 0)
	{
		m_my_ip = inet_addr(iai->IpAddressList.IpAddress.String);
		strcpy(m_my_ip_str, iai->IpAddressList.IpAddress.String);
	}
}

//
//
//
void OvernetUDPSocket::InitHwnd(HWND hwnd)
{
	m_hwnd=hwnd;
}

//
//
//
void OvernetUDPSocket::InitParent(ConnectionModule* parent)
{
	p_connection_module = parent;
}

//
//
//
void OvernetUDPSocket::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	TRACE("Searching %s\n",psq.m_search_string.c_str());
	hash_set<OvernetPeer>::iterator iter_peer = v_group_hashes[psq.m_keyword_hash[0]].hs_peers.begin();
	UINT search_count=0;
	UINT search_info_count=0;
	while(iter_peer != v_group_hashes[psq.m_keyword_hash[0]].hs_peers.end())
	{
		if(iter_peer->m_id[1]==psq.m_keyword_hash[1] )
		{
			SendSearchInfo(iter_peer->m_ip,iter_peer->m_port,psq);
			search_info_count++;
		}
		iter_peer++;
	}
	if(search_info_count<10)
	{
		//while(search_count < 100-search_info_count)
		iter_peer = v_group_hashes[psq.m_keyword_hash[0]].hs_peers.begin();
		while(iter_peer != v_group_hashes[psq.m_keyword_hash[0]].hs_peers.end())
		{
			//iter_peer = v_group_hashes[psq.m_keyword_hash[0]].hs_peers.begin();
			//int random = rand()%(v_group_hashes[psq.m_keyword_hash[0]].hs_peers.size());
			//for(int i=0;i<random;i++)
			//	iter_peer++;
			//SendSearch(psq.m_keyword_hash,iter_peer->m_ip,iter_peer->m_port,0x02);
			//if(iter_peer->m_id[1]!=psq.m_keyword_hash[1])
			int disc=GetHashDistince(iter_peer->m_id,psq.m_keyword_hash);
			if(disc<=4 && disc!=0)
			{
				SendSearchInfo(iter_peer->m_ip,iter_peer->m_port,psq);
				search_count++;
			}
			iter_peer++;
		}
	}
	CString msg;
	CString search_string = psq.v_search_words[0];
	for(UINT i=1;i<psq.v_search_words.size();i++)
		search_string += " " + psq.v_search_words[i];
	msg.Format("Searching Project: %s  - track:%d - Search string: %s - Searching Sent: %d - Searching Nodes: %d/%d\n", psq.m_project.c_str(),
		psq.m_track,search_string,search_info_count,search_count,v_group_hashes[psq.m_keyword_hash[0]].hs_peers.size());
	p_connection_module->Log(msg);
}

//
//
//
int OvernetUDPSocket::GetHashDistince(byte* hash1, byte* hash2)
{
	int dist=0;
	if(hash1[0]==hash2[0])
	{
		if(hash1[1]==hash2[1])
			return 0;
		else
			return abs(hash1[1]-hash2[1]);
	}
	return 999999;
}

//
//
//
void OvernetUDPSocket::SendSearchInfo(UINT ip, unsigned short int port, ProjectSupplyQuery& psq, unsigned short int start_pos)
{
	if(IsOneOfUs(ip))
		return;
	PacketBuffer buf;
	unsigned short int len;
	buf.PutByte(0xe3);	//edonkey
	buf.PutByte(0x10);	//Search Info
	buf.PutUStr((const unsigned char*)psq.m_keyword_hash,16);	//keyword hash
	buf.PutByte(0x01);
	for(int i=0;i<psq.v_search_words.size()-1;i++)
		buf.PutUShort(0);
	for(i=1;i<psq.v_search_words.size();i++)
	{
		len = psq.v_search_words[i].GetLength();
		buf.PutByte(0x01);
		buf.PutUShort(len);
		buf.PutStr(psq.v_search_words[i],len);
	}
	len=psq.m_search_type.GetLength();
	buf.PutByte(0x02); //search type
	buf.PutUShort(len);
	buf.PutStr(psq.m_search_type,len);
	buf.PutByte(0x01); buf.PutByte(0x00); buf.PutByte(0x03);
	buf.PutUShort(start_pos);
	buf.PutByte(0x64); buf.PutByte(0x00);

	UINT num_sent=0;
	int ret = SendTo(buf.p_data,buf.Size(),ip,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}

	//CString msg;
	//in_addr in;
	//in.S_un.S_addr=ip;
	//CString from_ip = inet_ntoa(in);
	//msg.Format("Sent Search to %s:%d for project %s - starting: %d",
	//from_ip,port,psq.m_project.c_str(),start_pos);
	//p_connection_module->Log(msg);
}

//
//
//
void OvernetUDPSocket::UpdateProjectKeywords(vector<ProjectKeywords>& keywords)
{
	v_all_project_keywords = keywords;
}

/*
hash_set<SearchSet,SearchSetHash>* OvernetUDPSocket::ReturnHashesPtr()
{
	return &hs_hashes;
}
*/

void OvernetUDPSocket::OnReceive(int error_code)
{
	char buf[4096];
	memset(buf,0,sizeof(buf));

	unsigned int ip=0;
	unsigned short int port=0;
	unsigned int num_read=0;
	int ret=ReceiveFrom(buf,sizeof(buf),&ip,&port,&num_read);
	if((byte)buf[0]!=0xe3) //check to see if it's overnet message
		return;

	byte* ptr = (byte*)&buf[1];
	byte op_code  = *ptr;
	switch(op_code)
	{
		case 0x0b: //Connect Reply
		{
			ptr++;
			unsigned short int list_size = *(unsigned short int*)ptr;
			ptr+=2;
			for(int i=0; i<list_size;i++)
			{
				OvernetPeer peer;
				UINT this_ip = *(UINT*)(ptr+16);
				unsigned short int this_port = *(unsigned short int*)(ptr+20);
				peer.SetID(ptr,this_ip,this_port);
				ptr+=23;
				pair< hash_set<OvernetPeer>::iterator, bool > pr;
				pr = v_group_hashes[peer.m_id[0]].hs_peers.insert(peer);
				if(pr.second)
				{
					//SendSearch(m_my_id,this_ip,this_port,0x14); //searching peer hash similar to me
					SendConnect(this_ip,this_port);
				}
			}
			break;
		}
		case 0x0f: //Search Next
		{
				ptr++;
				byte hash[16];
				memcpy(hash,ptr,16);

				ptr+=16;
				byte list_size = *ptr;
				ptr++;
				for(int i=0; i<(int)list_size;i++)
				{
					OvernetPeer peer;
					UINT this_ip = *(UINT*)(ptr+16);
					unsigned short int this_port = *(unsigned short int*)(ptr+20);
					peer.SetID(ptr,this_ip,this_port);
					ptr+=23;

					pair< hash_set<OvernetPeer>::iterator, bool > pr;
					pr = v_group_hashes[peer.m_id[0]].hs_peers.insert(peer);
						if(peer.m_id[0]==hash[0])
						{
							ProjectSupplyQuery psq = p_connection_module->GetPSQ(hash);
							if(psq.m_project_id!=0)
							{
								SendSearchInfo(peer.m_ip,peer.m_port,psq);
								//CString msg;
								//msg.Format("Search Next received - searching project %s",psq.m_project.c_str());
								//p_connection_module->Log(msg);
							}
						}
				}
			break;
		}
		case 0x11: //Search Result
		{
			ptr++;
			byte hash[16];
			memcpy(hash,ptr,16);
			ProjectSupplyQuery psq = p_connection_module->GetPSQ(hash);
			ptr+=16;
			byte file_hash[16];
			memcpy(file_hash,ptr,16);
			ptr+=16;
			UINT meta_list_size = *(UINT*)ptr;
			ptr+=4;
			SupplyData supply;
			for(UINT i=0;i<meta_list_size; i++)
			{
				byte type=*(byte*)ptr; ptr++;
				if(type == 0x02)//string type
				{
					unsigned short int tag_size = *(unsigned short int*)ptr; ptr+=2;
					if(tag_size==1)
					{
						byte tag_name = *(byte*)ptr; ptr++;
						switch(tag_name)
						{
							case 0x01: //filename
							{
								unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
								char* filename=new char[string_len+1];
								//ZeroMemory(filename,string_len+1);
								memcpy(filename,ptr,string_len); ptr+=string_len;
								filename[string_len]='\0';
								supply.m_filename=filename;
								CString lowered_filename = filename;
								delete [] filename;

								bool music_project=false;
								if(psq.m_search_type.CompareNoCase("audio")==0)
									music_project=true;

								lowered_filename.MakeLower();
								int weight = CheckRequiredKeywords(psq,lowered_filename);
								int track=-1;
								if(weight >= 100)
								{
									if(music_project)
										track=CheckQueryHitRequiredKeywords(psq,lowered_filename);
									else
										track=0;
								}
								else
								{
									return;
								}
								if( (music_project && track >0) || !music_project)
									supply.m_track=track;
								else
									return;
								break;
							}
							case 0x03: //type
							{
								unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
								char* file_type=new char[string_len+1];
								//ZeroMemory(file_type,string_len+1);
								memcpy(file_type,ptr,string_len); ptr+=string_len;
								file_type[string_len]='\0';
								supply.m_file_type=file_type;
								delete [] file_type;
								break;
							}
							case 0x04: //format
							{
								unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
								ptr+=string_len;
								break;
							}
							case 0x12: //Temporary file
							{
								unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
								ptr+=string_len;
								break;
								break;
							}
							default:
							{
								p_connection_module->Log("Corrupt Search Result received - type=0x02 - tag_size=1");
								return;
							}
						}
					}
					else if(tag_size > 1)
					{
						char* tag_name=new char[tag_size+1];
						//ZeroMemory(tag_name,tag_size+1);
						memcpy(tag_name,ptr,tag_size); ptr+=tag_size;
						tag_name[tag_size]='\0';
						if(stricmp(tag_name, "Artist")==0)
						{
							unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
							char* str=new char[string_len+1];
							//ZeroMemory(str,string_len+1);
							memcpy(str,ptr,string_len); ptr+=string_len;
							str[string_len]='\0';
							supply.m_artist=str;
							delete [] str;

						}
						else if(stricmp(tag_name, "Album")==0)
						{
							unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
							char* str=new char[string_len+1];
							//ZeroMemory(str,string_len+1);
							memcpy(str,ptr,string_len); ptr+=string_len;
							str[string_len]='\0';
							supply.m_album=str;
							delete [] str;
							
						}
						else if(stricmp(tag_name, "Title")==0)
						{
							unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
							char* str=new char[string_len+1];
							//ZeroMemory(str,string_len+1);
							memcpy(str,ptr,string_len); ptr+=string_len;
							str[string_len]='\0';
							supply.m_title=str;
							delete [] str;
							
						}
						else if(stricmp(tag_name, "codec")==0)
						{
							unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
							char* str=new char[string_len+1];
							//ZeroMemory(str,string_len+1);
							memcpy(str,ptr,string_len); ptr+=string_len;
							str[string_len]='\0';
							supply.m_codec=str;
							delete [] str;							
						}
						else if(stricmp(tag_name, "length")==0)
						{
							unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
							char* str=new char[string_len+1];
							//ZeroMemory(str,string_len+1);
							memcpy(str,ptr,string_len); ptr+=string_len;
							str[string_len]='\0';

							int hour,min,sec;
							hour=min=sec=-1;
							if(strchr(str,':')) //in 03:45:35 format
							{
								sscanf(str,"%d:%d:%d",&hour,&min,&sec);
								if(sec==-1)
								{
									sec=min;
									min=hour;
									hour=0;
								}
							}
							else if(strchr(str,'s')) //in 1h 15m 30s format
							{
								char* len_ptr = str;
								if(strchr(str,'h'))
								{
									sscanf(str,"%d",&hour);
									len_ptr=strchr(str,'h');
									len_ptr++;
								}
								else
									hour=0;
								if(strchr(str,'m'))
								{
									sscanf(len_ptr,"%d",&min);
									len_ptr=strchr(str,'m');
									len_ptr++;
								}
								else
									min=0;

								sscanf(len_ptr,"%d",&sec);
							}
							else
								hour=min=sec=0;
							supply.m_media_len=hour*3600+min*60+sec;
							delete [] str;							
						}
						else if(stricmp(tag_name, "ain")==0)
						{
							unsigned short int string_len = *(unsigned short int*)ptr; ptr+=2;
							ptr+=string_len;
						}
						else
						{
							p_connection_module->Log("Corrupt Search Result received - type=0x02 - tag_size > 1");
							delete [] tag_name;
							return;
						}
						delete [] tag_name;
					}					
				}
				else if(type == 0x03) //integer type
				{
					unsigned short int tag_size = *(unsigned short int*)ptr; ptr+=2;
					if(tag_size==1)
					{
						byte tag_name = *(byte*)ptr; ptr++;
						switch(tag_name)
						{
							case 0x02: //filesize
							{
								supply.m_filesize= *(UINT*)ptr; ptr+=sizeof(UINT);
								
								//check for filesize threshold for this project
								if( (supply.m_filesize < psq.m_supply_size_threshold))// || (supply.m_filesize%137==0) )
									return;
								break;
							}
							case 0x05: //Collection
							{
								ptr+=sizeof(int);
								break;
							}
							case 0x08: //Copied
							{
								ptr+=sizeof(int);
								break;
							}
							case 0x15: //Availability
							{
								supply.m_count=*(int*)ptr; ptr+=sizeof(int);
								break;
							}
							case 0x30: //0
							{
								ptr+=sizeof(int);
								break;
							}
							default:
							{
								CString msg;
								msg.Format("Corrupt Search Result received - type=0x03 - tag_size=1 - tag_name=%d",(int)tag_name);
								p_connection_module->Log(msg);
								return;
							}

						}
					}
					else if(tag_size>1)
					{
						char* tag_name=new char[tag_size+1];
						//ZeroMemory(tag_name,tag_size+1);
						memcpy(tag_name,ptr,tag_size); ptr+=tag_size;
						tag_name[tag_size]='\0';
						if(stricmp(tag_name, "bitrate")==0)
						{
							supply.m_bitrate=*(int*)ptr; ptr+=sizeof(int);
						}
						else
						{
							p_connection_module->Log("Corrupt Search Result received - type=0x03 - tag_size > 1");
							delete [] tag_name;
							return;
						}
						delete [] tag_name;
					}
				}
			}
			supply.m_project_id=psq.m_project_id;
			supply.m_network_name="Overnet";
			supply.m_hash = m_df.EncodeBase16(file_hash,16);
			p_connection_module->m_status_data.v_supply_data.push_back(supply);
			break;
		}
		case 0x12: //Search End
		{
			ptr++;
			byte hash[16];
			memcpy(hash,ptr,16);
			ptr+=16;
			unsigned short int result_start = *(unsigned short int*)ptr;
			ptr+=2;
			unsigned short int result_end = *(unsigned short int*)ptr;
			ProjectSupplyQuery psq = p_connection_module->GetPSQ(hash);
			if(result_start!=result_end)
			{
				if(psq.m_project_id!=0)
				{
					//CString msg;
					//in_addr in;
					//in.S_un.S_addr=ip;
					//CString from_ip = inet_ntoa(in);
					//msg.Format("Search End received from %s:%d - %s - %d/%d",
					//	from_ip,port,psq.m_project.c_str(),result_start,result_end);
					//p_connection_module->Log(msg);
					SendSearchInfo(ip,port,psq, result_start);
				}
			}
			//else
			//{
			//	CString msg;
			//	in_addr in;
			//	in.S_un.S_addr=ip;
			//	CString from_ip = inet_ntoa(in);
			//	msg.Format("Search End received from %s:%d - %s - %d/%d",
			//		from_ip,port,psq.m_project.c_str(),result_start,result_end);
			//	p_connection_module->Log(msg);				
			//}
			break;
		}
	}
}
		/*
		case 0x0c: //Publicize
		{
			
			ptr++;
			OvernetPeer peer;
			peer.SetID(ptr,ip,port);
			//peer.m_publicized_from=true;
			//peer.m_publicized_to=true;
			//peer.m_last_publish_time=CTime::GetCurrentTime();
			pair< hash_set<OvernetPeer>::iterator, bool > pr;
			pr=v_group_hashes[peer.m_id[0]].hs_peers.insert(peer);

			if(pr.second)
				SendPublicizeMyself(ip,port);
			SendPublicizeAck(ip,port);
			
			break;
		}
		case 0x1c: //IP Query Answer
		{
			ptr++;
			m_my_ip = *(UINT*)ptr;
			break;
		}
		case 0x0e: //Search
		{
			ptr++;
			if(*ptr == 0x14) //searching for net ID
			{
				//we will handle that later, screw them for now
			}
			break;
		}
		case 0x0d: //Publicize Ack
		{
			m_publicize_acks_received++;
			break;
		}
		case 0x14: //publish ack
		{
			m_publish_ack_received++;
			m_last_publish_ack_received=CTime::GetCurrentTime();
			ptr++;
			break;
		}
		default:
		{
			TRACE("Unknown opcode: 0x%.2x\n", op_code);
			break;
		}
	}

	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
		return; 
	}

}
*/
//
//
//
void OvernetUDPSocket::SendConnect(const char* ip, unsigned short int port)
{
	unsigned char buf[25];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x0a;
	memcpy(&buf[2],m_my_id,16);
	*(int*)&buf[18]=0;
	*(unsigned short int*)&buf[22]=m_my_udp_port;
	*(byte*)&buf[24]=0x00;

	unsigned long ip_int = inet_addr(ip);
	UINT num_sent=0;
	int ret = SendTo(buf,sizeof(buf),ip_int,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
}

void OvernetUDPSocket::SendConnect(UINT ip, unsigned short int port)
{
	unsigned char buf[25];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x0a;
	memcpy(&buf[2],m_my_id,16);
	*(int*)&buf[18]=0;
	*(unsigned short int*)&buf[22]=m_my_udp_port;
	*(byte*)&buf[24]=0x00;

	UINT num_sent=0;
	int ret = SendTo(buf,sizeof(buf),ip,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
}


//
//
//
UINT OvernetUDPSocket::ReturnNumberOfPeers()
{
	UINT size=0;
	for(UINT i=0;i<v_group_hashes.size();i++)
		size += (UINT)v_group_hashes[i].hs_peers.size();
	return size;
}

//
//
//
void OvernetUDPSocket::SendSearch(byte* id, UINT ip, unsigned short int port, byte search_type)
{
	unsigned char buf[19];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x0e;
	*(byte*)&buf[2]=search_type;
	memcpy(&buf[3],id,16);

	UINT num_sent=0;
	int ret = SendTo(buf,sizeof(buf),ip,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
}

//
//
//
int OvernetUDPSocket::CheckRequiredKeywords(ProjectSupplyQuery& psq,const char *lfilename)
{
	int weight=0;
	UINT j,k;
	// Extract the keywords from the supply
	vector<string> keywords;
	ExtractKeywordsFromSupply(&keywords,lfilename);

	// Check all of the keywords for this project
	for(j=0;j<psq.m_query_keywords.v_keywords.size();j++)
	{
		// Against all of the keywords for this query
		for(k=0;k<keywords.size();k++)
		{
			if(strstr(keywords[k].c_str(),psq.m_query_keywords.v_keywords[j].keyword.c_str())!=0)
			{
				weight+=psq.m_query_keywords.v_keywords[j].weight;
			}
		}
	}

	// Check all of the exact keywords for this project
	for(j=0;j<psq.m_query_keywords.v_exact_keywords.size();j++)
	{
		// Against all of the keywords for this query
		for(k=0;k<keywords.size();k++)
		{
			if(strcmp(keywords[k].c_str(),psq.m_query_keywords.v_exact_keywords[j].keyword.c_str())==0)
			{
				weight+=psq.m_query_keywords.v_exact_keywords[j].weight;
			}
		}
	}

	// If the weight is >= 100, then this is a query match for the current project.  Check with the kill words, just in case
	if(weight>=100)
	{
		for(j=0;j<keywords.size();j++)
		{
			// See if any of the project killwords are within the query's keywords
			bool found=false;
			for(k=0;k<psq.m_query_keywords.v_killwords.size();k++)
			{
				if(strstr(keywords[j].c_str(),psq.m_query_keywords.v_killwords[k].keyword.c_str())!=0)
				{
					found=true;
					weight=0;
					break;
				}
			}

			// If it didn't match a project killword, see if any of the project exact killwords are equal to the query's keywords
			if(!found)
			{
				for(k=0;k<psq.m_query_keywords.v_exact_killwords.size();k++)
				{
					if(strcmp(keywords[j].c_str(),psq.m_query_keywords.v_exact_killwords[k].keyword.c_str())==0)
					{
						weight=0;
						break;
					}
				}
			}

			// If the query contained a killword, the weight has been reset to 0...so move on to the next project
			if(weight==0)
			{
				break;
			}
		}
	}
	return weight;
}

//
//
//
void OvernetUDPSocket::ExtractKeywordsFromSupply(vector<string> *keywords,const char *query)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(query)+1];
	strcpy(lbuf,query);
	strlwr(lbuf);

	// Remove all non alpha-numeric characters
	ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	string cstring_string=cstring;
	strcpy(lbuf,cstring_string.c_str());

	// Extract the keywords
	char *ptr=lbuf;
	while(strlen(ptr)>0)
	{
		// Skip past any intermediate spaces in between keywords
		while((*ptr==' '))
		{
			ptr++;
		}

		bool done=true;
		if(strchr(ptr,' ')!=NULL)	// see if there are more keywords after this keyword
		{
			done=false;
			*strchr(ptr,' ')='\0';
		}

		// Check to see if this keyword is already in there
		bool found=false;
		for(i=0;i<keywords->size();i++)
		{
			if(strcmp((*keywords)[i].c_str(),ptr)==0)
			{
				found=true;
				break;
			}
		}
		if(!found)	// if not, then add it
		{
			keywords->push_back(ptr);
		}

		if(done)
		{
			break;
		}
		else
		{
			ptr+=strlen(ptr)+1;
		}	
	}
	
	delete [] lbuf;
}

//
// Replaces all characters that are not letters or numbers with spaces in place
//
void OvernetUDPSocket::ClearNonAlphaNumeric(char *buf,int buf_len)
{
	int i;
	for(i=0;i<buf_len;i++)
	{
		if(((buf[i]>='0') && (buf[i]<='9')) || ((buf[i]>='A') && (buf[i]<='Z')) || ((buf[i]>='a') && (buf[i]<='z')))
		{
			// It passes
		}
		else
		{
			buf[i]=' ';
		}
	}
}

// Return Values (Track)
// ---------------------
// -1 : query hit is not a match
//  0 : query hit is a match but there are no required keywords, so it does not correspond with any track number
// >0 : query hit is a match and is associated with the specified track number
//
int OvernetUDPSocket::CheckQueryHitRequiredKeywords(ProjectSupplyQuery& psq, const char *lfilename)
{
	UINT i,j;

	// Check the kill words
	bool killed=false;
	for(i=0;i<psq.v_killwords.size();i++)
	{
		if(strstr(lfilename,psq.v_killwords[i])!=NULL)
		{
			return -1;
		}
	}
	
	// Check the required keywords, to determine the track.
	if(psq.v_keywords.size()==0)
	{
		return 0;
	}

	// Check all of the required keywords tracks
	for(i=0;i<psq.v_keywords.size();i++)
	{
		// Check the required keywords for this track
		bool found=true;
		for(j=0;j<psq.v_keywords[i].v_keywords.size();j++)
		{
			if(strstr(lfilename,psq.v_keywords[i].v_keywords[j])==NULL)
			{
				found=false;
				break;
			}
		}

		// If we found a matching track
		if(found)
		{
			return psq.v_keywords[i].m_track;
		}
	}
	
	// None of the required keywords matched
	return -1;
}

/*
//
//
//
UINT OvernetUDPSocket::ReturnNumberOfHashes(UINT& hashes_not_published, UINT& total_hashes,UINT& search_index)
{
	UINT hash_count=0;
	hash_set<SearchSet>::const_iterator iter = hs_hashes.begin();
	while(iter!=hs_hashes.end())
	{
		if(iter->hs_peer_hashes.size()==0)
			hashes_not_published++;
		hash_count += iter->v_server_IPs.size();
		hash_count += iter->hs_meta_data.size();
		iter++;
	}
	total_hashes = hs_hashes.size();
	search_index = m_hash_search_index;
	return hash_count;
}
*/
//
//
//
void OvernetUDPSocket::OneSecondTick()
{
	m_seconds++;
	if(m_seconds%14400==0)//4 hours, reset peers' IPs
	{
		for(int i=0; i<(int)v_group_hashes.size();i++)
		{
			v_group_hashes[i].hs_peers.clear();
		}
	}
	p_connection_module->m_status_data.m_num_peers = ReturnNumberOfPeers();
	if(m_seconds%5==0) //farming peer IPs
	{
		if(ReturnNumberOfPeers()>100)
		{
			UINT index = (UINT)rand()%v_group_hashes.size();
			hash_set<OvernetPeer>::const_iterator iter;
			int count=0;
			while(count<50 && count<= v_group_hashes[index].hs_peers.size())
			{
				iter = v_group_hashes[index].hs_peers.begin();
				int random = rand()%(v_group_hashes[index].hs_peers.size());
				for(int i=0;i<random;i++)
					iter++;
				SendConnect(iter->m_ip,iter->m_port);
				count++;
			}
		}
		else
		{
			SendConnect("127.0.0.1",3739);
			SendConnect("127.0.0.1",3739);
		}
	}
/*
#ifdef KEYWORD_HASH_SPOOFER
	if(	((CTime::GetCurrentTime()-m_last_round_start_time).GetTotalSeconds() > 7200))
#else
	if(	((CTime::GetCurrentTime()-m_last_round_start_time).GetTotalSeconds() > 1800))
#endif
	{
		m_hash_manager.ReadInHashesFromDB();
		ReadInHashes();		
		RePublicizeAndRePublish();
		m_round++;
		m_last_round_start_time=CTime::GetCurrentTime();
	}
#if _DEBUG
	if(ReturnNumberOfPeers()>100000)
#else
	if(ReturnNumberOfPeers()>200000)
#endif
	{
		hash_set<SearchSet>::iterator iter = hs_hashes.begin();
		if(m_hash_search_index > hs_hashes.size()-1)
		{
			m_publish_file_hashes_completed=true;
			m_hash_search_index=0;
		}

		for(UINT i=0;i<m_hash_search_index;i++)
			iter++;
		int index_increment_count=0;
		while(iter!=hs_hashes.end() && index_increment_count<m_publish_rate)
		{
			//if(iter->hs_peer_hashes.size()==0)
			//{
				index_increment_count++;
				hash_set<OvernetPeer>::iterator iter_peer = v_group_hashes[iter->m_hash[0]].hs_peers.begin();
				UINT search_count=0;
				UINT publish_count=0;
				while(iter_peer != v_group_hashes[iter->m_hash[0]].hs_peers.end() )
				{
					//if(iter->IsHashCloseEnough(iter_peer->m_id[0]))
					publish_count=1;
					if(iter_peer->m_id[1]==iter->m_hash[1] )//|| ((iter->hs_meta_data.size() > 0) && (abs((int)iter_peer->m_id[1]-(int)iter->m_hash[1])<=5)))
					{
						SendSearch(iter->m_hash,iter_peer->m_ip,iter_peer->m_port,0x04);
						search_count++;

						if(iter->v_server_IPs.size() > 0)
						{
							//if(!m_publish_file_hashes_completed)
							//{
								for(UINT i=0;i<iter->v_server_IPs.size();i++)
									SendPublishFileHash(iter_peer->m_ip,iter_peer->m_port,iter->m_hash,iter->v_server_IPs[i].m_ip,iter->v_server_IPs[i].m_port,iter->v_server_IPs[i].m_user_hash);
								publish_count++;
							//}
							//else
							//{
							//	publish_count=1;
							//}
						}
						if(iter->hs_meta_data.size() > 0)
						{
							hash_set<MetaData>::const_iterator meta_iter = iter->hs_meta_data.begin();
							pair< hash_set<PublishQueue>::iterator, bool > pr;
							PublishQueue queue;
							queue.m_ip=iter_peer->m_ip;
							queue.m_port=iter_peer->m_port;
							pr = hs_publish_queues.insert(queue);
							while( (meta_iter != iter->hs_meta_data.end()) &&
								(pr.second || !m_publish_file_hashes_completed) )
							{
								//SendPublishKeywordHash(iter_peer->m_ip,iter_peer->m_port,iter->m_hash,*(meta_iter));
								pr.first->v_to_be_published.push_back(*(meta_iter));
								meta_iter++;
							}
							if(pr.second)
							{
								MetaData data = pr.first->ReturnCurrentMetaData();
								SendPublishKeywordHash(pr.first->m_ip,pr.first->m_port,data);
							}

							publish_count++;
					//		iter->hs_peer_hashes.insert(*(iter_peer));
						}
						iter->hs_peer_hashes.insert(*(iter_peer));
					}
					iter_peer++;
				}
				if(search_count==0) //didn't send a search to peer with the same first two byte hash
				{
					iter_peer = v_group_hashes[iter->m_hash[0]].hs_peers.begin();
					while(iter_peer != v_group_hashes[iter->m_hash[0]].hs_peers.end() )
					{
						SendSearch(iter->m_hash,iter_peer->m_ip,iter_peer->m_port,0x04); //send search to peer with the same first byte hash
						search_count++;
						iter_peer++;
					}
				}
				if(publish_count==0) // no peers with the same first byte hash to publish
				{
					int index1 = iter->m_hash[0] - 1;
					if(index1 < 0)
						index1 = 0;
					int index2 = iter->m_hash[0] + 1;
					if(index2 > 0xFF)
						index2 = 0xFF;

					iter_peer =  v_group_hashes[index1].hs_peers.begin();
					while(iter_peer != v_group_hashes[index1].hs_peers.end())
					{
						SendSearch(iter->m_hash,iter_peer->m_ip,iter_peer->m_port,0x04);
						search_count++;
						iter_peer++;
					}
					iter_peer =  v_group_hashes[index2].hs_peers.begin();
					while(iter_peer != v_group_hashes[index2].hs_peers.end())
					{
						SendSearch(iter->m_hash,iter_peer->m_ip,iter_peer->m_port,0x04);
						search_count++;
						iter_peer++;
					}
				}
				if(search_count==0)
				{
					TRACE("No peers are similar to this hash, fuck it!!\n");
				}

			//}
			iter++;
			m_hash_search_index++;
		}
	}

	//if(m_seconds%60==0) //checking publish queue status
	//{
		hash_set<PublishQueue>::iterator iter=hs_publish_queues.begin();
		while(iter!=hs_publish_queues.end())
		{
			for(int i=0;i<10;i++)
			{
				if( (iter->v_to_be_published.size() > 0) )
					//&& ((CTime::GetCurrentTime()-iter->m_last_publish_ack_received).GetTotalSeconds() > 5) )
				{
					MetaData data = iter->PublishAckReceived();
					SendPublishKeywordHash(iter->m_ip,iter->m_port,data);
				}
			}
			iter++;
		}
	//}
	*/
}
/*
//
//
//
void OvernetUDPSocket::SendPublicizeAck(UINT ip, unsigned short int port)
{
	unsigned char buf[2];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x0d;

	UINT num_sent=0;
	int ret = SendTo(buf,sizeof(buf),ip,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
	m_publicize_received++;
}

//
//
//
void OvernetUDPSocket::SendPublicizeMyself(UINT ip, unsigned short int port)
{
	if(m_my_ip != 0)
	{
		unsigned char buf[25];
		*(byte*)&buf[0]=0xe3;
		*(byte*)&buf[1]=0x0c;
		memcpy(&buf[2],m_my_id,16);
		*(UINT*)&buf[18]=m_my_ip;
		*(unsigned short int*)&buf[22]=m_my_udp_port;
		*(byte*)&buf[24]=0x00;

		UINT num_sent=0;
		int ret = SendTo(buf,sizeof(buf),ip,port,&num_sent);
		int error=0;
		if(ret==-1)
		{
			error=GetLastError();
		}
		m_publicize_sent++;
	}
	else
		TRACE("NO LOCAL IP FOUND\n");
}

//
//
//
void OvernetUDPSocket::SendPublishFileHash(UINT ip, unsigned short int port, byte* hash, const char* src_ip, unsigned short src_port, byte* user_hash)
{

	CString location;
	location.Format("bcp://%s:%d",src_ip,src_port);
	unsigned short int loc_len = location.GetLength();

	int buf_len=46+loc_len;
	unsigned char* buf = new unsigned char[buf_len];
	*(byte*)&buf[0]=0xe3;	//edonkey
	*(byte*)&buf[1]=0x13;	//publish
	memcpy(&buf[2],hash,16);	//file hash
	memcpy(&buf[18],user_hash,16); //my user hash
	*(UINT*)&buf[34]=1;			//meta tag list size
	*(byte*)&buf[38]=0x02;		//meta tag type
	*(unsigned short int*)&buf[39]=3;		//meta tag name size
	memcpy(&buf[41], "loc",3); //location
	*(unsigned short int*)&buf[44]=loc_len;
	memcpy(&buf[46],(const char*)location,loc_len);

	IPSpoof(inet_addr(src_ip),m_my_udp_port,ip,port,buf,buf_len);
	m_files_published++;
	delete [] buf;
}

//
//
//
void OvernetUDPSocket::IPSpoof(unsigned int src_ip,unsigned short int src_port,unsigned int dest_ip,unsigned short int dest_port,
							 byte* buf, UINT buf_len)
{
	//
	// Form the IP and UDP headers
	//

	// Create a new RAW socket that we will ip spoof with
	SOCKET hSocket=WSASocket(AF_INET,SOCK_RAW,0,NULL,0,0);	

	// Change the socket option to include the IP header in the send data buffer
	BOOL opt=TRUE;
	setsockopt(hSocket,IPPROTO_IP,IP_HDRINCL,(char *)&opt,sizeof(opt));	

	// IP Header
	unsigned char ip[20];
	memset(ip,0,sizeof(ip));
	// ip version
	ip[0]|=0x40;
	// length of header (in 32-bit words)
	ip[0]|=0x05;
	// total length in bytes of data and header (will be set automatically)
	ip[2]=0x00;
	ip[3]=0x00;
	// ID (continually incrementing counter to ID this packet in relation to other packets before and after it)
	ip[4]=rand()%256;
	ip[5]=rand()%256;
	// Fragment offset
	ip[6]=0x00;
	ip[7]=0x00;
	// TTL (hops/seconds ?)
	ip[8]=0x80;
	// Protocol
	ip[9]=0x11;	// UDP
	// Header checksum (will be set automatically)
	ip[10]=0x00;
	ip[11]=0x00;
	// Source address ip1.ip2.ip3.ip4
	ip[12]=(src_ip>>0)&0xFF;	// ip1
	ip[13]=(src_ip>>8)&0xFF;	// ip2
	ip[14]=(src_ip>>16)&0xFF;	// ip3
	ip[15]=(src_ip>>24)&0xFF;	// ip4
	// Dest address	ip1.ip2.ip3.ip4
	ip[16]=(dest_ip>>0)&0xFF;	// ip1
	ip[17]=(dest_ip>>8)&0xFF;	// ip1
	ip[18]=(dest_ip>>16)&0xFF;	// ip1
	ip[19]=(dest_ip>>24)&0xFF;	// ip1

	// UDP Header
	unsigned char udp[8];
	memset(udp,0,sizeof(udp));
	// Source port : 
	udp[0]=(src_port>>8)&0xFF;
	udp[1]=(src_port>>0)&0xFF;
	// Dest port : 
	udp[2]=(dest_port>>8)&0xFF;
	udp[3]=(dest_port>>0)&0xFF;
	// Length of UDP header and data (8+buf_len)
	udp[4]=((8+buf_len)>>8)&0xFF;
	udp[5]=((8+buf_len)>>0)&0xFF;
	// Checksum (will be calculated below)
	udp[6]=0x00;
	udp[7]=0x00;

	// Calculate the checksum

	// IP pseudo-header
	char pseudo[12];
	memset(pseudo,0,sizeof(pseudo));
	// - Source IP
	memcpy(&pseudo[0],&ip[12],4);
	// - Dest IP
	memcpy(&pseudo[4],&ip[16],4);
	// - Protocol
	pseudo[9]=ip[9];
	// - UDP Length
	memcpy(&pseudo[10],&udp[4],2);

	// Perform the checksum calculations
	unsigned int i;
	unsigned int sum=0;
	for(i=0;i<sizeof(pseudo);i+=2)
	{
		sum+=ntohs(*((unsigned short int *)&pseudo[i]));

		// If it is overflowing, bring it back
		if((sum & 0xFFFF0000) != 0)
		{
			sum=(sum&0xFFFF)+(sum>>16);
		}
	}
	for(i=0;i<sizeof(udp);i+=2)
	{
		sum+=ntohs(*((unsigned short int *)&udp[i]));

		// If it is overflowing, bring it back
		if((sum & 0xFFFF0000) != 0)
		{
			sum=(sum&0xFFFF)+(sum>>16);
		}
	}

	// Zero-pad the UDP data to make the size even (if necessary)
	unsigned int data_len=buf_len;
	if(data_len%2==1)
	{
		data_len++;
	}
	unsigned char *data=new unsigned char[data_len];
	memset(data,0,data_len);
	memcpy(data,buf,buf_len);
	for(i=0;i<data_len;i+=2)
	{
		sum+=ntohs(*((unsigned short int *)&data[i]));

		// If it is overflowing, bring it back
		if((sum & 0xFFFF0000) != 0)
		{
			sum=(sum&0xFFFF)+(sum>>16);
		}
	}

	// Finalize the checksum
	unsigned short int checksum=(unsigned short int)sum;
	checksum^=0xFFFF;

	// Copy the checksum to the UDP header
	udp[6]=(checksum>>8)&0xFF;
	udp[7]=checksum&0xFF;

	//
	// Create the datagram packet to send
	//
	unsigned int pkt_len=sizeof(ip)+sizeof(udp)+data_len;
	char *pkt=new char[pkt_len];
	memcpy(pkt,ip,sizeof(ip));
	memcpy(&pkt[sizeof(ip)],udp,sizeof(udp));
	memcpy(&pkt[sizeof(ip)+sizeof(udp)],data,data_len);

	WSABUF wsabuf;
	wsabuf.buf=pkt;
	wsabuf.len=pkt_len;

	DWORD sent=0;

	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=dest_ip;
	addr.sin_port=htons(dest_port);

	int ret=WSASendTo(hSocket,&wsabuf,1,&sent,0,(sockaddr *)&addr,sizeof(sockaddr_in),NULL,NULL);
	if(ret!=0)
	{
		int error=WSAGetLastError();
	}

	closesocket(hSocket);

	//free memory
	delete [] pkt;
	delete [] data;
	//delete [] buf;

	
}

//
//
//
void OvernetUDPSocket::SendPublishKeywordHash(UINT ip, unsigned short int port, const MetaData& meta)
{
	if(meta.m_filename.GetLength()==0)
		return;

	PacketBuffer buf;
	buf.PutByte(0xe3);	//edonkey
	buf.PutByte(0x13);	//publish
	buf.PutUStr((const unsigned char*)meta.m_keyword_hash,16);	//keyword hash
	buf.PutUStr((const unsigned char*)meta.m_file_hash,16); //file hash

	int tag_count=0;
    PacketBuffer next_buf;
	//filename
	next_buf.PutByte(0x02);
	next_buf.PutUShort(1);
	next_buf.PutByte(0x01);
	next_buf.PutUShort(meta.m_filename.GetLength());
	next_buf.PutStr(meta.m_filename,meta.m_filename.GetLength());
	tag_count++;

	//filesize
	next_buf.PutByte(0x03);
	next_buf.PutUShort(1);
	next_buf.PutByte(0x02);
	next_buf.PutUInt(meta.m_filesize);
	tag_count++;

	//file type audio
	if(meta.m_file_type.Compare("Audio")==0)
	{
		next_buf.PutByte(0x02);
		next_buf.PutUShort(1);
		next_buf.PutByte(0x03);
		next_buf.PutUShort(meta.m_file_type.GetLength()); // "Audio" length
		next_buf.PutStr(meta.m_file_type,meta.m_file_type.GetLength());
		tag_count++;

		//Artist
		if(meta.m_artist.GetLength()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(6);
			next_buf.PutStr("Artist",6);
			next_buf.PutUShort(meta.m_artist.GetLength());
			next_buf.PutStr(meta.m_artist,meta.m_artist.GetLength());
			tag_count++;
		}
		//Album
		if(meta.m_album.GetLength()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(5);
			next_buf.PutStr("Album",5);
			next_buf.PutUShort(meta.m_album.GetLength());
			next_buf.PutStr(meta.m_album,meta.m_album.GetLength());
			tag_count++;
		}

		//Title
		if(meta.m_title.GetLength()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(5);
			next_buf.PutStr("Title",5);
			next_buf.PutUShort(meta.m_title.GetLength());
			next_buf.PutStr(meta.m_title,meta.m_title.GetLength());
			tag_count++;
		}

		//bitrate
		if(meta.m_bitrate!=0)
		{
			next_buf.PutByte(0x03);
			next_buf.PutUShort(7);
			next_buf.PutStr("bitrate",7);
			next_buf.PutUInt(meta.m_bitrate);
			tag_count++;
		}
	}
	else if(meta.m_file_type.Compare("Video")==0)
	{
		next_buf.PutByte(0x02);
		next_buf.PutUShort(1);
		next_buf.PutByte(0x03);
		next_buf.PutUShort(meta.m_file_type.GetLength()); // "Video" length
		next_buf.PutStr(meta.m_file_type,meta.m_file_type.GetLength());
		tag_count++;
		
		//Codec
		if(meta.m_codec.GetLength()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(5);
			next_buf.PutStr("codec",5);
			next_buf.PutUShort(meta.m_codec.GetLength());
			next_buf.PutStr(meta.m_codec,meta.m_codec.GetLength());
			tag_count++;
		}
	}

	//file format
	if(meta.m_file_format.GetLength()>0)
	{
		next_buf.PutByte(0x02);
		next_buf.PutUShort(1);
		next_buf.PutByte(0x04);
		next_buf.PutUShort(meta.m_file_format.GetLength());
		next_buf.PutStr(meta.m_file_format,meta.m_file_format.GetLength());
		tag_count++;
	}


	//Length
	if(meta.m_length.GetLength()>0)
	{
		next_buf.PutByte(0x02);
		next_buf.PutUShort(6);
		next_buf.PutStr("length",6);
		next_buf.PutUShort(meta.m_length.GetLength());
		next_buf.PutStr(meta.m_length,meta.m_length.GetLength());
		tag_count++;
	}

	//Avaiability
	next_buf.PutByte(0x03);
	next_buf.PutUShort(1);
	next_buf.PutByte(0x15);
	next_buf.PutUInt(meta.m_availability);
	tag_count++;

	buf.PutUInt(tag_count);	//meta tag list size
	buf.Append(next_buf);
	UINT num_sent=0;
	int ret = SendTo(buf.p_data,buf.Size(),ip,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
	m_keywords_published++;
}

//
//
//
void OvernetUDPSocket::SendIPQuery(UINT ip, unsigned short int port)
{
	unsigned char buf[4];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x1b;
	*(unsigned short int*)&buf[2]=m_my_udp_port;

	UINT num_sent=0;
	int ret = SendTo(buf,sizeof(buf),ip,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
}

//
//
//
void OvernetUDPSocket::RePublicizeAndRePublish()
{
	m_publish_file_hashes_completed=false;
	for(int i=0;i<sizeof(m_my_id);i++) //randomly create my user id hash
		m_my_id[i]=rand()%256;
	TRACE("My user ID hash:%s\n",m_df.EncodeBase16(m_my_id,16));

	m_hash_search_index=0;
	m_publicize_sent=m_publicize_received=m_publicize_acks_received=m_files_published=m_keywords_published=0;

	for(int i=0; i<(int)v_group_hashes.size();i++)
	{
		v_group_hashes[i].hs_peers.clear();
	}
	hs_publish_queues.clear();
	hash_set<SearchSet>::iterator iter = hs_hashes.begin();
	while(iter!=hs_hashes.end())
	{
		iter->hs_peer_hashes.clear();
		iter++;
	}
	SendConnect("127.0.0.1",3739);
	SendConnect("127.0.0.1",3739);
}


//
//
//
void OvernetUDPSocket::ReadInProjectKeywords()
{
	char *folder="Keyword Files\\";
	string path;
	
	v_all_project_keywords.clear();

	WIN32_FIND_DATA file_data;
	path=folder;
	path+="*.kwd";
	HANDLE search_handle = ::FindFirstFile(path.c_str(), &file_data);
	BOOL found = FALSE;

	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}

	while(found == TRUE)
	{
		CFile keyword_data_file;
		path=folder;
		path+=file_data.cFileName;
		BOOL open = keyword_data_file.Open(path.c_str(),CFile::typeBinary|CFile::modeRead|CFile::shareDenyNone);

		if(open==TRUE)
		{
			unsigned char * keyword_data =  new unsigned char[(UINT)keyword_data_file.GetLength()];

			keyword_data_file.Read(keyword_data, (UINT)keyword_data_file.GetLength());

			ProjectKeywords new_keywords;
			new_keywords.ReadFromBuffer((char*)keyword_data);

			v_all_project_keywords.push_back(new_keywords);

			keyword_data_file.Close();

			delete [] keyword_data;
		}

		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);
	sort(v_all_project_keywords.begin(),v_all_project_keywords.end());
}

//
//
//
void OvernetUDPSocket::ReadInHashes()
{
	ReadInProjectKeywords();
	hs_hashes.clear();
	m_hash_search_index=0;

	UINT i,j,k,l;

	
	for(i=0;i<v_all_project_keywords.size();i++)
	{
		if(v_all_project_keywords[i].m_project_active)
		{
			if(v_all_project_keywords[i].m_search_type == ProjectKeywords::search_type::audio)
			{
				for(j=0;j<v_all_project_keywords[i].m_supply_keywords.v_keywords.size();j++) //each track
				{
					for(k=0;k<NUM_DECOYS_PER_TRACK;k++)
					{
						SearchSet set;
						byte hash[16];
						char ip[32];
						UINT filesize=0;
						vector<IPAndUserHash>IPs;
						if(m_hash_manager.GetHashAndIP(hash,IPs,filesize)==false) // no hashes to use
							return;
#ifdef FILE_HASH_SPOOFER
						set.SetHash(hash);
						pair< hash_set<SearchSet>::iterator, bool > pr;
						pr=hs_hashes.insert(set);
						if(pr.second)
							pr.first->v_server_IPs=IPs;
#endif					
#ifdef KEYWORD_HASH_SPOOFER					
						MetaData md;
						md.SetFileHash(hash);
						md.m_album=v_all_project_keywords[i].m_album_name.c_str();
						md.m_artist=v_all_project_keywords[i].m_artist_name.c_str();
						md.m_availability=(rand()%156)+100;
						if( (rand()%2)==0)
							md.m_bitrate=128;
						else
							md.m_bitrate=192;
						md.m_file_format="mp3";
						md.m_file_type="Audio";
						md.m_filesize=filesize;
						md.m_title=v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_track_name.c_str();
						md.m_length.Format("%.2d:%.2d",(rand()%5)+3,rand()%60);

						int pattern = rand()%10;

						switch(pattern)
						{
						case 0:
						case 1:
						case 2:
							{
								md.m_filename.Format("%s - %s.mp3",md.m_artist,md.m_title);
								break;
							}
						case 3:
							{
								md.m_filename.Format("%s - %s.mp3",md.m_title,md.m_artist);
								break;
							}
						case 4:
							{
								md.m_filename.Format("%.2d - %s - %s.mp3",v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_track,
									md.m_artist,md.m_title);
								break;
							}
						case 5:
							{
								md.m_filename.Format("%s - %.2d %s.mp3",md.m_artist,v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_track,
									md.m_title);
								break;
							}
						case 6:
							{
								md.m_filename.Format("%s - %s %d %s.mp3",md.m_artist,md.m_album,v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_track,
									md.m_title);
								break;
							}
						case 7:
							{
								md.m_filename.Format("%s - %s %.2d %s.mp3",md.m_artist,md.m_album,v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_track,
									md.m_title);
								break;
							}
						case 8:
							{
								md.m_filename.Format("%s - %s %s %.2d.mp3",md.m_artist,md.m_album,md.m_title,
									v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_track);
								break;
							}
						case 9:
							{
								md.m_filename.Format("%d - %s - %s.mp3",v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_track,
									md.m_artist,md.m_title);
								break;
							}
						}


						if(rand()%3 == 0)
							md.m_filename.MakeLower();					

						vector<CString>artist_keywords;
						GetArtistKeywords(md.m_artist,&artist_keywords);

						for(l=0;l<artist_keywords.size();l++)
						{
							char* keyword = new char[artist_keywords[l].GetLength()+1];
							strcpy(keyword,artist_keywords[l]);
							m_df.CreateHashFromInput(NULL,NULL,artist_keywords[l].GetLength(),hash,(byte*)keyword);
							SearchSet keyword_set;
							keyword_set.SetHash(hash);
							pair< hash_set<SearchSet>::iterator, bool > pr1;
							pr1 = hs_hashes.insert(keyword_set);
							md.m_keyword=artist_keywords[l];
							md.SetKeywordHash(hash);
							pr1.first->hs_meta_data.insert(md);
							if(pr1.first->v_server_IPs.size() > 0)
								TRACE("FUCKING WEIRD\n");
							delete [] keyword;
						}
						for(l=0;l<v_all_project_keywords[i].m_supply_keywords.v_keywords[j].v_keywords.size();l++)
						{
							CString keyword = v_all_project_keywords[i].m_supply_keywords.v_keywords[j].v_keywords[l];
							keyword.Trim();
							keyword.MakeLower();
							char* keyword2 = new char[keyword.GetLength()+1];
							strcpy(keyword2,keyword);

							m_df.CreateHashFromInput(NULL,NULL,keyword.GetLength(),hash,(byte*)keyword2);
							SearchSet keyword_set;
							keyword_set.SetHash(hash);
							md.SetKeywordHash(hash);
							pair< hash_set<SearchSet>::iterator, bool > pr1;
							pr1=hs_hashes.insert(keyword_set);
							md.m_keyword=keyword2;
							pr1.first->hs_meta_data.insert(md);
							if(pr1.first->v_server_IPs.size() > 0)
								TRACE("FUCKING WEIRD\n");
							delete [] keyword2;
						}
#endif
					}
				}
			}
			else if(v_all_project_keywords[i].m_search_type == ProjectKeywords::search_type::video)
			{
				for(k=0;k<NUM_DECOYS_PER_MOVIE;k++)
				{
					SearchSet set;
					byte hash[16];
					char ip[32];
					UINT filesize=0;
					vector<IPAndUserHash>IPs;
					if(m_hash_manager.GetMovieHashAndIP(hash,IPs,filesize)==false) // no hashes to use
						return;
#ifdef FILE_HASH_SPOOFER
					set.SetHash(hash);
					pair< hash_set<SearchSet>::iterator, bool > pr;
					pr=hs_hashes.insert(set);
					if(pr.second)
						pr.first->v_server_IPs=IPs;
#endif					
#ifdef KEYWORD_HASH_SPOOFER					
					MetaData md;
					md.SetFileHash(hash);
					md.m_availability=(rand()%156)+100;
					md.m_filesize=filesize;
					md.m_artist=v_all_project_keywords[i].m_artist_name.c_str();
					//md.m_artist="qweasdzxc";
					md.m_file_type="Video";
					md.m_availability=(rand()%11)+245;
					md.m_length.Format("1h %dm %ds",(rand()%30)+30,rand()%60);
					int pattern = rand()%3;
					switch(pattern)
					{
					case 0:
						{
							md.m_filename.Format("%s.mpg",md.m_artist);
							md.m_file_format="mpg";
							pattern=rand()%3;
							switch(pattern)
							{
							case 0:
								{
									md.m_codec="y41p";
									break;
								}
							}
							break;
						}
					case 1:
						{
							md.m_filename.Format("%s.mpeg",md.m_artist);
							md.m_file_format="mpeg";
							pattern=rand()%3;
							switch(pattern)
							{
							case 0:
								{
									md.m_codec="y41p";
									break;
								}
							}
							break;
						}
					case 2:
						{
							md.m_file_format="avi";
							pattern=rand()%7;
							switch(pattern)
							{
							case 0:
								{
									md.m_codec="divx";
									break;
								}
							case 1:
								{
									md.m_codec="div3";
									break;
								}
							case 2:
								{
									md.m_codec="xvid";
									break;
								}
							case 3:
								{
									md.m_codec="mp43";
									break;
								}
							case 4:
								{
									md.m_codec="mp43";
									break;
								}
							case 5:
								{
									md.m_codec="div4";
									break;
								}
							case 6:
								{
									md.m_codec="dx50";
									break;
								}
							}
							if((rand()%3 == 0) && (md.m_codec.Compare("divx")==0 || md.m_codec.Compare("xvid")==0) )
							{
								md.m_filename.Format("%s %s.avi",md.m_artist,md.m_codec);
							}
							else
							{
								md.m_filename.Format("%s.avi",md.m_artist);
							}
							break;
						}
					}
					if(rand()%5 == 0 && md.m_codec.GetLength()>0)
						md.m_codec.MakeUpper();
					if(rand()%3 == 0)
						md.m_filename.MakeLower();	
					vector<CString>artist_keywords;
					GetArtistKeywords(md.m_artist,&artist_keywords);
					for(l=0;l<artist_keywords.size();l++)
					{
						char* keyword = new char[artist_keywords[l].GetLength()+1];
						strcpy(keyword,artist_keywords[l]);
						m_df.CreateHashFromInput(NULL,NULL,artist_keywords[l].GetLength(),hash,(byte*)keyword);
						SearchSet keyword_set;
						keyword_set.SetHash(hash);
						pair< hash_set<SearchSet>::iterator, bool > pr1;
						pr1 = hs_hashes.insert(keyword_set);
						md.m_keyword=artist_keywords[l];
						md.SetKeywordHash(hash);
						pr1.first->hs_meta_data.insert(md);
						if(pr1.first->v_server_IPs.size() > 0)
							TRACE("FUCKING WEIRD\n");
						delete [] keyword;
					}
#endif
				}
			}
		}
	}
}

//
//
//
void OvernetUDPSocket::GetArtistKeywords(const char* artist, vector<CString>* artist_keywords)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(artist)+1];
	strcpy(lbuf,artist);
	strlwr(lbuf);

	// Remove all non alpha-numeric characters
	ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	string cstring_string=cstring;
	strcpy(lbuf,cstring_string.c_str());

	// Extract the keywords
	char *ptr=lbuf;
	while(strlen(ptr)>0)
	{
		// Skip past any intermediate spaces in between keywords
		while((*ptr==' '))
		{
			ptr++;
		}

		bool done=true;
		if(strchr(ptr,' ')!=NULL)	// see if there are more keywords after this keyword
		{
			done=false;
			*strchr(ptr,' ')='\0';
		}

		// Check to see if this keyword is already in there
		bool found=false;
		for(i=0;i<artist_keywords->size();i++)
		{
			if(strcmp((*artist_keywords)[i],ptr)==0)
			{
				found=true;
				break;
			}
		}
		if(!found)	// if not, then add it
		{
			artist_keywords->push_back(ptr);
		}

		if(done)
		{
			break;
		}
		else
		{
			ptr+=strlen(ptr)+1;
		}	
	}
	
	delete [] lbuf;
}

//
// Replaces all characters that are not letters or numbers with spaces in place
//
void OvernetUDPSocket::ClearNonAlphaNumeric(char *buf,int buf_len)
{
	int i;
	for(i=0;i<buf_len;i++)
	{
		if(((buf[i]>='0') && (buf[i]<='9')) || ((buf[i]>='A') && (buf[i]<='Z')) || ((buf[i]>='a') && (buf[i]<='z')) ||
			(buf[i]=='\'') )
			
		{
			// It passes
		}
		else
		{
			buf[i]=' ';
		}
	}
}

//
//
//
UINT OvernetUDPSocket::ReturnHashManagerNumberOfHashes()
{
	return m_hash_manager.GetNumHashes();
}

//
//
//
int OvernetUDPSocket::AlterPublishRate(int multi)
{
	m_publish_rate+=multi;

	// Limit the multi to fall within 1 and 1000
	if(m_publish_rate<0)
	{
		m_publish_rate=0;
	}
	else if(m_publish_rate>1000)
	{
		m_publish_rate=1000;
	}

	// Store in the registry
	RegistryKeys::SetPublishRate(m_publish_rate);


	return m_publish_rate;
}

//
//
//
UINT OvernetUDPSocket::ReturnNumberOfPublishQueues()
{
	return hs_publish_queues.size();
}

//
//
//
UINT OvernetUDPSocket::ReturnNumberOfRemainingKeywordHashesToBePublished()
{
	UINT hash_count=0;
	
	hash_set<PublishQueue>::const_iterator iter = hs_publish_queues.begin();
	while(iter!=hs_publish_queues.end())
	{
		hash_count += iter->v_to_be_published.size();
		iter++;
	}
	return hash_count;
}
*/

//
//
//
bool OvernetUDPSocket::IsOneOfUs(UINT ip)
{
	in_addr addr;
	addr.S_un.S_addr=ip;
	CString ip_addr = inet_ntoa(addr);
	int ip1,ip2,ip3,ip4;
	sscanf(ip_addr,"%d.%d.%d.%d",&ip1,&ip2,&ip3,&ip4);
	if(ip1==72 && ip2==35 && ip3==224)
		return true;
	return false;
}