#pragma once
#include <vector>
#include <fstream>
#include "xmlstruct.h"
#include <algorithm>
using namespace std;

//class that parses an xml document and computes ranges of ip addresses as well as tag inheritance
class XMlParser
{
public:
	XMlParser(void);
	~XMlParser(void);
	void ReadXMLFile();						//reads local xml type file
	vector<string> GetVector(CString tag);	//returns vector of ips
	vector<CString> GetTags();				//returns all the tags
	//vector<int>::iterator ip_result_iter;	//iterator for v_ip_result
	int GetIntIPFromStringIP(string ip);	//convert the integer to a string copied, courtesy of Ivan
	string GetIPStringFromIPInt(int ip_int);	//convert the int to a string, courtesy of Ivan
	vector<string> GetIPDestVector();			//gets all the dest ips read in from file
	vector<string> GetIPSourceVector();			//gets all the source ips read in from file
	
private:
	bool InfiniteLoop(CString check_tag);	//checks if an infinite loop will be created
	bool CheckDuplicate(string check_duplicate);	//checks if there are duplicate ips have been included
	void ComputeIPRange();					//computes the range of ip addresses
	void UniqueTags(CString tag);			//searches if tag is already in list
	void ResolveNames(CString entry_name);	//does recursion when <NAME> tags are 

	vector<XMLSTRUCT> v_rack_data;			//holds permanent data about tags and ips
	vector<XMLSTRUCT> v_temp_data;			//hold temporary data that will be transferred to v_rack_data
	vector<CString> v_tag_vector;			//holds all the unique tags(<NAME>) in a vector
	vector<CString> v_tags_vector;			//holds tags used for recursion search
	vector<string> v_ip_result;				//holds the result of ips from the tag search
	vector<int> v_search_ip;					//holds ip address converted to int for binary search
	vector<CString> v_infinite_check;		//uses vector to detect circular references
	//vector<CString> m_duplicate_check;		//uses vector to detect duplicate ip addresses in a vector
	int m_ip_int;							//integer form of the ip address
	bool m_found;							// if the ip was found by the binary search
	vector<int>::iterator m_location;		//iterator for binary search function
	vector<XMLSTRUCT> v_report;				//will hold up to date vector list
	vector<string> v_dest;				//holds all the destination ips declared in file
	vector<string> v_source;			//holds all the source ips declared in file
};
