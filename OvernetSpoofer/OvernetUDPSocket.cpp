#include "StdAfx.h"
#include "overnetudpsocket.h"
#include "iphlpapi.h"
#include "ConnectionModule.h"
#include "DonkeyMD4.h"
#define BOOTSTRAP	"216.151.155.87"

#define IP_HDRINCL 2	// for ip spoofing

OvernetUDPSocket::OvernetUDPSocket(void)
{
	SetNextResetTime();
	m_hash_search_index=0;
	ClearStatus();
	m_seconds=0;
	m_my_ip=0;
	m_my_tcp_port=4662;
//	GetMyIP();
#ifdef _DEBUG
	m_my_udp_port=4662;
#else
	m_my_udp_port=7871;
#endif

	for(int i=0;i<=0xFF;i++)
	{
		UserGroupHashes group(i);
		v_group_hashes.push_back(group);
	}

	int ret=0;
	m_overnet_dc_ip = inet_addr("206.161.141.53");
	m_overnet_dc2_ip = inet_addr("206.161.141.58");

	ret = WSocket::Startup();
	ret = Create(m_my_udp_port);
	m_number_overnet_users=0;
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
	CString log;
	log.Format("My IP: %s UDP Port: %d",m_my_ip_str,m_my_udp_port);
	p_connection_module->Log(log);
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
	//p_connection_module->Log("Reading generated hashes from DB started");
	//p_connection_module->m_hash_manager.ReadInHashesFromDB();
	//p_connection_module->Log("Reading generated hashes from DB ended");
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
void OvernetUDPSocket::UpdateProjectKeywords(vector<ProjectKeywords>& keywords)
{
	v_all_project_keywords = keywords;
	p_connection_module->ReadInSupplyEntries();
	//BuildSpoofs();
	BuildKeywordHashList();
}


void OvernetUDPSocket::OnReceive(int error_code)
{
	char buf[1024];
	memset(buf,0,sizeof(buf));

	unsigned int ip=0;
	unsigned short int port=0;
	unsigned int num_read=0;
	int ret=ReceiveFrom(buf,sizeof(buf),&ip,&port,&num_read);
	//int ret=ReceiveFrom(buf,2,&ip,&port,&num_read);
	if((byte)buf[0]!=0xe3) //check to see if it's overnet message
		return;
	//in_addr addr;
	//addr.S_un.S_addr=ip;
	//if(strstr(inet_ntoa(addr),"209.11.134")!=NULL || strcmp(inet_ntoa(addr),"206.161.141.53")==0)
	//	return;
	if(ip==m_overnet_dc_ip || ip==m_overnet_dc2_ip)
		return;


	byte* ptr = (byte*)&buf[1];
	byte op_code  = *ptr;
	switch(op_code)
	{
		case 0x0b: //Connect Reply
		{
			//ReceiveFrom(&buf[2],4094,&ip,&port,&num_read);
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
					m_number_overnet_users++;
					if(hs_my_user_hashes.size())
					{
						if(m_hash_search_index < hs_my_user_hashes.size())
						{
							hash_set<UserHash>::iterator iter = hs_my_user_hashes.begin();
							for(UINT j=0;j<=m_hash_search_index;j++)
								iter++;
							//SendSearch(iter->m_user_hash,this_ip,this_port,0x14); //searching peer hash similar to me
							SendConnect(this_ip,this_port,iter->m_user_hash);
							//m_hash_search_index++;
						}
					}
				}
			}
			break;
		}
		case 0x0c: //Publicize
		{
			//ReceiveFrom(&buf[2],4094,&ip,&port,&num_read);
			ptr++;
			OvernetPeer peer;
			peer.SetID(ptr,ip,port);
			pair< hash_set<OvernetPeer>::iterator, bool > pr;
			pr=v_group_hashes[peer.m_id[0]].hs_peers.insert(peer);
			v_group_hashes[peer.m_id[0]].hs_peers.insert(peer);
			
			if(pr.second)
			{
				m_number_overnet_users++;
				//vector<UserHash> hashes;
				//FindMyNearestUserHash(peer.m_id,hashes);
				//for(UINT i=0;i<hashes.size();i++)
				//{
				//	SendPublicizeMyself(ip,port,hashes[i].m_user_hash);
				//}
			}
			
			SendPublicizeAck(ip,port);
			break;
		}
		case 0x0d: //Publicize Ack
		{
			//ptr++;
			m_publicize_ack_received++;
			break;
		}
		case 0x0e: //Search
		{
			//ReceiveFrom(&buf[2],4094,&ip,&port,&num_read);
			ptr++;
			byte type = (*(byte*)ptr); ptr++;
			if( type==0x02)//query keyword searching
			{
				KeywordHash key_hash;
				key_hash.SetHash((byte*)ptr);
				hash_set<KeywordHash>::iterator keyword_iter = hs_keyword_hashes.find(key_hash);
				if(keyword_iter!=hs_keyword_hashes.end())
				{
//#ifdef _DEBUG
//					CString log;
//					log.Format("Searching hash received from %s - %s",inet_ntoa(addr),keyword_iter->m_keyword);
//					p_connection_module->Log(log);
//#endif
					SendSearchNext(ip,port,keyword_iter->m_hash,keyword_iter->m_my_user_hash);
				}
			}
			else if(type==0x14)//node searching
			{
				SendSearchNext(ip,port,ptr,v_group_hashes[ptr[0]].hs_peers);
			}
			break;
		}
		case 0x0f: //Search Next
		{
			//ReceiveFrom(&buf[2],4094,&ip,&port,&num_read);
			ptr++;
			UserHash user_hash;
			user_hash.SetHash(ptr);
			hash_set<UserHash>::iterator user_iter = hs_my_user_hashes.find(user_hash);
			if(user_iter!=hs_my_user_hashes.end())
			{
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
					if(pr.second)
					{
						m_number_overnet_users++;
						SendSearch(user_iter->m_user_hash,this_ip,this_port,0x14); //searching peer hash similar to me
						//if(*ptr == user_iter->m_user_hash[0]) //close enough?
						//{
							SendPublicizeMyself(this_ip,this_port,user_iter->m_user_hash);
						//}
					}
				}
			}
			break;
		}
		case 0x10: //query
		{
			//ReceiveFrom(&buf[2],4094,&ip,&port,&num_read);
			ptr++;
			m_queries_received++;
			byte hash[16];
			memcpy(hash,ptr,16);
			KeywordHash keyword_hash;
			keyword_hash.SetHash(hash);
			hash_set<KeywordHash>::iterator keyword_iter = hs_keyword_hashes.find(keyword_hash);
			if(keyword_iter==hs_keyword_hashes.end())
				break;
			CString search_string = keyword_iter->m_keyword;
			CString search_type;
			bool unknown_track=false;

			ptr+=16; //keyword hash
			if(*(byte*)ptr == 0x01)
			{
				ptr++; //constant byte 0x01
				unsigned short int num_keywords=1;
				while(*(unsigned short int*)ptr == 0)
				{
					num_keywords++;
					ptr+=2;
				}
				unsigned short int start_pos=0;
				for(int i=0;i<num_keywords;i++)
				{
					if(*(byte*)ptr == 0x01)
					{
						ptr++;//constant byte 0x01
						unsigned short int word_len=*(unsigned short int*)ptr;	ptr+=2;
						char* keyword= new char[word_len+1];
						memcpy(keyword,ptr,word_len);	ptr+=word_len;
						keyword[word_len]='\0';
						search_string+=" ";
						search_string+=keyword;
						delete [] keyword;
					}
					else if(*(byte*)ptr == 0x02) //search type
					{
						ptr++;
						unsigned short int word_len=*(unsigned short int*)ptr;	ptr+=2;
						char* type= new char[word_len+1];
						memcpy(type,ptr,word_len);	ptr+=word_len;
						type[word_len]='\0';
						search_type = type;
						delete [] type;
						ptr += 3; //constant 0x01, 0x00, 0x03
						start_pos=*(unsigned short int*)ptr;
					}
					else if(*(byte*)ptr == 0x04) //v1.2 - two search words only
					{
						unknown_track = true;
						break;
					}
				}
			}

			UINT project_id=0;
			bool audio_project=true;
			int track=0;
			vector<int> tracks;
			if(unknown_track)
				MatchingTracks(keyword_iter->vp_project_keywords,project_id,audio_project,tracks);
			else
				track=MatchingTrack(search_string, keyword_iter->vp_project_keywords,project_id,audio_project);
			
			if( (project_id!=0 && (track !=0 || !audio_project || tracks.size())) )
			{
				//spoof here
//#ifdef _DEBUG
//				CString log;
//				log.Format("Query received from %s, Query - %s, Search Type: %s, Start Pos: %d, Matching ProjectID: %u, Matching Track: %d",
//					inet_ntoa(addr),search_string,search_type,start_pos,project_id,track);
//				p_connection_module->Log(log);
//#endif
				hash_set<SupplyDataSet>::iterator supply_iter = p_connection_module->GetSupplyPointer(project_id);
				UINT spoof_sent=0;
				UINT intermediate_spoof_sent=0;
				CString log;
				if(supply_iter!=NULL)
				{
					if(!unknown_track)
					{
						if(track <= (int)(supply_iter->v_supply_data_by_track.size()-1))
						{
							if(supply_iter->v_supply_data_by_track[track].v_supply_data.size())
							{
								if(audio_project)
								{
									if(search_type == "audio" || search_type == "")
									{
										while(spoof_sent<100)
										{
											for(UINT i=0; i<100/*supply_iter->v_supply_data_by_track[track].v_supply_data.size()*/;i++)
											{
												if(i<supply_iter->v_supply_data_by_track[track].v_supply_data.size())
												{
													SendSpoofResult(ip,port,hash,
														supply_iter->v_supply_data_by_track[track].v_supply_data[i].p_overnet_supply_packet);
													spoof_sent++;
												}
												else
													break;
												if(!(spoof_sent%30))
													Sleep(5);
											}
										}
										m_spoofs_sent+=spoof_sent;
									}
//#ifdef _DEBUG
//									log.Format("%d spoofs sent to %s",supply_iter->v_supply_data_by_track[track].v_supply_data.size(),inet_ntoa(addr));
//#endif
								}
								else //movie or software project, track should be 0
								{
									if(track!=0)
									{
										log.Format("WARNING - NON ZERO TRACK: %d on Project ID %d, Search Type - %s",track, project_id, search_type);
										p_connection_module->Log(log);
										break;
									}
									if(search_type != "audio")
									{
										if(supply_iter->v_supply_data_by_track[0].v_supply_data.size())
										{
											while(spoof_sent<100)
											{
												for(UINT i=0; i<100/*supply_iter->v_supply_data_by_track[0].v_supply_data.size()*/;i++)
												{
													if(i<supply_iter->v_supply_data_by_track[0].v_supply_data.size())
													{
														SendSpoofResult(ip,port,hash,
															supply_iter->v_supply_data_by_track[0].v_supply_data[i].p_overnet_supply_packet);
														spoof_sent++;
													}
													else
														break;
													if(!(spoof_sent%30))
														Sleep(5);
												}
											}
											m_spoofs_sent+=spoof_sent;
										}
										else
										{
											log.Format("No supply to send, Project ID: %d, Track: %d",project_id,track);
											p_connection_module->Log(log);
										}
									}
//#ifdef _DEBUG
//									log.Format("%d spoofs sent to %s",supply_iter->v_supply_data_by_track[0].v_supply_data.size(),inet_ntoa(addr));
//#endif
								}
#ifdef _DEBUG
								p_connection_module->Log(log);
#endif
								int rand_sent= spoof_sent;
								int rand_total=rand()%1000 + rand_sent;
								SendSearchEnd(ip,port,hash,rand_sent,rand_total);
								//SendSearchEnd(ip,port,hash,rand_sent,rand_total);
								//SendSearchEnd(ip,port,hash,rand_sent,rand_total);
							}
						}
						else
						{
							log.Format("Track: %d not in supply vector, Project ID: %d",track,project_id);
							p_connection_module->Log(log);
						}
					}
					else //unknown track, v1.2+
					{
						for(UINT j=0;j<tracks.size();j++)
						{
							if(tracks[j] <= (int)(supply_iter->v_supply_data_by_track.size()-1))
							{
								if(supply_iter->v_supply_data_by_track[tracks[j]].v_supply_data.size())
								{
									if(audio_project)
									{
										if(search_type == "audio" || search_type == "")
										{
											while(spoof_sent<=100)
											{
												for(UINT i=0; i<100/*supply_iter->v_supply_data_by_track[tracks[j]].v_supply_data.size()*/;i++)
												{
													if(i<supply_iter->v_supply_data_by_track[tracks[j]].v_supply_data.size())
													{
														SendSpoofResult(ip,port,hash,
															supply_iter->v_supply_data_by_track[tracks[j]].v_supply_data[i].p_overnet_supply_packet);
														spoof_sent++;
													}
													else
														break;
													if(!(spoof_sent%30))
														Sleep(5);
													//if(spoof_sent>60)
													//	break;
												}
											}
//#ifdef _DEBUG
//											log.Format("%d spoofs sent to %s - Track: %d",spoof_sent,inet_ntoa(addr),tracks[j]);
//											p_connection_module->Log(log);
//#endif
										}
									}
									else //movie or software project, track should be 0
									{
										if(tracks[j]!=0)
										{
											log.Format("WARNING - NON ZERO TRACK: %d on Project ID %d, Search Type - %s",track, project_id, search_type);
											p_connection_module->Log(log);
											break;
										}
										if(search_type != "audio")
										{
											if(supply_iter->v_supply_data_by_track[0].v_supply_data.size())
											{
												while(spoof_sent<100)
												{
													for(UINT i=0; i<100/*supply_iter->v_supply_data_by_track[0].v_supply_data.size()*/;i++)
													{
														if(i<supply_iter->v_supply_data_by_track[0].v_supply_data.size())
														{
															SendSpoofResult(ip,port,hash,
																supply_iter->v_supply_data_by_track[0].v_supply_data[i].p_overnet_supply_packet);
															spoof_sent++;
														}
														else 
															break;
														if(!(spoof_sent%30))
															Sleep(5);
													}
												}
												//m_spoofs_sent+=spoof_sent;
											}
											else
											{
												log.Format("No supply to send, Project ID: %d, Track: %d",project_id,track);
												p_connection_module->Log(log);
											}
										}
//#ifdef _DEBUG
//										log.Format("%d spoofs sent to %s",supply_iter->v_supply_data_by_track[0].v_supply_data.size(),inet_ntoa(addr));
//										p_connection_module->Log(log);
//#endif
									}
								}
							}
							else
							{
								log.Format("Track: %d not in supply vector, Project ID: %d",track,project_id);
								p_connection_module->Log(log);
							}
							m_spoofs_sent+=spoof_sent;
							intermediate_spoof_sent+=spoof_sent;
							spoof_sent=0;
						}
						int rand_sent= spoof_sent;
						int rand_total=rand()%1000 + rand_sent;
						SendSearchEnd(ip,port,hash,rand_sent,rand_total);
						//SendSearchEnd(ip,port,hash,rand_sent,rand_total);
						//SendSearchEnd(ip,port,hash,rand_sent,rand_total);
					}
				}
				if(!intermediate_spoof_sent)
				{
//#ifdef _DEBUG
//					log.Format("No supply to spoof to %s",inet_ntoa(addr));
//					p_connection_module->Log(log);
//#endif
				}
			}
			break;			
		}
		case 0x13: //publish keyword hash
		{
			m_publish_received++;
			//ptr++;
			/*
			byte hash[16];
			memcpy(hash,ptr,16);
			SendPublishAck(ip,port,hash);
			*/
			break;

		}
		case 0x14: //publish ack
		{
			//ptr++;
			//m_publish_ack_received++;
			break;
		}
	}
}

