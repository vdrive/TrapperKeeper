#include "peerlist.h"
#include "bencode.h"
#include "stdafx.h"

bool debug = false;

PeerList::PeerList(void)
{
	next = NULL;
	b_isSeed = false;
	m_ip = "";
}

PeerList::PeerList(const PeerList &list)
{
	next = NULL;
	b_isSeed = false;
	m_ip = "";
	operator=(list);	
}


PeerList::PeerList(string peers, string peer_id)
{
	next = NULL;
	b_isSeed = false;
	m_connect_id = peer_id;
	unsigned short * temp = new unsigned short[peers.length()+1];
	for(unsigned int i = 0; i < peers.length(); i ++) {
		temp[i] = peers[i];
	}
	temp[peers.length()] = 0;
	ParseInput(temp, 0, (int)peers.length());
	delete [] temp;
}

PeerList::PeerList(unsigned short * peers, int len, string peer_id, string title)
{
	next = NULL;
	b_isSeed = false;
	m_connect_id = peer_id;
	m_title = title;
	ParseInput(peers, 0, len);
}

PeerList::PeerList(unsigned short * peers, int pos, int len, string peer_id)
{
	next = NULL;
	b_isSeed = false;
	m_connect_id = peer_id;
	ParseInput(peers, pos, len);
}

PeerList::PeerList(string ip, string port, string peer_id) {
	next = NULL;
	m_ip = ip;
	m_port = port;
	b_isSeed = false;
	m_connect_id = peer_id;
}


PeerList::~PeerList(void)
{
	
	if(next != NULL) delete next;
	next = NULL;
}


PeerList & PeerList::operator=(const PeerList &list) 
{
	m_title = list.m_title;
	m_ip = list.m_ip;
	m_port = list.m_port;
	m_connect_id = list.m_connect_id;
	memcpy(m_peer_id, list.m_peer_id, 20);
	if(list.next != NULL) {
		next = new PeerList();
		*next = *list.next;
	}
	return *this;
}

bool PeerList::operator==(const PeerList &list) 
{
	if(m_ip.compare(list.m_ip) == 0 && m_port.compare(list.m_port) == 0) {
		if(next != NULL && list.next != NULL) {
			return *next == *list.next;
		}
		else 
			return next==list.next;
	}
	else 
		return false;
}

bool PeerList::operator!=(const PeerList &list) 
{
	return !operator==(list);
}

string PeerList::GetTitle()
{
	return m_title;
}

string PeerList::GetID()
{
	return m_connect_id;
}


bool PeerList::IsSeed()
{
	return b_isSeed;
}

void PeerList::IsSeed(bool _isSeed)
{
	b_isSeed = _isSeed;
}

string PeerList::GetPort()
{
	return m_port;
}


PeerList * PeerList::GetNext() 
{
	return next;
}


void PeerList::Add(PeerList * _next)
{
	if(next == NULL) {
		next = _next;
	}
	else next->Add(_next);
}



string PeerList::GetIP()
{
	return m_ip;
}

int PeerList::GetCount() {
	if (this == NULL || m_ip.length() == 0)
	{
		return 0;
	}
	if(next != NULL) return 1+next->GetCount();
	else return 1;
}

string PeerList::ToString() {
	return ToString(true);
}

string PeerList::ToString(bool isFirst)
{
	string str = "";
	if(isFirst) str += "PeerList::[my peer_id:" + m_connect_id+"][";
	str += PrintNode();
	if(next != NULL) str += ", " + next->ToString(false);
	if(isFirst) str += "]";
	return str;
}

string PeerList::PrintNode() 
{
	string str = "";
	if(m_ip != "") str += m_ip;
	if(m_port != "") str += ":" + m_port;
	return str;
}

void PeerList::ParseInput(unsigned short * peers, int pos, int len)
{
	
 	if(pos == 0 && (Find(peers, pos, len, "d8") == 0 || Find(peers, pos, len, "d10") == 0)) {
		int start_index = Find(peers, pos, len, "5:peers");
		if(peers[start_index+7] == 'l') start_index = start_index + 9;
		pos = (int)start_index;
	} 


	while(true) {
		string key = GetValue(peers, &pos);
		//TRACE("key: %s\n", key.c_str());
		if(key == "ip") m_ip = GetValue(peers, &pos);
		else if(key == "peer id") {
			pos += 3;
			for(int i = 0; i < 20; i++) {
				m_peer_id[i] = peers[pos+i];
			}
			pos += 20;
		}
		else if(key == "port") m_port = GetValue(peers, &pos);
		else if(key == "peers" && peers[pos] != 'l') {
			if(peers[pos] == 'e') break;
			int peers_len = GetLength(peers, &pos);
			int peers_end_pos = pos + peers_len;
			if(peers_len > 0) {
				m_ip = UnCompactIP(peers, &pos);
				m_port = UnCompactPort(peers, &pos);
			}
			while(pos < peers_end_pos) {
				string ip = UnCompactIP(peers, &pos);
				string port = UnCompactPort(peers, &pos);
				Add(new PeerList(ip, port, m_connect_id));
			}
		}
		if(key == "break" || peers[pos] == 'e' || peers[pos] == 'l') break;
	}

	int index = Find(peers, pos, len, "2:ip");
	if(index  > 0) {
		Add(new PeerList(peers, (int)index, (int)len, m_connect_id));
	}
}

