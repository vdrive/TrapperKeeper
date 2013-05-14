#include "StdAfx.h"
#include "kadfile.h"
#include "KadFileKey.h"

KadFile::KadFile(const char* name,byte* hash,UINT size)
{
	m_name=name;
	m_hash.setValueBE(hash);
	m_size=size;

	CreateKeywords();
}

KadFile::~KadFile(void)
{
}

void KadFile::CreateKeywords(void)
{
	int len=(int)m_name.size();

	const char* str=m_name.c_str();

	string cur_keyword;
	for(int i=0;i<len;i++){
		if(IsWordBreakChar(str[i])){
			if(cur_keyword.size()>1){
				if(stricmp(cur_keyword.c_str(),"its")==0){  //not alternate flagged, but the word "its" is in there.  we want to also add "it's"
					AddKeyword("it's");
				}
				else
					AddKeyword(cur_keyword.c_str());
			}
			cur_keyword.clear();
		}
		else{ 
			cur_keyword+=tolower(str[i]);
		}
	}

	if(cur_keyword.size()>1){
		AddKeyword(cur_keyword.c_str());
	}
}

void KadFile::AddKeyword(const char* str)
{
	if(stricmp(str,"mp3")==0)
		return;
	KadFileKey *nfk=new KadFileKey(str);
	for(int i=0;i<(int)mv_keywords.Size();i++){
		KadFileKey *kfk=(KadFileKey*)mv_keywords.Get(i);
		if(stricmp(kfk->m_name.c_str(),str)==0){
			delete nfk;
			return;
		}
	}

	//TRACE("Adding keyword: %s\n",str);
	mv_keywords.Add(nfk);
}

bool KadFile::IsWordBreakChar(char ch)
{
	if(ch<0 || ch>127)
		return true;

	if(!isalpha(ch) && !isdigit(ch))
		return true;

	return false;

	/*
	switch(ch){
		case ' ': return true;
		case ',': return true;
		case '`': return true;
		case '~': return true;
		case '!': return true;
		case '@': return true;
		case '#': return true;
		case '$': return true;
		case '%': return true;
		case '^': return true;
		case '&': return true;
		case '*': return true;
		case '(': return true;
		case ')': return true;
		case '_': return true;
		case '-': return true;
		case '+': return true;
		case '=': return true;
		case '[': return true;
		case '{': return true;
		case ']': return true;
		case '}': return true;
		case '}': return true;
		case '\'': return true;
		case '"': return true;
		case ':': return true;
		case ';': return true;
		case ',': return true;
		case '.': return true;
		case '<': return true;
		case '>': return true;
		case '/': return true;
		case '?': return true;
		default: return false;
	}
	return false;*/
}
