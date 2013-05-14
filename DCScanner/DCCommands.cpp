#include "StdAfx.h"
#include "dccommands.h"
#include "SpoofStruct.h"

DCCommands::DCCommands(void)
{
	hub_ip = new char[200];
}

DCCommands::~DCCommands(void)
{
	//avoid exception error
	if(hub_ip != NULL)
	{
		//delete [] hub_ip;
	}
}

void DCCommands::GetProjectKeywordsVector(ProjectKeywordsVector *pk)
{
	p_pkv=pk;
}

CString DCCommands::SendKey(char* data)	//returns all the lock and pk to establish client-server handshake
{
	char* pk;
	char* ptr;
	ptr = strstr(data,"$Lock");
	pk =strstr(data," Pk=");
	if(ptr==NULL || pk==NULL || ptr>pk)
	{
		return "Empty";
	}
	ptr=ptr+6;	//remove the $lock from string
	//lock[256];
	//memset(lock,0,sizeof(lock));
	strcpy(lock,"");
	int i = 0;
	while(ptr != pk && i<256)
	{
		lock[i] = *ptr;
		ptr++;
		i++;
	}
	lock[i]='\0';	//end character
	CString lockstring = lock;
	m_key = "$Key ";
	m_key=m_key+m_keymanager.LocktoKey(lockstring);
	m_key=m_key+ "|$ValidateNick ";
	//raphael m_key=m_key+ "big_swapperr|";
	m_key=m_key+this->m_nick;
	m_key=m_key+ "|";
	return m_key;
}
void DCCommands::SenyMyInfo(char* data,char* str_myinfo)	//returns all $MyInfo string to send info to the hub
{
	//Raphael MyInfo="$Version 2.02|$GetNickList|$MyINFO $ALL big_swapperr none<DC V:2.02,M:A,H:1/0/0,S:6,0:6>$ $LAN(T1).$none$84000000000$|";
	//memset(MyInfo,0,sizeof(MyInfo));
	strcpy(MyInfo, "$Version 2.02|$GetNickList|$MyINFO $ALL ");
	strcat(MyInfo,this->m_nick);
	strcat(MyInfo," none<DC V:2.02,M:A,H:1/0/0,S:6,0:6>$ $LAN(T1).$none$55104430407$|");
	strcpy(str_myinfo,MyInfo);
	//delete [] MyInfo;
	return;
}
void DCCommands::SendMyNick(char* data,char* str_mynick)	//returns $MyNick info when a $connect to me is received
{
	//memset(MyNick,0,sizeof(MyNick));
	strcpy(MyNick,"$MyNick");
	//Raphael strcat(MyNick," big_swapperr|$Lock=");
	strcat(MyNick," ");
	strcat(MyNick,this->m_nick);
	strcat(MyNick,"|$Lock=");
	/////////////////////////////
	strcat(MyNick,m_keymanager.CreateLock());
	strcat(MyNick," Pk=");
	//strcat(MyNick,m_keymanager.CreatePK());
	strcat(MyNick,"$Version 1,0091|");
	//strcat(MyNick,"|");
	//char* send_MyNick=&MyNick[0];
	strcpy(str_mynick,MyNick);
	return;
}
void DCCommands::SendSRPasive(char* data,vector<string> &v_sr)		//returns the $SR command when a passive search is received
{
	querywords.clear();
	char* token;
	v_spoofs.clear();
	data_temp = data;
	data_temp = data_temp.Right(data_temp.GetLength()-data_temp.Find("$Search",0));
	//v_sr.clear();
	//////////////////////////////////////////////////////////////////////////////
	token = strtok(data_temp.GetBuffer(data_temp.GetLength()),"|");
	//extract correct portion of query
	if(token!=NULL)
	{
		data_temp = token;
	}
	
	CString temp=data_temp;
		char *tok=strtok(temp.GetBuffer(temp.GetLength())," ");
		char *extract_ip=NULL;
		char *extract_nick=NULL;
		int tok_num=0;
		while(tok!=NULL && tok_num<1)
		{
			tok=strtok(NULL," ");
			extract_ip=tok;		//requestee's ip address;
			tok_num++;
		}

		tok=strtok(extract_ip,":");
		tok_num=0;
		while(tok!=NULL)
		{
			if(tok_num==0)
				extract_ip=tok;
			else
				extract_nick=tok;
			tok=strtok(NULL,":");
			tok_num++;
		}

	if(extract_ip == NULL || extract_nick == NULL)
	{
		return;
	}
	//extract query words,
	token = strtok(data_temp.GetBuffer(data_temp.GetLength())," ");
	while(token != NULL)
	{
		data_temp = token;
		token = strtok(NULL," ");
	}

	int position =0;
	//further extraction of query words
	full_string = data_temp;
	token = strtok(data_temp.GetBuffer(data_temp.GetLength()),"?");
	query_words= data_temp;
	while(token != NULL)
	{
		query_words = token;
		token = strtok(NULL, "?");
	}
	token = strtok(query_words.GetBuffer(query_words.GetLength()),"$");
	while(token != NULL)
	{
		querywords.push_back(strlwr(token));
		token = strtok(NULL,"$");
	}

	weight = 0;
	found_killword = false;
	found_specific_track = false;
	UINT u;
	unsigned int projects_size = (unsigned int)p_pkv->v_projects.size();	//added to optimize for loop	2004-2-26
	for(UINT i=0;i<projects_size;i++)
	{
		if(p_pkv->v_projects[i].m_directconnect_spoofing_enabled)
		{
			weight = 0;
			found_killword = false;
			//check for killwords
			for(UINT j=0;j<p_pkv->v_projects[i].m_query_keywords.v_killwords.size();j++)
			{
				if(strstr(query_words.GetBuffer(query_words.GetLength()),p_pkv->v_projects[i].m_query_keywords.v_killwords[j].keyword.c_str())!=NULL)
				{
					//found_killword = true;
					//weight = 0;
					return;
				}
			}
			
			//check for exact killwords
			for(UINT j=0;j<p_pkv->v_projects[i].m_query_keywords.v_exact_killwords.size();j++)
			{
				for(u = 0;u<querywords.size();u++)
				{
					if(strcmp(querywords[u].c_str(),p_pkv->v_projects[i].m_query_keywords.v_exact_killwords[j].keyword.c_str())==0)
					{
						//found_killword = true;
						//weight = 0;
						return;
					}
				}
			}
			//if a killword was found don't go through more processing
			if(!found_killword)
			{
				//check for keywords
				for(UINT j=0;j<p_pkv->v_projects[i].m_query_keywords.v_keywords.size();j++)
				{
					if(strstr(query_words.GetBuffer(query_words.GetLength()),p_pkv->v_projects[i].m_query_keywords.v_keywords[j].keyword.c_str())!=NULL)
					{
						weight += p_pkv->v_projects[i].m_query_keywords.v_keywords[j].weight;
					}
				}
				//check for exact keywords that must be present
				for(UINT j=0;j<p_pkv->v_projects[i].m_query_keywords.v_exact_keywords.size();j++)
				{
					for(u = 0;u<querywords.size();u++)
					{
						if(strcmp(querywords[u].c_str(),p_pkv->v_projects[i].m_query_keywords.v_exact_keywords[j].keyword.c_str())==0)
						{
							weight += p_pkv->v_projects[i].m_query_keywords.v_exact_keywords[j].weight;
							break;	//exit for loop because it has already found the keyword it was looking for added 2004-2-26
						}
					}
				}
			}
			//there has been a match, continue with supply keywords
			if(weight>=100)
			{
				unique_track = false;
				found_match = false;
				exit = false;
				matches=0;
				found_killword = false;
				found_specific_track = false;
				//look for appropriate track this query is looking for using current project index
				//if there is no matches then the query is looking for all tracks,query for all tracks
				unsigned int killwords_size = (unsigned int)p_pkv->v_projects[i].m_supply_keywords.v_killwords.size();	//added to optimize 2004-02-26
				for(y=0;y<killwords_size;y++)
				{
					for(f=0;f<p_pkv->v_projects[i].m_supply_keywords.v_killwords[y].v_keywords.size();f++)
					{
						if(strstr(query_words.GetBuffer(query_words.GetLength()),p_pkv->v_projects[i].m_supply_keywords.v_killwords[y].v_keywords[f])!=NULL)
						{
							found_killword = true;
							return;
						}
					}
				}
					for(y=0;y<p_pkv->v_projects[i].m_supply_keywords.v_keywords.size();y++)
					{
						matches = 0;
						for(f=0;f<p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords.size();f++)
						{
							//TRACE("CHECK %d: %s\n",y,p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords[f]);
							if(strstr(full_string.GetBuffer(full_string.GetLength()),p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords[f])!=NULL)
							{
								//TRACE("match: %s",p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords[f]);
								matches++;	//increment matches
							}
						}
						if(matches==p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords.size())
							{
								unique_track = true;
								//TRACE("MATCH FOUND");
								break;
							}
					}
						if(y>p_pkv->v_projects[i].m_supply_keywords.v_keywords.size())
						{
							y=0;
						}
						//found a match, exit for loop
						//send spoof for current track
						//if(matches==p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords.size() && p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords.size()!=0)
						if(unique_track)
						{
							//p_pkv->v_projects[i].m_project_name;
							//TRACE("PASSIVE QUERY RECEIVED: %s\n", query_words);
							//TRACE("PASSIVE MATCH FOUND! Project: %sTrack: %s\n",p_pkv->v_projects[i].m_project_name.c_str(),p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].m_track_name.c_str());
							m_db->ExtractData(p_pkv->v_projects[i].m_project_name.c_str(),p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].m_track_name.c_str(),"direct_connect_supply",v_spoofs);
							//do multiple spoofing here, return a vector of $sr
							for(UINT spoof_pos=0;spoof_pos<v_spoofs.size();spoof_pos++)
							{
								memset(SR,0,sizeof(SR));
								//memset(buffer,0,sizeof(buffer));
								char* ptr= NULL;//(int*)&SR[0];
								char* ptr2 = NULL;
								ptr2=&SR[0];
								strcpy(SR,"$SR ");
								strcat(SR,this->m_nick);
								strcat(SR, " ");	//file path
								strcat(SR,this->GetSpoofPath().c_str());
								strcat(SR,v_spoofs[spoof_pos].m_filename.GetBuffer(0));
								ptr2 += strlen(SR);
								ptr=ptr2;
								*ptr = 0x5;	//include ascii char 5
								ptr2+=1;
								strcat(SR,itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10));	//filesize
								strcat(SR," 3/3");
								ptr2 += strlen(itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10));
								ptr2 += 4;
								ptr=ptr2;
								*ptr =0x05;
								ptr2+=1;
								strcat(SR,this->m_hubname);
								strcat(SR,"(");
								strcat(SR,this->hub_ip);
								strcat(SR,":411");
								strcat(SR,")");
								ptr2 += strlen(this->hub_ip);
								ptr2 += strlen(this->m_hubname) +3;
								ptr2 += strlen("411");
								ptr=ptr2;
								*ptr = 0x05;
								strcat(SR,extract_nick);
								strcat(SR,"|");
								//char *str_sr;
								//strcpy(str_sr,SR);
								string temp_cstr=SR;
								v_sr.push_back(temp_cstr);
							}
							m_db->SingleTrackDataExists(p_pkv->v_projects[i].m_project_name.c_str(),p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].m_track);
							m_db->DataExists(p_pkv->v_projects[i].m_project_name.c_str());
							m_db->IncrementDBDecoys(p_pkv->v_projects[i].m_project_name.c_str(),(int)v_spoofs.size());
							m_db->IncrementDBRequests(p_pkv->v_projects[i].m_project_name.c_str(),1,p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].m_track);
							return;
						}
						//send spoof for entire project
						else
						{
							//TRACE("PASSIVE SEND SPOOF\n");
						/*	#ifdef _DEBUG
							CMemoryState oldMemState, newMemState, diffMemState;
							oldMemState.Checkpoint();
							#endif*/

							m_db->ExtractData(p_pkv->v_projects[i].m_project_name.c_str(),"","direct_connect_supply",v_spoofs);
							
						/*	#ifdef _DEBUG
							newMemState.Checkpoint();
							if( diffMemState.Difference( oldMemState, newMemState ) )
							{
								TRACE( "Memory leaked!\n" );
							}
							#endif*/
	
							//do multiple spoofing here, return a vector of $sr
							if(p_pkv->v_projects[i].m_directconnect_decoys_enabled)
							{
								for(UINT spoof_pos=0;spoof_pos<v_spoofs.size();spoof_pos++)
								{
									memset(SR,0,sizeof(SR));
									//memset(buffer,0,sizeof(buffer));
									char* ptr = NULL;//(int*)&SR[0];
									char* ptr2 = NULL;
									ptr2=&SR[0];
									strcpy(SR,"$SR ");
									strcat(SR,this->m_nick);
									strcat(SR, " ");	//file path
									strcat(SR,this->GetSpoofPath().c_str());
									strcat(SR,v_spoofs[spoof_pos].m_filename.GetBuffer(0));
									ptr2 += strlen(SR);
									ptr=ptr2;
									*ptr = 0x5;	//include ascii char 5
									ptr2+=1;
									strcat(SR,itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10));	//filesize
									strcat(SR," 3/3");
									ptr2 += strlen(itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10));
									ptr2 += 4;
									ptr=ptr2;
									*ptr =0x05;
									ptr2+=1;
									strcat(SR,this->m_hubname);
									strcat(SR,"(");
									strcat(SR,this->hub_ip);
									strcat(SR,":411");
									strcat(SR,")");
									ptr2 += strlen(this->hub_ip);
									ptr2 += strlen(this->m_hubname) +3;
									ptr2 += strlen("411");
									ptr=ptr2;
									*ptr = 0x05;
									strcat(SR,extract_nick);
									strcat(SR,"|");
									//char *str_sr;
									//strcpy(str_sr,SR);
									string temp_cstr=SR;
									v_sr.push_back(temp_cstr);
								}
							//	TRACE("PASSIVE DONE SETTING DECOYS\n");
							}	//decoys enabled end if
							
								m_db->DataExists(p_pkv->v_projects[i].m_project_name.c_str());
								m_db->IncrementDBDecoys(p_pkv->v_projects[i].m_project_name.c_str(),(int)v_spoofs.size());
								m_db->IncrementDBRequests(p_pkv->v_projects[i].m_project_name.c_str(),1,0);	//track zero...searching for entire project
								//TRACE("PASSIVE EXITING...\n");
								return;
						}
					
				//else there is a specific track being searched for

				//break;
			}
			//no match reset weight
			else
			{
				weight = 0;
			}
		}	//if directconnect project
	}
	/*char *tok=strtok(data,"|");
	char *search_for;
	bool found = false;
	while(tok!=NULL)
	{
			data=tok;	//get the tok which has the search criteria
			tok=strtok(NULL,"|");
	}

	if(1)
	{
		CString temp=data;
		char *tok=strtok(temp.GetBuffer(temp.GetLength())," ");
		char *extract_hub="";
		char *extract_nick="";
		int tok_num=0;
		while(tok!=NULL && tok_num<1)
		{
			tok=strtok(NULL," ");
			extract_hub=tok;
			tok_num++;
		}

		tok=strtok(extract_hub,":");
		tok_num=0;
		while(tok!=NULL)
		{
			if(tok_num!=0)
				extract_nick=tok;	//hub name
			tok=strtok(NULL,":");
			tok_num++;
		}
		for(int x=0;x<2;x++)
		{
			memset(&SR,0,sizeof(SR));
			int* ptr= NULL;//(int*)&SR[0];
			char* ptr2=&SR[0];
			strcpy(SR,"$SR ");
			strcat(SR,this->m_nick);
			strcat(SR, " Complete\\02-50_cent-in_da_club_(dirty)-gsm.1.1.1.mp3");	//file path
			ptr2 += strlen(SR);
			ptr=(int*)ptr2;
			*ptr = 0x5;	//include ascii char 5
			ptr2+=1;
			char file_size[100]="5290342 4/4";
			strcat(SR,"5290342 4/4");	//filesize
			ptr2 += strlen(file_size);
			ptr=(int*)ptr2;
			*ptr =0x05;
			ptr2+=1;
			strcat(SR,this->m_hubname);
			strcat(SR,"(");
			strcat(SR,this->hub_ip);
			strcat(SR,":1090");
			strcat(SR,")");
			ptr2 += strlen(this->hub_ip);
			ptr2 += strlen(this->m_hubname) +3;
			ptr2 += strlen("1090");
			ptr=(int*)ptr2;
			*ptr = 0x05;
			strcat(SR,extract_nick);
			strcat(SR,"|");
			//char *str_sr;
			//strcpy(str_sr,SR);
			string temp_cstr=SR;
			v_sr.push_back(temp_cstr);
		}
		//v_SR.push_back(SR);	//push the char* to the vector
		//delete [] str_sr;
		return;	//return the vector
	}
	return ;	//return an error*/
}
void DCCommands::SendSRActive(char* data,vector<string> &v_sr,char* port,char* ip_address)		//returns the $SR command when an active search is received
{
	querywords.clear();
	char* token;
	v_spoofs.clear();	//clear vector from previous results
	data_temp = data;
	data_temp = data_temp.Right(data_temp.GetLength()-data_temp.Find("$Search",0));
	//v_sr.clear();
	//////////////////////////////////////////////////////////////////////////////
	token = strtok(data_temp.GetBuffer(data_temp.GetLength()),"|");
	//extract correct portion of query
	if(token!=NULL)
	{
		data_temp = token;
	}
	
	CString temp=data_temp;
		char *tok=strtok(temp.GetBuffer(temp.GetLength())," ");
		char *extract_ip=NULL;
		char *extract_port=NULL;
		int tok_num=0;
		while(tok!=NULL && tok_num<1)
		{
			tok=strtok(NULL," ");
			extract_ip=tok;		//requestee's ip address;
			tok_num++;
		}

		tok=strtok(extract_ip,":");
		tok_num=0;
		while(tok!=NULL)
		{
			if(tok_num==0)
				extract_ip=tok;
			else
				extract_port=tok;
			tok=strtok(NULL,":");
			tok_num++;
		}

	if(extract_ip == NULL || extract_port == NULL)
	{
		return;
	}
	//if one of our supply collectors
	if(strcmp(extract_ip,"38.119.64.77")==0)
	{
		return;		//exit
	}
	//extract query words,
	token = strtok(data_temp.GetBuffer(data_temp.GetLength())," ");
	while(token != NULL)
	{
		data_temp = token;
		token = strtok(NULL," ");
	}

	int position =0;
	full_string = data_temp;
	//further extraction of query words
	token = strtok(data_temp.GetBuffer(data_temp.GetLength()),"?");
	query_words= data_temp;
	while(token != NULL)
	{
		query_words = token;
		token = strtok(NULL, "?");
	}
	token = strtok(query_words.GetBuffer(query_words.GetLength()),"$");
	while(token != NULL)
	{
		querywords.push_back(strlwr(token));
		token = strtok(NULL,"$");
	}

	weight = 0;
	found_killword = false;
	found_specific_track = false;
	UINT u;
	for(UINT i=0;i<p_pkv->v_projects.size();i++)
	{
		if(p_pkv->v_projects[i].m_directconnect_spoofing_enabled)
		{
			weight = 0;
			found_killword = false;
			//check for killwords
			for(UINT j=0;j<p_pkv->v_projects[i].m_query_keywords.v_killwords.size();j++)
			{
				if(strstr(query_words.GetBuffer(query_words.GetLength()),p_pkv->v_projects[i].m_query_keywords.v_killwords[j].keyword.c_str())!=NULL)
				{
					//found_killword = true;
					//weight = 0;
					return;
				}
			}
			
			//check for exact killwords
			for(UINT j=0;j<p_pkv->v_projects[i].m_query_keywords.v_exact_killwords.size();j++)
			{
				for(u = 0;u<querywords.size();u++)
				{
					if(strcmp(querywords[u].c_str(),p_pkv->v_projects[i].m_query_keywords.v_exact_killwords[j].keyword.c_str())==0)
					{
						//found_killword = true;
						//weight = 0;
						return;
					}
				}
			}
			//if a killword was found don't go through more processing
			if(!found_killword)
			{
				//check for keywords
				for(UINT j=0;j<p_pkv->v_projects[i].m_query_keywords.v_keywords.size();j++)
				{
					if(strstr(query_words.GetBuffer(query_words.GetLength()),p_pkv->v_projects[i].m_query_keywords.v_keywords[j].keyword.c_str())!=NULL)
					{
						weight += p_pkv->v_projects[i].m_query_keywords.v_keywords[j].weight;
					}
				}
				//check for exact keywords that must be present
				for(UINT j=0;j<p_pkv->v_projects[i].m_query_keywords.v_exact_keywords.size();j++)
				{
					for(u = 0;u<querywords.size();u++)
					{
						if(strcmp(querywords[u].c_str(),p_pkv->v_projects[i].m_query_keywords.v_exact_keywords[j].keyword.c_str())==0)
						{
							weight += p_pkv->v_projects[i].m_query_keywords.v_exact_keywords[j].weight;
							break;	//exit for loop because it has already found the keyword it was looking for added 2004-2-26
						}
					}
				}
			}
			//there has been a match, continue with supply keywords
			if(weight>=100)
			{
				unique_track = false;
				found_match = false;
				exit = false;
				matches=0;
				found_killword = false;
				found_specific_track = false;
				//look for appropriate track this query is looking for using current project index
				//if there is no matches then the query is looking for all tracks,query for all tracks
				for(y=0;y<p_pkv->v_projects[i].m_supply_keywords.v_killwords.size();y++)
				{
					for(f=0;f<p_pkv->v_projects[i].m_supply_keywords.v_killwords[y].v_keywords.size();f++)
					{
						if(strstr(query_words.GetBuffer(query_words.GetLength()),p_pkv->v_projects[i].m_supply_keywords.v_killwords[y].v_keywords[f])!=NULL)
						{
							found_killword = true;
							return;
						}
					}
				}
					for(y=0;y<p_pkv->v_projects[i].m_supply_keywords.v_keywords.size();y++)
					{
						matches = 0;
						for(f=0;f<p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords.size();f++)
						{
							//TRACE("CHECK %d: %s\n",y,p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords[f]);
							if(strstr(full_string.GetBuffer(full_string.GetLength()),p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords[f])!=NULL)
							{
								//TRACE("match: %s",p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords[f]);
								matches++;	//increment matches
							}
						}
						if(matches==p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords.size())
							{
								unique_track = true;
								//TRACE("MATCH FOUND");
								break;
							}
					}
						if(y>p_pkv->v_projects[i].m_supply_keywords.v_keywords.size())
						{
							y=0;
						}
						//found a match, exit for loop
						//send spoof for current track
						//if(matches==p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords.size() && p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].v_keywords.size()!=0)
						if(unique_track)
						{
							//p_pkv->v_projects[i].m_project_name;
							//TRACE("QUERY RECEIVED: %s\n", query_words);
							//TRACE("MATCH FOUND! Project: %sTrack: %s\n",p_pkv->v_projects[i].m_project_name.c_str(),p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].m_track_name.c_str());
							m_db->ExtractData(p_pkv->v_projects[i].m_project_name.c_str(),p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].m_track_name.c_str(),"direct_connect_supply",v_spoofs);
							//do multiple spoofing here, return a vector of $sr
							for(UINT spoof_pos=0;spoof_pos<v_spoofs.size();spoof_pos++)
							{
								memset(SR,0,sizeof(SR));	//reset the char buffer
								//memset(buffer,0,sizeof(buffer));
								char* ptr_send= NULL;//(int*)&SR[0];
								char* ptr2_send = NULL;
								ptr2_send=&SR[0];
								strcpy(SR,"$SR ");
								strcat(SR,v_spoofs[spoof_pos].m_user.GetBuffer(0));
								strcat(SR," ");
								strcat(SR,this->GetSpoofPath().c_str());
								strcat(SR,v_spoofs[spoof_pos].m_filename.GetBuffer(0));	//file path
								ptr2_send += strlen(SR);
								ptr_send=ptr2_send;
								*ptr_send = 0x5;
								ptr2_send+=1;
								strcat(SR,itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10));
								strcat(SR," 4/6");	//filesize
								//memset(buffer,0,sizeof(buffer));
								ptr2_send += strlen(itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10))+4;	//include 1/6
								ptr_send=ptr2_send;
								*ptr_send =0x05;
								ptr2_send+=1;
								strcat(SR," (");
								strcat(SR,"1.1.1.1.");	//made constant
								strcat(SR,":411)");
								strcpy(ip_address,extract_ip);
								strcpy(port,extract_port);
								string struc_active;
								struc_active=SR;
								v_sr.push_back(struc_active);
							}
							m_db->SingleTrackDataExists(p_pkv->v_projects[i].m_project_name.c_str(),p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].m_track);
							m_db->DataExists(p_pkv->v_projects[i].m_project_name.c_str());
							m_db->IncrementDBSpoofs(p_pkv->v_projects[i].m_project_name.c_str(),(int)v_spoofs.size());
							m_db->IncrementDBRequests(p_pkv->v_projects[i].m_project_name.c_str(),1,p_pkv->v_projects[i].m_supply_keywords.v_keywords[y].m_track);
							return;
						}
						//send spoof for entire project
						else
						{
							//TRACE("SEND SPOOF\n");
							m_db->ExtractData(p_pkv->v_projects[i].m_project_name.c_str(),"","direct_connect_supply",v_spoofs);
							//do multiple spoofing here, return a vector of $sr
							if(p_pkv->v_projects[i].m_directconnect_decoys_enabled)
							{
								for(UINT spoof_pos=0;spoof_pos<v_spoofs.size();spoof_pos++)
								{
									memset(SR,0,sizeof(SR));	//reset the char buffer
									//memset(buffer,0,sizeof(buffer));
									char *ptr_send= NULL;//(int*)&SR[0];
									char *ptr2_send = NULL;
									strcpy(SR,"$SR ");
									ptr2_send=&SR[0];
									//strcat(SR,v_spoofs[spoof_pos].m_user.GetBuffer(0));
									//Raphael strcat(SR,"big_swapperr");
									strcat(SR,this->m_nick);
									//strcat(SR," dc\\shared files\\");
									strcat(SR," ");
									strcat(SR,this->GetSpoofPath().c_str());
									strcat(SR,v_spoofs[spoof_pos].m_filename.GetBuffer(0));	//file path
									ptr2_send += strlen(SR);
									ptr_send=ptr2_send;
									*ptr_send = 0x5;
									ptr2_send+=1;
									//char file_size[100]="12345688 3/3";
									//strcat(SR,itoa(v_spoofs[i].m_filesize,buffer,10));
									strcat(SR,itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10));
									strcat(SR," 3/6");	//filesize
									//memset(buffer,0,sizeof(buffer));
									ptr2_send += strlen(itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10))+4;	//include 3/3
									ptr_send=ptr2_send;
									*ptr_send =0x05;
									ptr2_send+=1;
									strcat(SR,this->m_hubname);		//hub name
									strcat(SR,"(");
									strcat(SR,hub_ip);		//hub ip
									strcat(SR,":");
									strcat(SR,"411");		//hub port
									//strcat(SR,"411");
									strcat(SR,")");
									//strcat(SR,v_spoofs[spoof_pos].ip.GetBuffer(0));
									//strcat(SR,":411)");
									strcpy(ip_address,extract_ip);
									strcpy(port,extract_port);
									string struc_active;
									//struc_active.ip_address=extract_ip;
									//struc_active.port=extract_port;
									struc_active=SR;
									v_sr.push_back(struc_active);
								}
								//TRACE("DONE SETTING DECOYS\n");
							}	//decoys enabled end if
								for(UINT spoof_pos=0;spoof_pos<v_spoofs.size();spoof_pos++)
								{
								//memset(SR,0,sizeof(SR));	//reset the char buffer
								//memset(buffer,0,sizeof(buffer));
								int* ptr_send= NULL;//(int*)&SR[0];
								char* ptr2_send=&SR[0];
								strcpy(SR,"$SR ");
								//strcat(SR,v_spoofs[spoof_pos].m_user.GetBuffer(0));
								//Raphael strcat(SR,"big_swapperr");
								strcat(SR,v_spoofs[spoof_pos].m_user.GetBuffer(0));
								//strcat(SR," dc\\shared files\\");
								strcat(SR," ");
								strcat(SR,this->GetSpoofPath().c_str());
								strcat(SR,v_spoofs[spoof_pos].m_filename.GetBuffer(0));	//file path
								ptr2_send += strlen(SR);
								ptr_send=(int*)ptr2_send;
								*ptr_send = 0x5;
								ptr2_send+=1;
								//char file_size[100]="12345688 3/3";
								//strcat(SR,itoa(v_spoofs[i].m_filesize,buffer,10));
								strcat(SR,itoa(v_spoofs[spoof_pos].m_filesize,buffer,10));
								strcat(SR," 3/3");	//filesize
								//memset(buffer,0,sizeof(buffer));
								ptr2_send += strlen(itoa(this->SetFileSize(v_spoofs[spoof_pos].m_filesize),buffer,10))+4;	//include 3/3
								ptr_send=(int*)ptr2_send;
								*ptr_send =0x05;
								ptr2_send+=1;
								strcat(SR,this->m_hubname);		//hub name
								strcat(SR,"(");
								strcat(SR,hub_ip);		//hub ip
								strcat(SR,":");
								strcat(SR,"411");		//hub port
								//strcat(SR,"411");
								strcat(SR,")");
								//strcat(SR,v_spoofs[spoof_pos].ip.GetBuffer(0));
								//strcat(SR,":411)");
								strcpy(ip_address,extract_ip);
								strcpy(port,extract_port);
								string struc_active;
								//struc_active.ip_address=extract_ip;
								//struc_active.port=extract_port;
								struc_active=SR;
								v_sr.push_back(struc_active);
								}
								//TRACE("DONE SETTING SPOOFS\n");
								
								m_db->DataExists(p_pkv->v_projects[i].m_project_name.c_str());
								m_db->IncrementDBSpoofs(p_pkv->v_projects[i].m_project_name.c_str(),(int)v_spoofs.size());
								m_db->IncrementDBRequests(p_pkv->v_projects[i].m_project_name.c_str(),1,0);
								m_db->IncrementDBDecoys(p_pkv->v_projects[i].m_project_name.c_str(),(int)v_spoofs.size());
								//TRACE("EXITING...\n");
								return;
						}
					
				//else there is a specific track being searched for

				//break;
			}
			//no match reset weight
			else
			{
				weight = 0;
			}
		}	//if directconnect project
	}
	return;

}
void DCCommands::GetDBConnection(DB *db)
{
	m_db = db;	//pass connection pointer
}

