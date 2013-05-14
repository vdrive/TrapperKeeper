#pragma once

#include <iostream>
#include <string>
#include "stdafx.h"


class PeerList
{
public:
	PeerList(void);
	PeerList(const PeerList & list);
	PeerList(char *data, int *len);
	PeerList(string peers, string peer_id);
	PeerList(unsigned short * peers, int len, string peer_id, string title); 
	PeerList(unsigned short * peers, int pos, int len, string peer_id);
	PeerList(string ip, string port, string peer_id);
	~PeerList(void);
	void Add(PeerList*peer);  // adds peer to the end of the link list.
	string GetIP(); // ip of this peer
	string GetPort(); // port for this peer
	string GetTitle(); // title as listed on the web page 
	string GetID(); // peer id we used to connect to the tracker
	int GetCount();  // returns total number of peers in list
	string ToString(); 
	string PrintNode();
	PeerList * GetNext();
	void IsSeed(bool isSeed); // set true if this is a seed
	bool IsSeed(); // true if this is a seed, false otherwise
	int GetSize();
	void GetBuffer(char *buffer);
	PeerList & operator=(const PeerList &list);
	bool operator == (const PeerList &list);
	bool operator != (const PeerList &list);
protected:
	char * GetBuffer();
private:
	int GetLength(unsigned short *data, int*pos);
	string UnCompactIP(unsigned short *data, int*pos);
	string UnCompactPort(unsigned short *data, int*pos);
	string ToString(bool isFirst);
	string GetValue(unsigned short * data, int*pos);
	void ParseInput(unsigned short * input, int pos, int len);
	string ConvertToString(unsigned short * source, int start, int len);
	int Find(const unsigned short * source, int start, int len, char *find);

	PeerList * next;
	string m_title;
	string m_ip;
	string m_port;
	string m_connect_id;
	unsigned short m_peer_id[20];
	bool b_isSeed;

	void PrintList(unsigned short * list, int pos, int len);
};