//
//
//
void OvernetUDPSocket::SendConnect(const char* ip, unsigned short int port, byte* hash)
{
	unsigned char buf[25];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x0a;
	memcpy(&buf[2],hash,16);
	*(int*)&buf[18]=0;
	*(unsigned short int*)&buf[22]=m_my_udp_port;
#ifdef _DEBUG
	*(byte*)&buf[24]=0x01;
#else
	*(byte*)&buf[24]=0x00;
#endif

	unsigned long ip_int = inet_addr(ip);
	UINT num_sent=0;
	int ret = SendTo(buf,sizeof(buf),ip_int,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
}


void OvernetUDPSocket::SendConnect(UINT ip, unsigned short int port, byte* hash)
{
	unsigned char buf[25];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x0a;
	memcpy(&buf[2],hash,16);
	*(int*)&buf[18]=0;
	*(unsigned short int*)&buf[22]=m_my_udp_port;

#ifdef _DEBUG
	*(byte*)&buf[24]=0x01;
#else
	*(byte*)&buf[24]=0x00;
#endif

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
	//UINT size=0;
	//for(UINT i=0;i<v_group_hashes.size();i++)
	//	size += (UINT)v_group_hashes[i].hs_peers.size();
	//return size;
	return m_number_overnet_users;
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
/*
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
*/
//
//
//
void OvernetUDPSocket::OneSecondTick()
{
	UpdateStatus();
	m_seconds++;
	/*
	if(m_completed_searching_one_round &&
		p_connection_module->m_status_data.m_num_total_keyword_hashes_in_queue_to_be_sent==0)
	{
		p_connection_module->Log("Completed one round searching.");
		//LogStatusToFile();
		//BuildSpoofs();
		BuildKeywordHashList();
	}
	*/
	if(m_seconds%m_next_reset_time==0)//4-6 hours, reset peers' IPs
	{
		SetNextResetTime();
		//LogStatusToFile();
		for(int i=0; i<(int)v_group_hashes.size();i++)
		{
			v_group_hashes[i].hs_peers.clear();
		}
		m_number_overnet_users=0;
		/*
		p_connection_module->m_hash_manager.ResetHashPtr();
		p_connection_module->Log("Reading generated hashes from DB started");
		p_connection_module->m_hash_manager.ReadInHashesFromDB();
		p_connection_module->Log("Reading generated hashes from DB ended");
		*/
		p_connection_module->Log("Timer fires off, resetting peer's IPs");
		//BuildSpoofs();
		BuildKeywordHashList();
	}
	if(m_seconds%10==0) //farming peer IPs
	{
		if(ReturnNumberOfPeers() > 400000)
		{
			for(int i=0; i<(int)v_group_hashes.size();i++)
			{
				v_group_hashes[i].hs_peers.clear();
			}
			m_number_overnet_users=0;
			p_connection_module->Log("Over 400,000 users, resetting peer's IPs");
		}
		if(!m_my_ip)
			GetMyIP();
		if(ReturnNumberOfPeers()>100)
		{
			UINT index = (UINT)rand()%v_group_hashes.size();
			hash_set<OvernetPeer>::const_iterator iter;
			int count=0;
			while(count<50 && count<(int)v_group_hashes[index].hs_peers.size())
			{
				iter = v_group_hashes[index].hs_peers.begin();
				int random = rand()%((UINT)v_group_hashes[index].hs_peers.size());
				for(int i=0;i<random;i++)
					iter++;
				if(hs_my_user_hashes.size())
				{
					if(m_hash_search_index < hs_my_user_hashes.size())
					{
						hash_set<UserHash>::iterator my_iter = hs_my_user_hashes.begin();
						for(UINT j=0;j<=m_hash_search_index;j++)
							my_iter++;
						SendConnect(iter->m_ip,iter->m_port,my_iter->m_user_hash);
					}
				}
				count++;
			}
		}
		else
		{
			if(hs_my_user_hashes.size())
			{
				if(m_hash_search_index < hs_my_user_hashes.size())
				{
					hash_set<UserHash>::iterator iter = hs_my_user_hashes.begin();
					for(UINT j=0;j<=m_hash_search_index;j++)
						iter++;
					SendConnect(BOOTSTRAP,7871,iter->m_user_hash);
				}
			}
		}
	}
#if _DEBUG
	if(ReturnNumberOfPeers()>100000)
#else
	if(ReturnNumberOfPeers()>200000)
#endif
	{
		if(hs_my_user_hashes.size())
		{
			hash_set<UserHash>::iterator iter = hs_my_user_hashes.begin();
			if(m_hash_search_index > hs_my_user_hashes.size()-1)
			{
				m_hash_search_index=0;
				p_connection_module->Log("One round completed, resetting publicize history");
				while(iter!=hs_my_user_hashes.end())
				{
					iter->hs_peer_hashes.clear();
					iter++;
				}
				iter = hs_my_user_hashes.begin();
				p_connection_module->Log("History resetting done");
			}
			else
			{
				for(UINT i=0;i<m_hash_search_index;i++)
					iter++;
			}
			hash_set<OvernetPeer>::iterator iter_peer = v_group_hashes[iter->m_user_hash[0]].hs_peers.begin();
			UINT search_count=0;
			UINT publish_count=0;
			while(iter_peer != v_group_hashes[iter->m_user_hash[0]].hs_peers.end())
			{
				publish_count=1;
				//if(iter_peer->m_id[1]==iter->m_user_hash[1] )
				//{
					pair< hash_set<OvernetPeer>::iterator, bool > pr_peer;
					pr_peer = iter->hs_peer_hashes.insert(*(iter_peer));
					if(pr_peer.second)
					{
						SendPublicizeMyself(iter_peer->m_ip,iter_peer->m_port,iter->m_user_hash);
						publish_count++;
					}
					SendSearch(iter->m_user_hash,iter_peer->m_ip,iter_peer->m_port,0x14);
					search_count++;
				//}
				iter_peer++;
			}
			if(search_count==0) //didn't send a search to peer with the same first two byte hash
			{
				iter_peer = v_group_hashes[iter->m_user_hash[0]].hs_peers.begin();
				while(iter_peer != v_group_hashes[iter->m_user_hash[0]].hs_peers.end())
				{
					int disc=GetHashDistince(iter_peer->m_id,iter->m_user_hash);
					if(disc<=4 && disc!=0)
					{
						SendSearch(iter->m_user_hash,iter_peer->m_ip,iter_peer->m_port,0x14); //send search to peer with the same first byte hash
						search_count++;
					}
					iter_peer++;
				}
			}
			if(publish_count==0) // no peers with the same first byte hash to publish
			{
				int index1 = iter->m_user_hash[0] - 1;
				if(index1 < 0)
					index1 = 0;
				int index2 = iter->m_user_hash[0] + 1;
				if(index2 > 0xFF)
					index2 = 0xFF;

				iter_peer =  v_group_hashes[index1].hs_peers.begin();
				while(iter_peer != v_group_hashes[index1].hs_peers.end())
				{
					SendSearch(iter->m_user_hash,iter_peer->m_ip,iter_peer->m_port,0x14);
					search_count++;
					iter_peer++;
				}
				iter_peer =  v_group_hashes[index2].hs_peers.begin();
				while(iter_peer != v_group_hashes[index2].hs_peers.end())
				{
					SendSearch(iter->m_user_hash,iter_peer->m_ip,iter_peer->m_port,0x14);
					search_count++;
					iter_peer++;
				}
			}
			if(search_count==0)
			{
				p_connection_module->Log("No peers are similar to this hash, fuck it!!");
			}
			iter++;
			m_hash_search_index++;
		}
	}
	/*
	//publish stuff in publish queue
	hash_set<PublishQueue>::iterator iter=hs_publish_queues.begin();
	while(iter!=hs_publish_queues.end())
	{
		//bool no_more=false;
		for(int i=0;i<30;i++)
		{
			if(iter->v_to_be_published.size())
			{
				MetaData data = iter->PublishAckReceived();
				SendPublishKeywordHash(iter->m_ip,iter->m_port,data);
			}
			else
			{
				//hs_publish_queues.erase(iter);
				//no_more=true;
				break;
			}
		}
		//if(!no_more)
		iter++;
		//else
		//	break;
	}	
	*/
}

//
//
//
CString OvernetUDPSocket::GetExtension(const char* filename)
{
	CString ext = filename;
	int index = ext.ReverseFind('.');
	if(index > 0)
	{
		ext.Delete(0,index+1);
	}
	else
		ext="";
	return ext;
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
			if(strlen(ptr)>2 && strcmp(ptr,"the")!=0 && strcmp(ptr,"mp3"))
			{
				artist_keywords->push_back(ptr);
				delete [] lbuf;
				return;
			}
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
//
//
void OvernetUDPSocket::UpdateStatus()
{
	/** ToDo
	UINT num_total_hashes_to_be_published;
	UINT num_hashes_not_yet_published;
	num_total_hashes_to_be_published=num_hashes_not_yet_published=0;
	hash_set<SearchSet>::const_iterator iter = hs_hashes.begin();
	while(iter!=hs_hashes.end())
	{
		if(iter->hs_peer_hashes.size()==0)
			num_hashes_not_yet_published++;
		//num_total_hashes_to_be_published += iter->v_server_IPs.size();
		num_total_hashes_to_be_published += iter->hs_meta_data.size();
		iter++;
	}
	p_connection_module->m_status_data.m_num_unique_hashes_to_be_published = hs_hashes.size();
	p_connection_module->m_status_data.m_num_total_hashes_to_be_published = num_total_hashes_to_be_published;
	p_connection_module->m_status_data.m_num_hashes_not_yet_published = num_hashes_not_yet_published;
	p_connection_module->m_status_data.m_num_IPs_to_send_keyword_hashes_to=hs_publish_queues.size();
	p_connection_module->m_status_data.m_num_total_keyword_hashes_in_queue_to_be_sent=ReturnNumberOfRemainingKeywordHashesToBePublished();
	*/
	p_connection_module->m_status_data.m_num_peers = ReturnNumberOfPeers();
	p_connection_module->m_status_data.m_userhash_to_publicize = (UINT)hs_my_user_hashes.size();
	p_connection_module->m_status_data.m_publicize_ack_received=m_publicize_ack_received;
	p_connection_module->m_status_data.m_hash_search_index = m_hash_search_index;
	p_connection_module->m_status_data.m_queries_received=m_queries_received;
	p_connection_module->m_status_data.m_publish_received=m_publish_received;
	p_connection_module->m_status_data.m_spoofs_sent=m_spoofs_sent;
}

//
//
//
/*
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
	if(meta.m_file_type.CompareNoCase("Audio")==0)
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
	else if(meta.m_file_type.CompareNoCase("Video")==0)
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
	else if( (meta.m_file_type.CompareNoCase("Pro")==0) ||
				(meta.m_file_type.CompareNoCase("OTHER")==0) )
	{
		next_buf.PutByte(0x02);
		next_buf.PutUShort(1);
		next_buf.PutByte(0x03);
		next_buf.PutUShort(meta.m_file_type.GetLength()); // "Video" length
		next_buf.PutStr(meta.m_file_type,meta.m_file_type.GetLength());
		tag_count++;
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
*/

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
}

//
//
//
void OvernetUDPSocket::SendPublicizeMyself(UINT ip, unsigned short int port, byte* hash)
{
	if(m_my_ip != 0)
	{
		unsigned char buf[25];
		*(byte*)&buf[0]=0xe3;
		*(byte*)&buf[1]=0x0c;
		memcpy(&buf[2],hash,16);
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
	}
	else
		TRACE("NO LOCAL IP FOUND\n");
}

//
//
//
void OvernetUDPSocket::BuildKeywordHashList()
{
	p_connection_module->Log("Building keyword hash list started");
	hs_my_user_hashes.clear();
	hs_keyword_hashes.clear();
	ClearStatus();
//#ifdef SKYCAT
//			//vector<ProjectKeywords>::iterator project_iter = v_all_project_keywords.begin();
//			byte hash[16];
//			ZeroMemory(&hash,16);
//			CString word = "skycat";
//			GetHashFromWord(hash,word);
//			
//			KeywordHash keyword_hash;
//			keyword_hash.SetHash(hash);
//			//keyword_hash.vp_project_keywords.push_back(project_iter);
//			keyword_hash.m_keyword=word;
//			pair<hash_set<KeywordHash>::iterator,bool>pr_keyword_hash;
//			pr_keyword_hash=hs_keyword_hashes.insert(keyword_hash);
//			//if(!pr_keyword_hash.second)
//			//	pr_keyword_hash.first->vp_project_keywords.push_back(project_iter);
//
//			PartialRandomizeHash(hash,3);
//			pr_keyword_hash.first->SetUserHash(hash);
//
//			UserHash userhash;
//			userhash.SetHash(hash);
//			pair< hash_set<UserHash>::iterator, bool > pr;
//			pr=hs_my_user_hashes.insert(userhash);
//			for(UINT i=0;i<NUM_USERHASH_PER_KEYWORD+2;i++)
//			{
//				PartialRandomizeHash(hash,2);
//				UserHash userhash;
//				userhash.SetHash(hash);
//				hs_my_user_hashes.insert(userhash);
//			}
//			for(UINT i=0;i<NUM_USERHASH_PER_KEYWORD+4;i++)
//			{
//				PartialRandomizeHash(hash,1);
//				UserHash userhash;
//				userhash.SetHash(hash);
//				hs_my_user_hashes.insert(userhash);
//			}
//#else
//	UINT j;
	vector<ProjectKeywords>::iterator project_iter = v_all_project_keywords.begin();
	while(project_iter != v_all_project_keywords.end())
	{
		//artist name
		vector<CString>artist_keywords;
		GetArtistKeywords(project_iter->m_supply_keywords.m_search_string.c_str(),&artist_keywords);
		if(artist_keywords.size())
		{
			byte hash[16];
			ZeroMemory(&hash,16);
			char* word=new char[artist_keywords[0].GetLength()+1];
			strcpy(word,artist_keywords[0]);
			GetHashFromWord(hash,word);
			
			KeywordHash keyword_hash;
			keyword_hash.SetHash(hash);
			keyword_hash.vp_project_keywords.push_back(project_iter);
			keyword_hash.m_keyword=word;
			delete [] word;
			pair<hash_set<KeywordHash>::iterator,bool>pr_keyword_hash;
			pr_keyword_hash=hs_keyword_hashes.insert(keyword_hash);
			if(!pr_keyword_hash.second)
				pr_keyword_hash.first->vp_project_keywords.push_back(project_iter);

			pair< hash_set<UserHash>::iterator, bool > pr;

			for(UINT i=0;i<NUM_USERHASH_PER_KEYWORD;i++)
			{
				PartialRandomizeHash(hash,4);
				pr_keyword_hash.first->SetUserHash(hash);
				UserHash userhash;
				userhash.SetHash(hash);
				pr=hs_my_user_hashes.insert(userhash);
			}

#ifdef _DEBUG
			if(!pr.second)
				TRACE("Keyword - %s - of project - %s - is already in the list\n",artist_keywords[0],project_iter->m_project_name.c_str());
#endif
			//for(UINT i=0;i<NUM_USERHASH_PER_KEYWORD-1;i++)
			//{
			//	PartialRandomizeHash(hash,2);
			//	UserHash userhash;
			//	userhash.SetHash(hash);
			//	hs_my_user_hashes.insert(userhash);
			//}
			//for(UINT i=0;i<NUM_USERHASH_PER_KEYWORD;i++)
			//{
			//	PartialRandomizeHash(hash,1);
			//	UserHash userhash;
			//	userhash.SetHash(hash);
			//	hs_my_user_hashes.insert(userhash);
			//}
		}
#ifdef _DEBUG
		else
			TRACE("No keyword to use for project - %s\n",project_iter->m_project_name.c_str());
#endif
		//track name
//		for(j=0;j<project_iter->m_supply_keywords.v_keywords.size();j++)
//		{
//			vector<CString>track_keywords;
//			GetArtistKeywords(project_iter->m_supply_keywords.v_keywords[j].m_track_name.c_str(),&track_keywords);
//			if(track_keywords.size())
//			{
//				byte hash[16];
//				ZeroMemory(&hash,16);
//				char* word=new char[track_keywords[0].GetLength()+1];
//				strcpy(word,track_keywords[0]);
//				GetHashFromWord(hash,word);
//
//				KeywordHash keyword_hash;
//				keyword_hash.SetHash(hash);
//				keyword_hash.vp_project_keywords.push_back(project_iter);
//				keyword_hash.m_keyword=word;
//				delete [] word;
//
//				pair<hash_set<KeywordHash>::iterator,bool>pr_keyword_hash;
//				pr_keyword_hash=hs_keyword_hashes.insert(keyword_hash);
//				if(!pr_keyword_hash.second)
//					pr_keyword_hash.first->vp_project_keywords.push_back(project_iter);
//
//				UserHash userhash;
//				PartialRandomizeHash(hash,3);
//				userhash.SetHash(hash);
//				pair< hash_set<UserHash>::iterator, bool > pr;
//				pr=hs_my_user_hashes.insert(userhash);
//#ifdef _DEBUG
//				if(!pr.second)
//					TRACE("Track Keyword - %s - of project - %s - Title - %s - is already in the list\n",track_keywords[0],
//						project_iter->m_project_name.c_str(),project_iter->m_supply_keywords.v_keywords[j].m_track_name.c_str());
//#endif
//				for(UINT i=0;i<NUM_USERHASH_PER_KEYWORD-1;i++)
//				{
//					UserHash userhash;
//					PartialRandomizeHash(hash,2);
//					userhash.SetHash(hash);
//					hs_my_user_hashes.insert(userhash);
//				}
//
//			}
//#ifdef _DEBUG
//			else
//				TRACE("No keyword to use for project - %s - Title - %s\n",project_iter->m_project_name.c_str(),
//						project_iter->m_supply_keywords.v_keywords[j].m_track_name.c_str());	
//#endif
//		}
		project_iter++;
	}
//#endif
	p_connection_module->Log("Building keyword hash list ended");
	m_hash_search_index=0;
	if(hs_my_user_hashes.size())
	{
		if(m_hash_search_index < hs_my_user_hashes.size())
		{
			hash_set<UserHash>::iterator iter = hs_my_user_hashes.begin();
			for(UINT j=0;j<=m_hash_search_index;j++)
				iter++;
			SendConnect(BOOTSTRAP,7871,iter->m_user_hash);
		}
	}
}

//
//
//
void OvernetUDPSocket::PartialRandomizeHash(byte* hash, int start)
{
	//if(hash[start]!=0xFF)
	//	hash[start]=hash[start]++;
	for(int i=start;i<16;i++)
	{
		hash[i]=hash[i]^m_my_ip;//rand()%256;
	}
}
//
//
//
void OvernetUDPSocket::GetHashFromWord(byte* hash, char* word)
{
	m_df.CreateHashFromInput(NULL,NULL,(int)strlen(word),hash,(byte*)word);
}
//
//
//
void OvernetUDPSocket::FindMyNearestUserHash(byte* hash, vector<UserHash>& hashes)
{
	hash_set<UserHash>::iterator iter=hs_my_user_hashes.begin();
	while(iter!=hs_my_user_hashes.end())
	{
		if(hash[0]==iter->m_user_hash[0] && hash[1]==iter->m_user_hash[1])
			hashes.push_back(*iter);
		iter++;
	}
}

//
//
//
int OvernetUDPSocket::MatchingTrack(const char* query, vector<vector<ProjectKeywords>::iterator>& projects, UINT &project_id, bool& audio_project)
{

	int track=-1;
	UINT j,k,h;
	vector<vector<ProjectKeywords>::iterator>::iterator project_iter = projects.begin();
	while(project_iter!=projects.end())
	{
		//if(!unknown_track)
		//{
			// Extract the keywords from the query
			vector<string> keywords;
			ExtractKeywordsFromQuery(&keywords,query);
			int weight=100;
			/**** V1.2 skipped the second keyword, so no weighted exact keyword comparison here ***/
			/*
			// Check all of the keywords for this project
			for(j=0;j<(*project_iter)->m_query_keywords.v_keywords.size();j++)
			{
				// Against all of the keywords for this query
				for(k=0;k<keywords.size();k++)
				{
					if(strstr(keywords[k].c_str(),(*project_iter)->m_query_keywords.v_keywords[j].keyword.c_str())!=0)
					{
						weight+=(*project_iter)->m_query_keywords.v_keywords[j].weight;
					}
				}
			}

			// Check all of the exact keywords for this project
			for(j=0;j<(*project_iter)->m_query_keywords.v_exact_keywords.size();j++)
			{
				// Against all of the keywords for this query
				for(k=0;k<keywords.size();k++)
				{
					if( (stricmp((*project_iter)->m_query_keywords.v_exact_keywords[j].keyword.c_str(),"the")==0) ||
						(strlen((*project_iter)->m_query_keywords.v_exact_keywords[j].keyword.c_str())<=2))
						weight+=(*project_iter)->m_query_keywords.v_exact_keywords[j].weight;
					else
					{
						if(strcmp(keywords[k].c_str(),(*project_iter)->m_query_keywords.v_exact_keywords[j].keyword.c_str())==0)
						{
							weight+=(*project_iter)->m_query_keywords.v_exact_keywords[j].weight;
						}
					}
				}
			}
			*/

			// If the weight is >= 100, then this is a query match for the current project.  Check with the kill words, just in case
			if(weight>=100)
			{
				for(j=0;j<keywords.size();j++)
				{
					// See if any of the project killwords are within the query's keywords
					bool found=false;
					for(k=0;k<(*project_iter)->m_query_keywords.v_killwords.size();k++)
					{
						if(strstr(keywords[j].c_str(),(*project_iter)->m_query_keywords.v_killwords[k].keyword.c_str())!=0)
						{
							found=true;
							weight=0;
							break;
						}
					}

					// If it didn't match a project killword, see if any of the project exact killwords are equal to the query's keywords
					if(!found)
					{
						for(k=0;k<(*project_iter)->m_query_keywords.v_exact_killwords.size();k++)
						{
							if(strcmp(keywords[j].c_str(),(*project_iter)->m_query_keywords.v_exact_killwords[k].keyword.c_str())==0)
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

			// If the weight is still >= 100, then this is definitely a match
			if(weight>=100)
			{
				if( (*project_iter)->m_search_type == ProjectKeywords::audio )
					audio_project=true;
				else
					audio_project=false;
				project_id = (*project_iter)->m_id;
				// Find the appropriate track (if any) this query is looking for
				track=0;
				RemoveArtistNames(keywords,(*project_iter)->m_artist_name.c_str());

				if(keywords.size())
				{
					for(j=0;j<(*project_iter)->m_supply_keywords.v_keywords.size();j++) //for each track
					{
						bool found=true;
						for(h=0;h<keywords.size();h++) //for each keyword
						{
							if(strstr((*project_iter)->m_supply_keywords.v_keywords[j].m_track_name.c_str(),keywords[h].c_str())==NULL)
							{
								found=false;
								break;
							}
						}
						if(found)
						{
							track=(*project_iter)->m_supply_keywords.v_keywords[j].m_track;
							return track;
						}
					}
				}
			}
		//}
		//else //unknown track
		//{
		//	if( (*project_iter)->m_search_type == ProjectKeywords::audio )
		//		audio_project=true;
		//	else
		//		audio_project=false;
		//	project_id = (*project_iter)->m_id;
		//	// Find the appropriate track (if any) this query is looking for
		//	track=0;
		//	for(j=0;j<(*project_iter)->m_supply_keywords.v_keywords.size();j++) //for each track
		//	{
		//		CString track_name = (*project_iter)->m_supply_keywords.v_keywords[j].m_track_name.c_str();
		//		int count = track_name.Replace(" ","_");
		//		if(!count)
		//		{
		//			track=(*project_iter)->m_supply_keywords.v_keywords[j].m_track;
		//			return track;
		//		}
		//	}
		//}
		project_iter++;
	}
	return track;
}

//
//
//
void OvernetUDPSocket::MatchingTracks(vector<vector<ProjectKeywords>::iterator>& projects, UINT &project_id, bool& audio_project,vector<int>& tracks)
{
	UINT j;
	vector<vector<ProjectKeywords>::iterator>::iterator project_iter = projects.begin();
	while(project_iter!=projects.end())
	{
		bool tracks_added=false;
		if( (*project_iter)->m_search_type == ProjectKeywords::audio )
			audio_project=true;
		else
			audio_project=false;
//		project_id = (*project_iter)->m_id;
		// Find the appropriate track (if any) this query is looking for
		for(j=0;j<(*project_iter)->m_supply_keywords.v_keywords.size();j++) //for each track
		{
			CString track_name = (*project_iter)->m_supply_keywords.v_keywords[j].m_track_name.c_str();
			int count = track_name.Replace(" ","_");
			if(!count)
			{
				tracks.push_back((*project_iter)->m_supply_keywords.v_keywords[j].m_track);
				tracks_added=true;
			}
		}
		if(tracks_added || !audio_project)
		{
			project_id = (*project_iter)->m_id;
			if(!audio_project)
				tracks.push_back(0);
			return;
		}
		project_iter++;
	}
	return;
}

//
//
//
void OvernetUDPSocket::ExtractKeywordsFromQuery(vector<string> *keywords,const char *query)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(query)+1];
	strcpy(lbuf,query);
	//strlwr(lbuf);

	// Remove all non alpha-numeric characters
	ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	cstring.MakeLower();
	//string cstring_string=cstring;
	strcpy(lbuf,cstring);

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
		if(((buf[i]>='0') && (buf[i]<='9')) || ((buf[i]>='A') && (buf[i]<='Z')) || ((buf[i]>='a') && (buf[i]<='z')) || (buf[i]=='\''))
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
void OvernetUDPSocket::RemoveArtistNames(vector<string>& keywords, CString artist_name)
{
	artist_name.MakeLower();
	vector<string>::iterator iter=keywords.begin();
	while(iter!=keywords.end())
	{
		if(strstr(artist_name, iter->c_str())!=NULL)
		{
			keywords.erase(iter);
		}
		else
			iter++;
	}
}

//
//
//
void OvernetUDPSocket::SendSearchNext(UINT ip, unsigned short int port,byte* hash, byte* user_hash)
{
	unsigned char buf[42];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x0f;
	memcpy(&buf[2], hash, 16); 
	buf[18]=0x01; //peer list size
	memcpy(&buf[19],user_hash,16);
	*(UINT*)&buf[35]=m_my_ip;
	*(unsigned short int*)&buf[39]=m_my_udp_port;
	buf[41]=0x00; //peer type?

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
void OvernetUDPSocket::SendSearchNext(UINT ip, unsigned short int port,byte* hash, hash_set<OvernetPeer,OvernetPeerHash>& user_hashes)
{
	UINT user_size;
	if(user_hashes.size() >= 20)
		user_size=20;
	else
		user_size=(UINT)user_hashes.size();
	if(user_size==0)
		return;
	int buf_len=(23*user_size)+19;
	unsigned char* buf = new unsigned char[buf_len];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x0f;
	memcpy(&buf[2], hash, 16); 
	buf[18]= (unsigned char)user_size;//peer list size
	unsigned char* ptr = &buf[19];
	hash_set<OvernetPeer>::iterator user_iter=user_hashes.begin();
	for(UINT i=0;i<user_size;i++)
	{
		memcpy(ptr,user_iter->m_id,16);
		ptr+=16;
		*(UINT*)ptr=user_iter->m_ip;
		ptr+=4;
		*(unsigned short int*)ptr=user_iter->m_port;
		ptr+=2;
		byte peer_type = (byte)rand()%5;
		*(byte *)ptr=peer_type; //peer type?
		ptr++;
		user_iter++;
	}

	UINT num_sent=0;
	int ret = SendTo(buf,buf_len,ip,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
	delete [] buf;
}

//
//
//
void OvernetUDPSocket::SendSearchEnd(UINT ip, unsigned short int port,byte* hash, unsigned short int result_sent, unsigned short int total_result)
{
	unsigned char buf[22];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x12;
	memcpy(&buf[2], hash, 16); 
	*(unsigned short int*)&buf[18]=result_sent;
	*(unsigned short int*)&buf[20]=total_result;

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
void OvernetUDPSocket::SendPublishAck(UINT ip, unsigned short int port,byte* hash)
{
	unsigned char buf[18];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x14;
	memcpy(&buf[2], hash, 16); 
	//&buf[18]=availability; //availability?

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
void OvernetUDPSocket::ClearStatus()
{
	m_queries_received=0;
	m_publicize_ack_received=0;
	m_publish_received=0;
	m_spoofs_sent=0;
}

//
//
//
void OvernetUDPSocket::SetNextResetTime()
{
	m_seconds=0;
	m_next_reset_time=(((UINT)rand())%3600)+7200; //2-3 hours
}

//
//
//
void OvernetUDPSocket::SendSpoofResult(UINT ip, unsigned short int port, byte* hash, PacketBuffer* packet)
{
	UINT buf_len = 34 + packet->m_used;
	byte *buf = new byte[buf_len];
	*(byte*)&buf[0]=0xe3;
	*(byte*)&buf[1]=0x11;
	memcpy(&buf[2], hash, 16); //keyword hash
	for(UINT i=18; i<34;i++)
		buf[i]=rand()%256; //random file hash
	memcpy(&buf[34], packet->p_data, packet->m_used);

#ifdef IP_SPOOF
	UINT src_ip=inet_addr("66.65.64.63");
	SendIPSpoofDataPacket(src_ip,4662,ip,port,buf,buf_len);
#else
	UINT num_sent=0;
	int ret = SendTo(buf,buf_len,ip,port,&num_sent);
	int error=0;
	if(ret==-1)
	{
		error=GetLastError();
	}
#endif
	delete [] buf;
}

//
//
//
void OvernetUDPSocket::SendIPSpoofDataPacket(unsigned int src_ip,unsigned short int src_port,unsigned int dest_ip,unsigned short int dest_port,
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
	// Source port : 41170
	udp[0]=(src_port>>8)&0xFF;
	udp[1]=(src_port>>0)&0xFF;
	// Dest port : 41170
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
}