//
//
bool DCCommands::SendConnectToMe(char* data,char* str_connect)
{
	//memset(Connect,0,sizeof(Connect));
	strcpy(Connect,"");
	strcat(Connect,"$ConnectToMe");
	CString temp=data;
	CString ip_address_temp;
	CString ip_address;
	int position = temp.Find("|",0);
	if(position<0)
		return false;
	temp=temp.Left(position);
	temp.Replace("|","");
	//memset(ip_address,0,sizeof(ip_address));	//clear contents of ip_address
	ip_address=temp.GetBuffer(temp.GetLength());
	//char* tok=strtok(ip_address," ");
	//while(tok!=NULL)
	//{
	//	ip_address=tok;
//		tok=strtok(NULL," ");
//	}
	int cur_pos=0;
	ip_address_temp=temp.Tokenize("' '",cur_pos);
	while(ip_address_temp!="")
	{
		ip_address=ip_address_temp;
		ip_address_temp=temp.Tokenize("' '",cur_pos);
	}
	//raphael strcat(Connect," big_swapperr ");
	strcat(Connect," ");
	strcat(Connect,this->m_nick);
	strcat(Connect," ");
	strcat(Connect,ip_address);
	strcat(Connect,"|");
	char* send_Connect=&Connect[0];
	strcpy(str_connect,Connect);
	return true;
}

