#include "StdAfx.h"
#include "aressnspoof.h"

AresSNSpoof::AresSNSpoof(const char* project, UINT track, const char* filename, const char* title, const char* artist, 
								   const char* album, const char* genre, const char* comments, UINT bitrate, UINT media_length, UINT size,UINT popularity)
{
	m_project=project;
	m_track=track;
	m_filename=filename;
	m_title=title;
	m_artist=artist;
	m_album=album;
	m_genre=genre;
	m_comments=comments;
	m_bitrate=bitrate;
	m_media_length=media_length;
	m_size=size;
	m_popularity=popularity;

	CalculateExtension();
	CreateKeywords();


	memset(m_clump_hash,0,20);

	//in order to clump spoofs, lets pregenerate a hash based on title and file name
	//set it to be the file name
	for(int j=0;j<(int)m_filename.size();j++){
		byte b=(byte)(m_filename.c_str()[j]);
		m_clump_hash[j%20]=b;
	}

	//add the title to the hash
	for(int j=0;j<(int)m_title.size();j++){
		byte b=((int)m_clump_hash[j%20]+(int)(m_title.c_str()[j]))%255;
		m_clump_hash[j%20]=b;
	}

	//add the project to the hash
	for(int j=0;j<(int)m_project.size();j++){
		byte b=((int)m_clump_hash[j%20]+(int)(m_project.c_str()[j]))%255;
		m_clump_hash[j%20]=b;
	}

	//add the file size to the hash
	*(UINT*)(m_clump_hash+14)=m_size;

	m_search_type=65000;

	if(stricmp(m_file_extension.c_str(),".avi")==0 || stricmp(m_file_extension.c_str(),".mpg")==0
		|| stricmp(m_file_extension.c_str(),".wmv")==0){

		m_search_type=3;
	}
	else if(stricmp(m_file_extension.c_str(),".mp3")==0 || stricmp(m_file_extension.c_str(),".wma")==0){

		m_search_type=1;
	}

	m_resolution1=0;
	m_resolution2=0;

	if(m_search_type==3){
		int res_pick=m_clump_hash[5]%7;
		if(res_pick==0 || res_pick==1){
			m_resolution1=352;
			m_resolution2=240;
		}
		else if(res_pick==2){
			m_resolution1=720;
			m_resolution2=480;
			m_category="science fiction & fantasy";
		}
		else if(res_pick==3){
			m_resolution1=320;
			m_resolution2=240;
			m_category="comedy";
		}	
		else if(res_pick==4){
			m_resolution1=576;
			m_resolution2=320;
			m_category="action & adventure";
		}	
		else if(res_pick==5){
			m_resolution1=592;
			m_resolution2=384;
		}	
		else if(res_pick==6){
			m_resolution1=320;
			m_resolution2=240;
			m_category="humour";
		}	


		int cat_pick=m_clump_hash[6]%6;
		if(cat_pick==0){
			m_category="science fiction & fantasy";
		}
		else if(cat_pick==1){
			m_category="humour";
		}	
		else if(cat_pick==2){
			m_category="action & adventure";
		}	
		else if(cat_pick==3){
			m_category="comedy";
		}	

		//skip 3 and 4 because we want to leave it blank usually, we just don't want people assuming fakes always have a blank category


		if(m_clump_hash[7]&1)
			m_language="english";

		UINT random_val=*(UINT*)(m_clump_hash+5);
		m_media_length=5600+(random_val%5600); //between 1.5 and 3 hours long
	}

	
	mb_valid=true;
}

AresSNSpoof::~AresSNSpoof(void)
{
}

void AresSNSpoof::AddKeyword(vector <string>&v, string& str)
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
}


void AresSNSpoof::ExtractKeywords(const char* str,vector <string> &v_keywords)
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

void AresSNSpoof::CreateKeywords()
{
	mv_keywords.clear();
	ExtractKeywords(m_artist.c_str(),mv_keywords);
	ExtractKeywords(m_title.c_str(),mv_keywords);
	ExtractKeywords(m_filename.c_str(),mv_keywords);
	ExtractKeywords(m_album.c_str(),mv_keywords);

	bool b_trace=false;
	if(b_trace){
		TRACE("File = %s\n",m_filename.c_str());
		for(int i=0;i<(int)mv_keywords.size();i++){
			TRACE("  kw = %s\n",mv_keywords[i].c_str());
		}
		TRACE("\n",m_filename.c_str());
	}
}

void AresSNSpoof::CalculateExtension()
{
	const char *file_name=m_filename.c_str();
	int len=(int)m_filename.size();
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

bool AresSNSpoof::IsValidSpecialChar(char ch)
{
	if(ch!='.' && ch!='&' && ch!=',' && ch!='(' && ch!=')' && ch!='[' && ch!=']' && ch!='_' && ch!=' ' && ch!='.' && ch!='\'' && ch!='-'){
		return false;
	}
	else return true;
}
bool AresSNSpoof::HasKeyword(const char* keyword)
{
	for(int i=0;i<(int)mv_keywords.size();i++){  //check if it is already a keyword
		if(stricmp(mv_keywords[i].c_str(),keyword)==0){
			return true;
		}
	}
	return false;

}
