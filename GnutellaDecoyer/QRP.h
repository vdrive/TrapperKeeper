// QRP.h

//
// These functions were taken from the QRP document at http://www.limewire.com/developer/query_routing/keyword%20routing.htm
//
#pragma once

class QRP
{
public:
	QRP();
	~QRP();

	static void AddUpTo3KeywordsToTable(const char *keyword,byte *table);

private:
	static int Hash(const char *buf,unsigned char bits);
	static int Hash(const char *buf,int start,int end,unsigned char bits);
	static int HashFast(int x,byte bits);

	static void AddKeywordToTable(const char *keyword,byte *table);
	static void ExtractKeywordsFromQuery(vector<string> *keywords, const char *query);
	static void ClearNonAlphaNumeric(char *buf,int buf_len);


};