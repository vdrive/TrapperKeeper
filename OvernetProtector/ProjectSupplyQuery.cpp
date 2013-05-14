// ProjectSupplyQuery.cpp

#include "stdafx.h"
#include "ProjectSupplyQuery.h"
#include "..\DCMaster\ProjectKeywords.h"

//
//
//
ProjectSupplyQuery::ProjectSupplyQuery()
{
	Clear();
}

//
// Copy constructor
//
ProjectSupplyQuery::ProjectSupplyQuery(const ProjectSupplyQuery &psq)
{
	UINT i;

	m_project=psq.m_project;

	m_search_string=psq.m_search_string;
	m_supply_size_threshold=psq.m_supply_size_threshold;

	// Copy keywords
	for(i=0;i<psq.v_keywords.size();i++)
	{
		v_keywords.push_back(psq.v_keywords[i]);
	}

	// Copy killwords
	for(i=0;i<psq.v_killwords.size();i++)
	{
		char *ptr=new char[strlen(psq.v_killwords[i])+1];
		strcpy(ptr,psq.v_killwords[i]);
		v_killwords.push_back(ptr);
	}

	//m_project_status_index=psq.m_project_status_index;
	m_project_id=psq.m_project_id;
	memcpy(m_keyword_hash,psq.m_keyword_hash,16);
	v_search_words = psq.v_search_words;
	m_search_type = psq.m_search_type;
	m_query_keywords=psq.m_query_keywords;
	m_track=psq.m_track;

}

//
//
//
ProjectSupplyQuery::~ProjectSupplyQuery()
{
	UINT i;

	// Free killwords
	for(i=0;i<v_killwords.size();i++)
	{
		delete [] v_killwords[i];
	}
	v_killwords.clear();
}

//
//
//
void ProjectSupplyQuery::Clear()
{
	memset(&m_keyword_hash,0,16);
	
	m_project.erase();

	m_search_string.erase();
	m_supply_size_threshold=0;

	// Free keywords
	v_keywords.clear();

	// Free killwords
	for(UINT i=0;i<v_killwords.size();i++)
	{
		delete [] v_killwords[i];
	}
	v_killwords.clear();
	v_search_words.clear();

	//m_project_status_index=0;
	m_project_id=0;
	m_search_type.Empty();
	m_query_keywords.Clear();
	m_track=0;
}

//
//
//
ProjectSupplyQuery& ProjectSupplyQuery::operator=(const ProjectSupplyQuery &psq)
{
	UINT i;

	Clear();

	m_project=psq.m_project;

	m_search_string=psq.m_search_string;
	m_supply_size_threshold=psq.m_supply_size_threshold;

	for(i=0;i<psq.v_keywords.size();i++)
	{
		v_keywords.push_back(psq.v_keywords[i]);
	}

	for(i=0;i<psq.v_killwords.size();i++)
	{
		char *ptr=new char[strlen(psq.v_killwords[i])+1];
		strcpy(ptr,psq.v_killwords[i]);
		v_killwords.push_back(ptr);
	}

	//m_project_status_index=psq.m_project_status_index;
	m_project_id=psq.m_project_id;
	memcpy(m_keyword_hash,psq.m_keyword_hash,16);
	v_search_words = psq.v_search_words;
	m_search_type = psq.m_search_type;
	m_query_keywords = psq.m_query_keywords;
	m_track=psq.m_track;

	return *this;
}

//
//
//
void ProjectSupplyQuery::ExtractProjectKeywordData(ProjectKeywords *keywords/*, int track*/)
{
	UINT i,j;
	/*
	if(track==-1)
		track=m_track;
		*/
	Clear();

	//m_track=track;
	m_project=keywords->m_project_name;
	m_project_id=keywords->m_id;
	m_search_string=keywords->m_supply_keywords.m_search_string;
	m_supply_size_threshold=keywords->m_supply_keywords.m_supply_size_threshold;
	if(keywords->m_search_type == ProjectKeywords::audio)
		m_search_type="audio";
	else if(keywords->m_search_type == ProjectKeywords::video)
		m_search_type="video";
	else if(keywords->m_search_type == ProjectKeywords::software)
		m_search_type="pro";

	// Extract the keywords
	for(i=0;i<keywords->m_supply_keywords.v_keywords.size();i++)
	{
		ProjectSupplyQueryKeywords psqk;
		psqk.m_track=keywords->m_supply_keywords.v_keywords[i].m_track;

		// Extract out the required keywords from the required keyword string
//		ExtractKeywordsFromKeywordString((char *)keywords->m_supply_keywords.v_keywords[i].keyword.c_str(),psqk.v_keywords);
		for(j=0;j<keywords->m_supply_keywords.v_keywords[i].v_keywords.size();j++)
		{
			char *ptr=new char[strlen(keywords->m_supply_keywords.v_keywords[i].v_keywords[j])+1];
			strcpy(ptr,keywords->m_supply_keywords.v_keywords[i].v_keywords[j]);
			psqk.v_keywords.push_back(ptr);
		}

		v_keywords.push_back(psqk);
	}

	// Extract the killwords
	for(i=0;i<keywords->m_supply_keywords.v_killwords.size();i++)
	{
		char *ptr=new char[strlen(keywords->m_supply_keywords.v_killwords[i].v_keywords[0])+1];	// kinda kludgey *&*
		strcpy(ptr,keywords->m_supply_keywords.v_killwords[i].v_keywords[0]);	// kinda kludgey *&*
		v_killwords.push_back(ptr);
	}


	CString search_string = m_search_string.c_str();
	/*
	if(track >= 0)
	{
		if(track < keywords->m_supply_keywords.v_keywords.size())
		{
			search_string += " ";
			search_string += keywords->m_supply_keywords.v_keywords[track].m_track_name.c_str();
		}
	}
	*/

	ExtractKeywords(search_string,&v_search_words);

	for(i=0;i<v_search_words.size();i++)
	{
		if(v_search_words[i].GetLength()>2 && stricmp(v_search_words[i],"the")!=0 )
		{
			break;
		}
		else
		{
			v_search_words.erase(v_search_words.begin()+i);
			i--;
		}
	}
	m_query_keywords = keywords->m_query_keywords;
}

