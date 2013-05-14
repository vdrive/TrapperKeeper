// QueryID.h

#ifndef QUERY_ID_H
#define QUERY_ID_H

class QueryID
{
public:
	QueryID();
	~QueryID();
	void Clear();

	unsigned int m_ip;
	unsigned int m_count;
};

#endif // QUERY_ID_H