int PeerList::Find(const unsigned short * peers, int start, int len, char *find_str) 
{
	int find_len = (int)strlen(find_str);
	int index = -1;
	for(int i = start; i < len-find_len; i++) {
		if(peers[i] == find_str[0]) {
			index = i;
			for(int y = 0; y < find_len; y++) {
				if(peers[i] != find_str[y]) {
					index = -1;
					break;
				}
				i++;
			}
			if(index >= 0) return index;
		}
	}
	return -1;
}

/*******************
	Each peer takes 6 bytes in the string and is in the following format:
	192.168.24.52:2001 => 0xC0 0xA8 0x18 0x34 0xD1 0x07
	Each octet of the IP address is one byte. The port (16-bit value) takes two, 
	and is in network-byte order (big-endian)
********************/
string PeerList::UnCompactIP(unsigned short *data, int *pos)
{
	int cur_pos = *pos;
	char ip[16];
	char temp[4];

	strcpy(ip, "");
	strcat(ip, itoa((unsigned char)data[cur_pos++], temp, 10));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)data[cur_pos++], temp, 10));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)data[cur_pos++], temp, 10));
	strcat(ip, ".");
	strcat(ip, itoa((unsigned char)data[cur_pos++], temp, 10));
	*pos = cur_pos;
	return ip;
}

/*******************
	Each peer takes 6 bytes in the string and is in the following format:
	192.168.24.52:2001 => 0xC0 0xA8 0x18 0x34 0xD1 0x07
	Each octet of the IP address is one byte. The port (16-bit value) takes two, 
	and is in network-byte order (big-endian)
********************/
string PeerList::UnCompactPort(unsigned short *data, int *pos) 
{
	string retVal = "";
	int cur_pos = *pos;
	unsigned char port[2];
	port[0] = (unsigned char)data[cur_pos+1];
	port[1] = (unsigned char)data[cur_pos];
	unsigned short port_val[1];
	memcpy(port_val, port, 2);
	char temp[16];
	itoa((int)port_val[0], temp, 10);
	retVal = temp;
	cur_pos += 2;
	*pos = cur_pos;
	return retVal;
}

int PeerList::GetLength(unsigned short *data, int*pos) 
{
	int cur_pos = *pos;
	int size = 0;
	char * lenStr = new char[16];
	if(data[cur_pos] == 'd') {
		cur_pos++;
	}
	else if (!isdigit(data[cur_pos])) {
		cout << cur_pos << endl;
	}
	for(int i = cur_pos; data[i] != ':' && size < 16; i++) {
		lenStr[i-cur_pos] = (char)data[i];
		size++;
	}
	if(debug) cout << "size: " << size << " ";
	if(size >= 15) {
		delete [] lenStr;
		return -1;
	}
	lenStr[size] = 0;
	int retVal = atoi(lenStr);

	//TRACE("LEN: ");
	//TRACE(lenStr);
	//TRACE("\n");

	delete [] lenStr;
	cur_pos += size;
	cur_pos++;
	*pos = cur_pos;
	return retVal;
}

string PeerList::ConvertToString(unsigned short * source, int start, int len)
{
	string retStr = "";
	char * value = new char[len+1];
	for(int i = 0; i < len; i++) {
		value[i] = (char)source[start + i];
	}
	value[len] = 0;
	if(debug) cout << value << endl;
	retStr = value;
	delete [] value;
	return retStr;
}

