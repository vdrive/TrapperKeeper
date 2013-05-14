#include "StdAfx.h"
#include "mysql.h"
#include ".\searchinfo.h"



// protected constructor, use static GetSearchInfo to get a link list
SearchInfo::SearchInfo(void)
{
	next = NULL;
}

SearchInfo::SearchInfo(SearchInfo &si)
{
	next = NULL;
	operator=(si);
}

SearchInfo::~SearchInfo(void)
{
	if(next != NULL) delete next;
	next = NULL;
}

SearchInfo & SearchInfo::operator =(SearchInfo &si)
{
	m_type = si.m_type;
	m_project = si.m_type;

	int size = (int)si.v_keywords.size();
	for(int i = 0; i < size; i++) {
		v_keywords.push_back(si.v_keywords[i]);
	}
	if(si.next != NULL) {
		next = new SearchInfo(*si.next);
	}
	else next = NULL;

	return *this;
}


// returns a vector of this projects keywords
vector<string> SearchInfo::GetKeywords() 
{
	return v_keywords;
}

 // One of four search types
 // AUDIO
 // EVERYTHING
 // SOFTWARE
 // VIDEO
string SearchInfo::GetType()
{
	return m_type;
}


 // Project name 
string SearchInfo::GetProject()
{
	return m_project;
}


void SearchInfo::SetKeywords(vector<string> keywords)
{
	v_keywords = keywords;
}

void SearchInfo::SetKeywords(string keywords) 
{
	v_keywords.clear();
	int start = 0;
	int index = (int)keywords.find(" ");
	while(index > 0) {
		v_keywords.push_back(keywords.substr(start, index - start));
		start = index+1;
		index = (int)keywords.find(" ", start);
	}
	v_keywords.push_back(keywords.substr(start));

}

void SearchInfo::SetType(string type)
{
	m_type = type;
}

void SearchInfo::SetProject(string project) 
{
	m_project = project;
}

SearchInfo * SearchInfo::GetNext()
{
	return next;
}


void SearchInfo::Add(SearchInfo * si) 
{
	if(next != NULL) next->Add(si);
	else next = si;
}

SearchInfo * SearchInfo::GetSearchInfo() 
{
	string address = "38.118.160.161";
	string user = "onsystems";
	string password = "ebertsux37";
	string database = "project_management";
	string table = "projects";
	SearchInfo * retVal = NULL;

	MYSQL *conn = new MYSQL();
	MYSQL_RES *res = NULL;
	string query = "SELECT \
		search_string, \
		search_type, \
		project_name \
		FROM projects \
		where active = 'A' \
		and owner not like 'test%' \
		order by artist	";

	mysql_init(conn); 
	if (mysql_real_connect(conn,address.c_str(),user.c_str(),password.c_str(),database.c_str(),0,NULL,0) == NULL)
	{
		TRACE("(Failed to connect to database: %s", mysql_error(conn));
	}
	else
	{
		int ret = mysql_query(conn, query.c_str());
		if (ret != 0)	// check for error
			TRACE(mysql_error(conn));

		MYSQL_ROW row;
		res = mysql_store_result(conn); // allocates mem (::)

		if(res != NULL)
		{
			int num_rows = (int)mysql_num_rows(res);

			if(num_rows > 0)
			{
				// Iterate through the rowset
				while ((row = mysql_fetch_row(res)))
				{
					if(row[0] != NULL && row[1] != NULL)
					{
						string search_type = row[1];
						string search_keywords = row[0];
						string project_name = (row[2] == NULL) ? "" : row[2];
						SearchInfo * si = new SearchInfo();
						si->SetType(search_type);
						si->SetKeywords(search_keywords);
						si->SetProject(project_name);
						if(retVal == NULL) {
							retVal = si;
						}
						else {	
							retVal->Add(si);
						}
					}
				}
			}
		}
		mysql_free_result(res);			// free's mem (::)
		mysql_close(conn);
		delete conn;
	}
	return retVal;	
}