//
//
//
void ProjectSupplyQuery::ExtractProjectKeywordTrackData(ProjectKeywords *keywords, int track)
{
	UINT i,j;
	/*
	if(track==-1)
		track=m_track;
		*/
	Clear();

	m_track=track;
	m_project=keywords->m_project_name;
	m_project_id=keywords->m_id;
	m_search_string=keywords->m_supply_keywords.m_search_string;
	m_supply_size_threshold=keywords->m_supply_keywords.m_supply_size_threshold;
	if(keywords->m_search_type == ProjectKeywords::audio)
		m_search_type="audio";
	else if(keywords->m_search_type == ProjectKeywords::video)
		m_search_type="video";
	else if(keywords->m_search_type == ProjectKeywords::software)
		m_search_type="pro";

	// Extract the keywords
	for(i=0;i<keywords->m_supply_keywords.v_keywords.size();i++)
	{
		ProjectSupplyQueryKeywords psqk;
		psqk.m_track=keywords->m_supply_keywords.v_keywords[i].m_track;

		// Extract out the required keywords from the required keyword string
//		ExtractKeywordsFromKeywordString((char *)keywords->m_supply_keywords.v_keywords[i].keyword.c_str(),psqk.v_keywords);
		for(j=0;j<keywords->m_supply_keywords.v_keywords[i].v_keywords.size();j++)
		{
			char *ptr=new char[strlen(keywords->m_supply_keywords.v_keywords[i].v_keywords[j])+1];
			strcpy(ptr,keywords->m_supply_keywords.v_keywords[i].v_keywords[j]);
			psqk.v_keywords.push_back(ptr);
		}

		v_keywords.push_back(psqk);
	}

	// Extract the killwords
	for(i=0;i<keywords->m_supply_keywords.v_killwords.size();i++)
	{
		char *ptr=new char[strlen(keywords->m_supply_keywords.v_killwords[i].v_keywords[0])+1];	// kinda kludgey *&*
		strcpy(ptr,keywords->m_supply_keywords.v_killwords[i].v_keywords[0]);	// kinda kludgey *&*
		v_killwords.push_back(ptr);
	}


	CString search_string;
	
	if(track >= 0)
	{
		if(track < keywords->m_supply_keywords.v_keywords.size())
		{
			search_string += keywords->m_supply_keywords.v_keywords[track].m_track_name.c_str();
			search_string += " ";
			search_string += m_search_string.c_str();
		}
	}
	

	ExtractKeywords(search_string,&v_search_words);

	for(i=0;i<v_search_words.size();i++)
	{
		if(v_search_words[i].GetLength()>2 && stricmp(v_search_words[i],"the")!=0)
		{
			break;
		}
		else
		{
			v_search_words.erase(v_search_words.begin()+i);
			i--;
		}
	}
	m_query_keywords = keywords->m_query_keywords;
}

//
//
//
void ProjectSupplyQuery::ExtractKeywordsFromKeywordString(char *keyword_string,vector<char *> &keywords)
{
	UINT i;

	// Make the string lowercase (although it already should be)
	char *lbuf=new char[strlen(keyword_string)+1];
	strcpy(lbuf,keyword_string);
	strlwr(lbuf);

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
		for(i=0;i<keywords.size();i++)
		{
			if(strcmp(keywords[i],ptr)==0)
			{
				found=true;
				break;
			}
		}
		if(!found)	// if not, then add it
		{
			char *tmp_ptr=new char[strlen(ptr)+1];
			strcpy(tmp_ptr,ptr);
			keywords.push_back(tmp_ptr);
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
void ProjectSupplyQuery::ExtractKeywords(const char* artist, vector<CString>* keywords)
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
	RemoveWord(cstring," mp3");

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
			if(strcmp((*keywords)[i],ptr)==0)
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
//
//
void ProjectSupplyQuery::RemoveWord(CString& string, const char* word)
{
	int index = string.Find(word);
	if(index>-1)
		string.Delete(index,strlen(word));
}

//
// Replaces all characters that are not letters or numbers with spaces in place
//
void ProjectSupplyQuery::ClearNonAlphaNumeric(char *buf,int buf_len)
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