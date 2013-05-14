// Project.h

#ifndef PROJECT_H
#define PROJECT_H

#include <vector>
using namespace std;

class Project
{
public:
	Project();
	void Clear();

	string m_name;
	string m_query;
	unsigned int m_min_size;

	// These are the database specifications:
	CString m_ip;
	CString m_login;
	CString m_pass;
	CString m_db;
	CString m_table;

	enum SearchType
	{
		Everything,
		Audio,
		Video,
		Images,
		Documents,
		Software,
		Playlists
	};

	SearchType m_search_type;

	enum SearchOption
	{
		None,
		Artist
	};

	SearchOption m_search_option;

	vector<string> v_killwords;
	vector<string> v_required_keywords;

	bool m_find_more;
//	bool m_is_mpaa_project;
//	bool m_is_sony_project;
};

#endif // PROJECT_H