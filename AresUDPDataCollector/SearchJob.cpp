#include "StdAfx.h"
#include "searchjob.h"

UINT search_id=0;

SearchJob::SearchJob(UINT proj_id,const char* artist, const char* album, UINT track_num, const char* track_name, const char* search_str,UINT size_threashold)
{
	mb_valid=true;
	m_project_id=proj_id;
	m_search_id=search_id++;
	m_artist=artist;
	m_album=album;
	m_track_num=track_num;
	m_track_name=track_name;
	m_search_str=search_str;
	ExtractKeywords(search_str,mv_search_keywords);

	m_size_threashold=size_threashold;
}

SearchJob::~SearchJob(void)
{
}

void SearchJob::ExtractKeywords(const char* str,vector <string> &v_keywords)
{
	int len=(int)strlen(str);

	string cur_keyword;
	string alternate_keyword;
	bool b_use_alternate=false;
	for(int i=0;i<len;i++){
		if(str[i]<0){
			mb_valid=false;
			return;
		}
		if(!isalpha(str[i]) && !isdigit(str[i]) && !IsValidSpecialChar(str[i])){
			mb_valid=false;
			return;
		}
		if(!isalpha((int)str[i]) && !isdigit(str[i])){
			if(str[i]=='\''){
				b_use_alternate=true;  //we need to use the alternate keyword we are building, as well as the keyword without the apostrophe so we handle both search cases
				alternate_keyword+=tolower(str[i]);
				continue;  //skip apostrophes
			}
			if(cur_keyword.size()>1){
				//v_keywords.push_back(cur_keyword.c_str());
				//if(b_trace)
				//	TRACE("  keyword %s\n",cur_keyword.c_str());

				AddKeyword(v_keywords,cur_keyword);
				//m_keywords_length+=(int)cur_keyword.size();

				if(b_use_alternate){
					//if(b_trace)
					//	TRACE("  alt_keyword %s\n",alternate_keyword.c_str());

					AddKeyword(v_keywords,alternate_keyword);
					//m_keywords_length+=(int)alternate_keyword.size();
					//v_keywords.push_back(alternate_keyword.c_str());
				}
				else if(stricmp(cur_keyword.c_str(),"its")==0){  //not alternate flagged, but the word "its" is in there.  we want to also add "it's"
					string tmp("it's");
					
					AddKeyword(v_keywords,tmp);
					//v_keywords.push_back("it's");
					//m_keywords_length+=(int)strlen("it's");
				}
			}
			cur_keyword.clear();
			alternate_keyword.clear();
			b_use_alternate=false;
		}
		else{ 
			cur_keyword+=tolower(str[i]);
			alternate_keyword+=tolower(str[i]);
		}
	}

	if(cur_keyword.size()>1){
		//if(b_trace)
		//	TRACE("  keyword %s\n",cur_keyword.c_str());

		AddKeyword(v_keywords,cur_keyword);
		//v_keywords.push_back(cur_keyword.c_str());
		//m_keywords_length+=(int)cur_keyword.size();

		if(b_use_alternate){
			//if(b_trace)
			//	TRACE("  alt_keyword %s\n",alternate_keyword.c_str());

			AddKeyword(v_keywords,alternate_keyword);
			//m_keywords_length+=(int)alternate_keyword.size();
			//v_keywords.push_back(alternate_keyword.c_str());
		}
		//TRACE("Added artist keyword %s\n",cur_keyword.c_str());
	}

	cur_keyword.clear();
}
void SearchJob::AddKillword(const char* killword)
{
	mv_killwords.push_back(killword);
}
