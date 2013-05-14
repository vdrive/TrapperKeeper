// QueryKeywords.h

#ifndef QUERY_KEYWORDS_H
#define QUERY_KEYWORDS_H

#include "ProjectDataStructure.h"
#include "QueryKeyword.h"

class QueryKeywords : public ProjectDataStructure
{
public:
	
	// Public Member Functions
	QueryKeywords();
	~QueryKeywords();

	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	// Public Data Members
	vector<QueryKeyword> v_keywords;
	vector<QueryKeyword> v_exact_keywords;
	vector<QueryKeyword> v_killwords;
	vector<QueryKeyword> v_exact_killwords;
};

#endif // QUERY_KEYWORDS_H