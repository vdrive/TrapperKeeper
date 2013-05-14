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

	static void AddUpTo3KeywordsToTable(char *keyword,char *table);

private:
	static int Hash(char *buf,unsigned char bits);
	static int Hash(char *buf,int start,int end,unsigned char bits);
	static int HashFast(int x,byte bits);

	static void AddKeywordToTable(char *keyword,char *table);
};