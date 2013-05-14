#pragma once
#include <vector>
using namespace std;

//class that represents xml document
class XMLSTRUCT
{
public:
	XMLSTRUCT(void);
	~XMLSTRUCT(void);
	vector<CString> m_ip;	//ip of the cstring to be used for computing ranges
	vector<CString> m_tags;	//vector of tags, <NAME> tags within an <ENTRY NAME="">
	CString entry_name;	//name of entry

	string m_one_ip;	//a single ip stored as a string
	vector<string> m_vector_ip;	//a vector of ip addresses stored as strings
	int m_int_ip;		//ip represented as ip
};