string PeerList::GetValue(unsigned short *data, int*pos)
{
	int cur_pos = *pos;
	string retStr = "";
	if(data[cur_pos] == 'i') {
		int len = 0;
		for(int i = cur_pos+1; data[i] != 'e'; i++) {
			len++;
		}
		retStr = ConvertToString(data, cur_pos+1, len);
		cur_pos += len + 2;
	}
	else {
		int len = GetLength(data, &cur_pos);
		if(len == -1) return "break";
		if(debug) cout << len << ":";
		retStr = ConvertToString(data, cur_pos, len);
		if(debug && retStr.length() != len) {
			cout << "len != str.len -- ";
			cout << retStr << endl;
			for(int i = 0; i < len ; i++) {
				cout << (char)data[i+cur_pos + 1];
			}
			cout << endl;
			cout << &data[cur_pos + 1 + len];
		}
		if(retStr == "exbc") {
			char * temp = new char[20];
			int start = cur_pos+1;
			if(debug) cout << &data[start+20];
			strcpy(temp, "BitComet");
			strcat(temp, (data[start+4]+"."));
			strcat(temp, data[start+5]/10+"0");
			strcat(temp, data[start+5]%10+"0");
			char * str = new char[13];
			for(int i = 0; i < 12; i++) {
				str[i] = temp[i];
			}
			str[12] = 0;
			retStr = str;
			delete [] str;
			if(debug) {
				for(i = 0; i < 20; i++) {
					cout << (unsigned char)data[i+start];
				}
				cout << endl;
			}
			delete [] temp;
		}
		cur_pos += len;
	}
	*pos = cur_pos;
	
	//TRACE("value: ");
	//TRACE(retStr.c_str());
	//TRACE("\n");

	return retStr;
}

void PeerList::PrintList(unsigned short * data, int start, int len) 
{
	if(!debug) return;
	string temp = (char*)&data[start];
	cout << endl << "****** start: " <<  start << " ****** len: " << len << " ******" << endl;
	for(int i = start; i < len && i < 3000+start; i++) {
		if(i%5 == 0) cout << i;
		else if(i < 10 || !((i > 10 && i%5 == 1) || (i > 100 && i%5 == 2) || (i > 1000 && i%5 == 3))) cout << " ";
	}
	cout << endl;
	//cout << temp << endl;;
	for(int i = start; i < len; i++) {
		cout << "=";
	}
	cout << endl;
	for(int i = start; i < len; i++) {
		cout << (signed char)data[i];
	}
	cout <<  endl << "*******" << endl;
}

int PeerList::GetSize() 
{
	int size = 0;
	//size += (int)m_title.size()+1;
	size += (int)m_ip.size()+1;
	size += (int)m_port.size()+1;
	size += (int)20*sizeof(unsigned short);
	size += (int)m_connect_id.size()+1;
	size += sizeof b_isSeed;
	return size;
}

void PeerList::GetBuffer(char *buffer) 
{
	char * temp = GetBuffer();
	memcpy(buffer, temp, GetSize());
	delete [] temp;
}

char * PeerList::GetBuffer() 
{
	int size = GetSize();
	char * buffer = new char[size];
	int cur_pos = 0;

	//memcpy(&buffer[cur_pos], m_title.c_str(), m_title.size());
	//buffer[cur_pos+m_title.size()] = 0;
	//cur_pos += (int)m_title.size()+1;

	memcpy(&buffer[cur_pos], m_ip.c_str(), m_ip.size());
	buffer[cur_pos+m_ip.size()] = 0;
	cur_pos += (int)m_ip.size()+1;

	memcpy(&buffer[cur_pos], m_port.c_str(), m_port.size());
	buffer[cur_pos+m_port.size()] = 0;
	cur_pos += (int)m_port.size()+1;

	memcpy(&buffer[cur_pos], m_peer_id,20*sizeof(unsigned short));
	cur_pos += (int)20*sizeof(unsigned short);

	memcpy(&buffer[cur_pos], m_connect_id.c_str(), m_connect_id.size());
	buffer[cur_pos+m_connect_id.size()] = 0;
	cur_pos += (int)m_connect_id.size()+1;

	buffer[cur_pos] = b_isSeed;
	cur_pos += sizeof b_isSeed;
	//cout << "PeerList::size: " << size << " cur_pos: " << cur_pos  << endl;
	return buffer;
}

PeerList::PeerList(char *buffer, int *pos) 
{

	next = NULL;
	int cur_pos = 0;

	//m_title = &buffer[cur_pos];
	//cur_pos += (int)m_title.size()+1;

	m_ip = &buffer[cur_pos];
	cur_pos += (int)m_ip.size()+1;

	m_port = &buffer[cur_pos];
	cur_pos += (int)m_port.size()+1;

	memcpy(m_peer_id, &buffer[cur_pos], 20*sizeof(unsigned short));
	cur_pos += (int)20*sizeof(unsigned short);

	m_connect_id = &buffer[cur_pos];
	cur_pos += (int)m_connect_id.size()+1;

	b_isSeed = buffer[cur_pos] == 1;
	cur_pos += sizeof b_isSeed;
	
	*pos+=cur_pos;

}