#include "StdAfx.h"
#include "searchresult.h"

UINT SearchResult::sm_search_result_count=0;
UINT report_count=0;

SearchResult::SearchResult(void)
{
	sm_search_result_count++;  //delete me
	
}

SearchResult::~SearchResult(void)
{
	sm_search_result_count--;  //delete me
	report_count++;
	if(report_count%400==0){  //every 500 search results that are created, report how many remain undestroyed.
		TRACE("SearchResult destroyed, %d remaining.\n",sm_search_result_count);  //delete me
	}
}

void SearchResult::Clear(void)
{
	m_file_name.clear();
	ZeroMemory(m_hash,16);
	m_size=0;
//	mv_ips.Clear();
	m_search_string.clear();
}

/*void SearchResult::AddSource(UINT ip)
{	
//	TRACE("MetaMachine Service:  Adding Source %d.%d.%d.%d to file %s with hash %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x.\n",
//		((ip)	& 255),
//		((ip>>8)	& 255),
//		((ip>>16)	& 255),
//		((ip>>24)		& 255),
//		m_file_name.c_str(),
//		m_hash[0],m_hash[1],m_hash[2],m_hash[3],m_hash[4],m_hash[5],m_hash[6],m_hash[7],m_hash[8],m_hash[9],m_hash[10],m_hash[11],m_hash[12],m_hash[13],m_hash[14],m_hash[15]);
	mv_ips.Add(new SuperInt(ip));
}*/

bool SearchResult::IsHash(byte* hash)
{
	if(memcmp(hash,m_hash,16)==0){
		return true;
	}
	else return false;
}

void SearchResult::Dump(void)
{
	TRACE("MetaMachine Service:  SearchResult:\nSearch String: %s\nFile: %s\nSize: %d\nHash: %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n\n",
		m_search_string.c_str(),
		m_file_name.c_str(),
		m_size,
		m_hash[0],m_hash[1],m_hash[2],m_hash[3],m_hash[4],m_hash[5],m_hash[6],m_hash[7],m_hash[8],m_hash[9],m_hash[10],m_hash[11],m_hash[12],m_hash[13],m_hash[14],m_hash[15]);
}
