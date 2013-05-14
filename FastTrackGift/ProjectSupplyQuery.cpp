// ProjectSupplyQuery.cpp

#include "stdafx.h"
#include "ProjectSupplyQuery.h"
#include "ProjectKeywords.h"

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
	m_search_id = psq.m_search_id;

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

	m_query_keywords=psq.m_query_keywords;
	m_project_status_index=psq.m_project_status_index;
	m_search_type = psq.m_search_type;
	m_project_id=psq.m_project_id;

// =>_<= 	m_is_searcher_query=psq.m_is_searcher_query;
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
	UINT i;
	
	m_project.erase();
	m_search_id=0;

	m_search_string.erase();
	m_supply_size_threshold=0;
	m_query_keywords.Clear();

	// Free keywords
	v_keywords.clear();

	// Free killwords
	for(i=0;i<v_killwords.size();i++)
	{
		delete [] v_killwords[i];
	}
	v_killwords.clear();

	m_project_status_index=0;
	m_search_type=ProjectKeywords::search_type::everything;
	m_project_id=0;
}

//
//
//
bool  ProjectSupplyQuery::operator < (const ProjectSupplyQuery& other)const
{
	if(this->m_search_id < other.m_search_id)
		return true;
	else
		return false;
}

//
//
//
bool  ProjectSupplyQuery::operator < (const unsigned short& other)const
{
	if(this->m_search_id < other)
		return true;
	else
		return false;
}

//
//
//
ProjectSupplyQuery& ProjectSupplyQuery::operator=(const ProjectSupplyQuery &psq)
{
	UINT i;

	Clear();

	m_project=psq.m_project;
	m_search_id=psq.m_search_id;

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

	m_project_status_index=psq.m_project_status_index;
	m_search_type=psq.m_search_type;
	m_project_id=psq.m_project_id;
	m_query_keywords = psq.m_query_keywords;
	return *this;
}

//
//
//
void ProjectSupplyQuery::ExtractProjectKeywordData(ProjectKeywords *keywords)
{
	UINT i,j;

	// Save the m_search_id and Clear the object and then restore the m_search_id
	unsigned int search_id = m_search_id;
	unsigned int project_id = m_project_id;
	Clear();
	m_search_id=search_id;
	m_project_id = project_id;

	m_project=keywords->m_project_name;

	//remove mp3 in search string
	char* temp_str = new char[keywords->m_supply_keywords.m_search_string.length()+1];
	strcpy(temp_str, keywords->m_supply_keywords.m_search_string.c_str());
	char* tmp = strstr(temp_str, " mp3");
	if(tmp!=NULL)
		tmp[0]='\0';
	m_search_string=temp_str;
	delete [] temp_str;

	m_supply_size_threshold=keywords->m_supply_keywords.m_supply_size_threshold;

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