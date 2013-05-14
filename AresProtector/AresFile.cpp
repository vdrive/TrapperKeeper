#include "StdAfx.h"
#include ".\aresfile.h"
//#include "AresProtectionSystem.h"
//#include "aresfile.h"


AresFile::AresFile()
{
}

AresFile::AresFile(byte* hash,const char* artist, const char* title, const char* file_name, const char* project, unsigned int size, int media_length,bool b_trace,bool b_swarm)
{
	Init(hash,artist,title,file_name,project,size,media_length,b_trace,b_swarm);
}

void AresFile::Init(byte* hash,const char* artist, const char* title, const char* file_name, const char* project, unsigned int size, int media_length,bool b_trace,bool b_swarm){
	mb_valid=true;
	if(hash==NULL){
		for(int i=0;i<20;i++)
			m_hash[i]=rand()&255;
	}
	else{
		for(int i=0;i<20;i++)
			m_hash[i]=hash[i];  //yes memcpy is faster, but i think it breaks if source and dest are same buffer.  not sure but did this to be safe.
	}

	if(!b_swarm)
		*(UINT*)(m_hash+2)=size;  //make 4 bytes within the hash the size of the file so we can extract it upon request

	m_media_length=media_length;
	m_meta_artist=artist;

	m_meta_title=title;
	m_file_name=file_name;
	const char *debug_ptr=m_meta_title.c_str();
	m_project=project;
	m_size=size;

	CalculateExtension();

	if(m_meta_title.size()==0)
		m_meta_title=GetExtensionlessFileName();

	m_keywords_length=0; 

	m_type=0x01;

	if(stricmp(m_file_extension.c_str(),".avi")==0 || stricmp(m_file_extension.c_str(),".mpg")==0 || stricmp(m_file_extension.c_str(),".wmv")==0){
		m_type=0x05;

		//if its a movie set the artist to be something stupid since movie artists typically say something stupid
		int tmp=rand()%13;
		//m_meta_artist="specialmovie";
		
		if(m_meta_artist.size()!=0)
			m_meta_artist+=" ";
		switch(tmp){
			case 0: m_meta_artist+="Movies"; break;
			case 1: m_meta_artist+="DVD Rip"; break;
			case 2: m_meta_artist+="dvd.rip"; break;
			case 3: m_meta_artist+="tmd"; break;
			case 4: m_meta_artist+="DivX"; break;
			case 5: m_meta_artist+="ShareConnector"; break;
			case 6: m_meta_artist+="XviD"; break;
			case 7: m_meta_artist+="XviD-DoNE"; break;
			case 8: m_meta_artist+="dvdrip xvid"; break;
			case 9: m_meta_artist+="Video"; break;
			case 10: m_meta_artist+="SVCD TS"; break;
			default:m_meta_artist+="Movie"; break;
		}

	}
	
	CreateKeywords(b_trace);
}

AresFile::~AresFile(void)
{
}

void AresFile::CalculateExtension()
{
	const char *file_name=m_file_name.c_str();
	int len=(int)m_file_name.size();
	m_file_extension.clear();
	for(int i=len-1;i>=0;i--){
		if(file_name[i]!='.'){
			string tmp;
			tmp+=tolower(file_name[i]);
			tmp+=m_file_extension;
			m_file_extension=tmp;
		}
		else{
			string tmp=".";
			tmp+=m_file_extension;
			m_file_extension=tmp;
			return;
		}
	}
}


void AresFile::CreateKeywords(bool b_trace)
{
	mv_keyword_artist.clear();
	mv_keyword_title.clear();
	m_keywords_length=0;

	ExtractKeywords(m_meta_artist.c_str(),mv_keyword_artist,b_trace);
	ExtractKeywords(m_meta_title.c_str(),mv_keyword_title,b_trace);

	if(b_trace){
		for(int i=0;i<(int)mv_keyword_artist.size();i++){
			TRACE("kw_artist = %s\n",mv_keyword_artist[i].c_str());
		}
		for(int i=0;i<(int)mv_keyword_title.size();i++){
			TRACE("kw_title = %s\n",mv_keyword_title[i].c_str());
		}
	}
	/*
	const char *artist=m_meta_artist.c_str();

	int len=(int)m_meta_artist.size();

	string cur_keyword;
	for(int i=0;i<len;i++){
		if(!isalpha(artist[i])){
			if(cur_keyword.size()>1){
				mv_keyword_artist.push_back(cur_keyword.c_str());
				//TRACE("Added artist keyword %s\n",cur_keyword.c_str());
				m_keywords_length+=(int)cur_keyword.size();
			}
			cur_keyword.clear();
		}
		else cur_keyword+=tolower(artist[i]);
	}

	if(cur_keyword.size()>1){
		mv_keyword_artist.push_back(cur_keyword.c_str());
		//TRACE("Added artist keyword %s\n",cur_keyword.c_str());
		m_keywords_length+=(int)cur_keyword.size();
	}
	cur_keyword.clear();
	*/


	/*
	const char *title=m_meta_title.c_str();

	len=(int)m_meta_title.size();

	for(int i=0;i<len;i++){
		if(!isalpha(title[i])){
			if(cur_keyword.size()>1){
				mv_keyword_title.push_back(cur_keyword.c_str());
				//TRACE("Added keyword %s\n",cur_keyword.c_str());
				m_keywords_length+=(int)cur_keyword.size();
				cur_keyword.clear();
			}
			cur_keyword.clear();
		}
		else cur_keyword+=tolower(title[i]);
	}

	if(cur_keyword.size()>1){
		mv_keyword_title.push_back(cur_keyword.c_str());
		//TRACE("Added keyword %s\n",cur_keyword.c_str());
		m_keywords_length+=(int)cur_keyword.size();
	}
	cur_keyword.clear();*/
}

string AresFile::GetExtensionlessFileName(void)
{
	string tmp_file_name;
	int tmp_len=(int)m_file_name.size()-(int)m_file_extension.size();

	for(int i=0;i<tmp_len;i++){
		tmp_file_name+=m_file_name[i];
	}

	return tmp_file_name;
}

void AresFile::ExtractKeywords(const char* str,vector <string> &v_keywords,bool b_trace)
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

bool AresFile::IsValidSpecialChar(char ch)
{
	if(ch!='.' && ch!='&' && ch!=',' && ch!='(' && ch!=')' && ch!='[' && ch!=']' && ch!='_' && ch!=' ' && ch!='.' && ch!='\'' && ch!='-'){
		return false;
	}
	else return true;
}

void AresFile::AddKeyword(vector <string>&v, string& str)
{
//	if(str[str.size()-1]=='\'')
//		return;
	if(str.size()==0)
		return;
	for(int i=0;i<(int)v.size();i++){  //check if it is already a keyword
		if(stricmp(v[i].c_str(),str.c_str())==0){
			return;
		}
	}

	v.push_back(str);
	m_keywords_length+=(int)str.size();
}