//creates a false file path
string DCCommands::GetSpoofPath()
{
	
//	srand(timeGetTime());	//random seed
	m_path = "";
	m_folders = rand() % 5;
	m_folders += 1;
	for(int i=0;i<m_folders;i++)
	{
		m_combo = rand() % 16 ;
		switch(m_combo)
		{
			case 0:
				m_path += "download\\";
				break;
			case 1:
				m_path += "shared\\";
				break;
			case 2:
				m_path += "dc\\";
				break;
			case 3:
				m_path += "my music\\";
				break;
			case 4:
				m_path += "library\\";
				break;
			case 5:
				m_path += "shared folder\\";
				break;
			case 6:
				m_path += "Documents and Settings\\";
				break;
			case 7:
				m_path += "Incoming\\";
				break;
			case 8:
				m_path += "shared stuff\\";
				break;
			case 9:
				m_path += "mp3\\";
				break;
			case 10:
				m_path += "backup\\";
				break;
			case 11:
				m_path += "MP3\\";
				break;
			case 12:
				m_path += "full\\";
				break;
			case 13:
				m_path += "complete\\";
				break;
			case 14:
				m_path += "full albums\\";
				break;
			case 15:
				m_path += "others\\";
				break;
		}
	}
	return m_path;
}

//
//checks whether we currently have spoof data for the project being spoof'd
bool DCCommands::ProjectExists(string project, vector<SpoofCollector> temp)
{
	bool found_project = false;	//if project is found return true
	for(UINT i = 0; i< temp.size(); i++)
	{
		if(strcmp(project.c_str(),temp[i].m_project.c_str()) == 0)
		{
			found_project = true;
			break;	//we don't need more processing, exit from for loop
		}
	}

	return found_project;
}

//set the current hub name to the commands object so when it spoofs and decoys it has
//the correct hub name
void DCCommands::SetHubName(char hub[200])
{
	strcpy(m_hubname,hub);
	strcat(m_hubname," ");
}

void DCCommands::SetHubIpAddress(long ip)
{
	in_addr temp;
	temp.S_un.S_addr = ip;
	hub_ip = inet_ntoa(temp);

}
//create a bogus filesize that is divisible by 137
int DCCommands::SetFileSize(int filesize)
{
	m_difference = filesize%137;
	filesize-=m_difference;
	return filesize;
}
void DCCommands::SetNick(char* nick)
{
	strcpy(m_nick,nick);
}

char* DCCommands::GetNick()
{
	return m_nick;
}