#include <iostream>
#include "bencode.h"
#include "torrentfile.h"
#include "afx.h"
#include "TorrentFileHeader.h"

TorrentFile::TorrentFile(void)
{
	m_info_hash_string = "";
	strcpy(m_url, "");
	strcpy(m_name, "");
	m_creationdate = 0;
	m_length  = 0;
	m_piecelength = 0;
	m_numpieces = 0;
	m_size = 0;
	m_data = NULL;
	m_peers = NULL;
	info_len = 0;
	//m_info_hash[0] = 0;
	ZeroMemory(&m_info_hash, 20*sizeof(unsigned short));
	b_is_valid = false;
}

TorrentFile::TorrentFile(const TorrentFile & tf) 
{
	m_info_hash_string = "";
	strcpy(m_url, "");
	m_creationdate = 0;
	m_length  = 0;
	m_piecelength = 0;
	m_numpieces = 0;
	m_size = 0;
	m_data = NULL;
	m_peers = NULL;
	info_len = 0;
	ZeroMemory(&m_info_hash, 20*sizeof(unsigned short));
	//m_info_hash[0] = 0;
	operator=(tf);
}

TorrentFile::~TorrentFile(void)
{
	for (int i = 0; i < (int)v_pieces.size(); i ++) {
		if(v_pieces[i] != NULL) delete [] v_pieces[i];
		v_pieces[i] = NULL;
	}

	v_announce_list.clear();
	v_pieces.clear();
	if(m_data != NULL) {
		delete [] m_data;
		m_data = NULL;
		m_size = 0;
	}
	if(m_peers != NULL) {
		delete m_peers;
		m_peers = NULL;
	}
}



TorrentFile & TorrentFile::operator=(const TorrentFile & tf)
{
	memcpy(m_url, tf.m_url, 256);
	m_creationdate = tf.m_creationdate;
	m_length = tf.m_length;
	memcpy(m_name, tf.m_name, 256);
	m_piecelength = tf.m_piecelength;
	m_numpieces = tf.m_numpieces;
	info_len = tf.info_len;
	info_start = tf.info_start;
	m_info_hash_string = tf.m_info_hash_string;
	memcpy(m_info_hash, tf.m_info_hash, 20*sizeof(unsigned short));
	m_size = tf.m_size;
	if(m_size > 0 && tf.m_data != NULL) {
		m_data = new unsigned short[m_size];
		memcpy(m_data, tf.m_data, m_size);
	}
	else
	{
		m_data = NULL;
	}
	int size = (int)tf.v_announce_list.size();
	for(int i = 0; i < size; i++) {
		v_announce_list.push_back(tf.v_announce_list[i]);
	}
	size = (int)tf.v_pieces.size();
	for(int i = 0; i < size; i++) {
		unsigned short * temp = new unsigned short[20];
		memcpy(temp, tf.v_pieces[i], 20*sizeof(unsigned short));
		v_pieces.push_back(temp);
	}
	if(tf.m_peers == NULL) {
		m_peers = NULL;
	}
	else {
		m_peers = new PeerList(*tf.m_peers);
	}	
	b_is_valid = tf.b_is_valid;
	return *this;
}

bool TorrentFile::operator==(const TorrentFile &tf) 
{
	//	PeerList * m_peers;
	if(strcmp(m_url, tf.m_url) != 0) return false;
	if(m_creationdate != tf.m_creationdate) return false;
	if(m_length != tf.m_length) return false;
	if(strcmp(m_name, tf.m_name) != 0)  return false;
	if(m_piecelength != tf.m_piecelength) return false;
	if(m_numpieces != tf.m_numpieces) return false;
	if(info_len != tf.info_len) return false;
	if(info_start != tf.info_start) return false;
	if(m_info_hash_string != tf.m_info_hash_string) return false;
	if(m_size != tf.m_size) return false;
	if(m_peers != tf.m_peers) return false;
	for(int i = 0; i < 20; i++) {
		if(m_info_hash[i] != tf.m_info_hash[i]) return false;
	}
	int size = (int)v_announce_list.size();
	if(size != (int)tf.v_announce_list.size()) return false;
	for(int i = 0; i < size; i++) {
		if(v_announce_list[i] != tf.v_announce_list[i]) return false;
	}
	size = (int)v_pieces.size();
	if(size != (int)tf.v_pieces.size()) return false;
	for(int i = 0; i < size; i++) {
		if(v_pieces[i] != tf.v_pieces[i]) return false;
	}
	if(m_data == NULL && tf.m_data == NULL) return true;
	if(m_data == NULL || tf.m_data == NULL) return false;
	for(int i = 0; i < m_size; i++) {
		if(m_data[i] != tf.m_data[i]) return false;
	}
	if(b_is_valid != tf.b_is_valid) return false;
	return true;
}

bool TorrentFile::IsValid()
{
	if(m_name[0] == 0 || m_url[0] == 0) return false;
	return(b_is_valid);
}
int TorrentFile::GetPeerCount() 
{
	if(m_peers == NULL) return 0;
	else return m_peers->GetCount();
}

string TorrentFile::GetName()
{
	return m_name;
}

void TorrentFile::ReadInTorrentFile(char * filename)
{

	CFile t_file;
//	int tempint;
	
	if(t_file.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return;
	}
	int filelength = (int)t_file.GetLength();
	char * buf = new char[filelength];
	memset(buf,0,filelength);
	t_file.Read(buf,filelength);
	unsigned short * shortbuf = new unsigned short[filelength];
	for (int i=0; i<filelength; i++)
	{
		shortbuf[i] = buf[i];
	}
	ParseTorrentFile(shortbuf, filelength);
	delete[]buf;
	delete[]shortbuf;
	t_file.Close();
}


string TorrentFile::ParseNext(unsigned short * buf, int * pos) {
	int cur_pos = *pos;
	unsigned short type = buf[cur_pos];
	while(type > 0 && type < 255 && !isdigit(type)) {
		if(type == 'i') {
			int i = DecodeInt(buf, &cur_pos);
		}
		else (cur_pos)++;
		type = buf[cur_pos];
	}
	if(type < 0) {
		//WriteFile(buf, m_size);
	}
	*pos = cur_pos;
	if(cur_pos > m_size) return "break";
	return DecodeString(buf,pos);	
}

vector<string> TorrentFile::GetAnnounceList() 
{
	if(v_announce_list.size() == 0) v_announce_list.push_back(GetURL());
	return v_announce_list;
}

void TorrentFile::SetPeers(PeerList * peers) 
{
	m_peers = peers;
}

PeerList * TorrentFile::GetPeers() 
{
	return m_peers;
}

bool TorrentFile::ParseTorrentFile(unsigned short * buf, int size) 
{
	string tempstring;
	int pos = 1; 
	if(buf[0] != 'd') {
		tempstring = "";
		strcpy(m_url,tempstring.c_str());
		return false;
	}
	int len = size;
	if(m_data == NULL) m_data = new unsigned short[len+1];
	memcpy(m_data, buf, len);
	for(int i = 0; i < len; i++) {
		if(m_data[i] != buf[i]) {
			m_data[i] = buf[i];
		}
	}	
	m_data[len] = 0;
	m_size = size;
	
	info_start = 0;
	int info_end = 0;
	bool havePieces = false;
	while(true) {
		tempstring = ParseNext(buf, &pos);
		// The URL
		if(tempstring.compare("announce") == 0) {
			tempstring = DecodeString(buf, &pos);
			strcpy(m_url,tempstring.c_str());
		}
		else if(tempstring.compare("announce-list") == 0) {
			AtomList al = DecodeList(buf, &pos);
			v_announce_list = al.v_elements;
		}
		else if(tempstring.compare("info") == 0) {
			info_start = pos;
		}
		// Creation date
		else if(tempstring.compare("creation date") == 0) {
			m_creationdate = DecodeInt(buf,&pos);
		}
		// Length
		else if(tempstring.compare("length") == 0) {
			m_length += DecodeInt(buf,&pos);
		}
		// Name
		else if(tempstring.compare("name") == 0) {
			tempstring = DecodeString(buf, &pos);
			strcpy(m_name,tempstring.c_str());
		}
		// Piece Length
		else if(tempstring.compare("piece length") == 0) {
			m_piecelength = DecodeInt(buf,&pos);
		} 
		// Pieces 
		else if(tempstring.compare("pieces") == 0) {
			v_pieces = DecodePieces(buf, &pos);
			m_numpieces = (int)v_pieces.size();
			havePieces = true;
		}
		if(havePieces && (buf[pos] == 'e') || tempstring.compare("break") == 0) {
			info_end = pos;
			info_len = info_end-info_start+1;
			break;
		}
	}
	b_is_valid = true;
	return true;
}



string TorrentFile::GetURL()
{
	return (m_url);
}

unsigned short * TorrentFile::GetInfoHash() 
{
	bool b_hash_init = false;
	for(int i = 0; i < 20; i++) {
		if(m_info_hash[i] != 0) {
			b_hash_init	= true;
			break;
		}
	}
	//if(m_info_hash[0] == NULL) {
	if(!b_hash_init) {
		CreateInfoHash();
	}
	return m_info_hash;
}


string TorrentFile::GetInfoHashString()
{
	if(m_info_hash_string == "") {
		CreateInfoHash();
	}
	return m_info_hash_string;
}

void TorrentFile::CreateInfoHash() {
	if(info_len == 0) return;
	CSHA1 sha;

	unsigned char * info  = new unsigned char[info_len];
	for(int i = 0; i < info_len; i++) {
		info[i] = (unsigned char)m_data[info_start+i];
	}
	sha.Update(info, info_len);
	sha.Final();
	unsigned char * temp = new unsigned char[256];
	
	sha.GetHash(temp);
	char retStr[61];
	retStr[0] = 0;
	char szTemp[4];
	char c[2];
	c[1] = 0;
	for(int i = 0; i < 20; i++) {
		sprintf(szTemp, "%u", temp[i]);
		int value = atoi(szTemp);
		m_info_hash[i] = value;
		if((value > 0 && value < 255) && (isalnum(value))) {
			c[0] = value;
			strcat(retStr, c);
		}
		else {
			c[0] = '%';
			strcat(retStr, c);
			if(value < 16) strcat(retStr, "0");
			itoa(value, szTemp, 16);
			strcat(retStr, szTemp);
		}
	}

	retStr[60] = 0;
	m_info_hash_string = retStr;
	delete [] info;
	info = NULL;
	delete [] temp;

}

int TorrentFile::GetNumPieces()
{
	return (m_numpieces);
}

int TorrentFile::GetPieceLength()
{
	return (m_piecelength);
}

int TorrentFile::GetLength()
{
	return(m_length);
}

unsigned int TorrentFile::GetSize() 
{
	unsigned int size = 0;
	unsigned int len = (unsigned int)v_announce_list.size(); // announce list
	vector <string>::iterator v1_Iter;
	for(v1_Iter = v_announce_list.begin(); v1_Iter != v_announce_list.end( ) ;v1_Iter++) {
		size+= (unsigned int)(*v1_Iter).size() + 1;    
	}
	//size +=
	size += 20*sizeof(m_info_hash); // m_info_hash;
	size += 256; // m_url[256];
	size += 256; // char m_name[256];
	size += (unsigned int)m_info_hash_string.size() + 1; // m_info_hash_string

	size += (unsigned int)v_pieces.size()*20*sizeof(unsigned short);
	
	PeerList * curr = m_peers;
	while(curr != NULL) {
		size += curr->GetSize()+sizeof(int);
		curr = curr->GetNext();
	}

	size += sizeof(bool);

	return size+sizeof(TorrentFileHeader);
}


void TorrentFile::GetBuffer(char *buffer) 
{
	char * temp = GetBuffer();
	memcpy(buffer, temp, GetSize());
	delete [] temp;
}

char * TorrentFile::GetBuffer() 
{
	int size = GetSize();
	int cur_pos = 0;
	char * buf = new char[size];
	memset(buf,0,size);
	char * ptr = buf;

	TorrentFileHeader header;
	header.SetAnnounceLength((int)GetAnnounceList().size());
	header.SetNumPieces((int)GetNumPieces());
	header.m_creationdate = m_creationdate;
	header.m_length = m_length;
	header.m_piecelength = m_piecelength;
	header.m_numpieces = m_numpieces;
	header.info_len = info_len;
	header.info_start = info_start;
	header.m_size = 0;//m_size;
	
	memcpy(ptr, &header, sizeof(TorrentFileHeader));
	ptr += sizeof(TorrentFileHeader);

	vector <string>::iterator v1_Iter;
	for(v1_Iter = v_announce_list.begin(); v1_Iter != v_announce_list.end( ) ;v1_Iter++) {
		strcpy(ptr, (*v1_Iter).c_str());
		ptr += (*v1_Iter).size() + 1;
	}
	
	memcpy(ptr, m_info_hash, 20*sizeof(unsigned short));
	ptr += 20*sizeof(unsigned short); // m_info_hash;
	memcpy(ptr, m_url, 256);
	ptr += 256; // m_url[256];
	memcpy(ptr, m_name, 256);
	ptr += 256; // char m_name[256];
	strcpy(ptr, m_info_hash_string.c_str());
	ptr  += (int)m_info_hash_string.size() + 1; // m_info_hash_string
	*((bool *)ptr) = b_is_valid;
	ptr+=sizeof(bool);


	vector <unsigned short*>::iterator v2_Iter;
	for(v2_Iter = v_pieces.begin(); v2_Iter != v_pieces.end( ) ;v2_Iter++) {
		memcpy(ptr, (*v2_Iter), 20);
		ptr += 20*sizeof(unsigned short);
	}
	
	PeerList * curr = m_peers;
	while(curr != NULL) {
		curr->GetBuffer(ptr);
		ptr += curr->GetSize();
		curr = curr->GetNext();
	}
	return buf;
}

TorrentFile::TorrentFile(void* data, UINT data_length) 
{
	char * buf = (char*)data;
	char * ptr = buf;

	TorrentFileHeader *header = (TorrentFileHeader*)data;
	ptr += sizeof(TorrentFileHeader);
	int announce_size = header->GetAnnounceLength();
	int num_pieces = header->GetNumPieces();

	m_piecelength = header->m_piecelength;
	m_numpieces = num_pieces;
	m_creationdate = header->m_creationdate;
	m_length = header->m_length;
	info_len = header->info_len;
	info_start = header->info_start;
	m_size = header->m_size;


	m_data = NULL;

	char temp[256];
	for(int i = 0; i < announce_size; i++) {
		string temp_str = strcpy(temp, ptr);
		v_announce_list.push_back(temp_str);
		ptr += (int)temp_str.size() + 1;
	}
	memcpy(m_info_hash, ptr, 20*sizeof(unsigned short));
	ptr += 20*sizeof(unsigned short); // m_info_hash;


	memcpy(m_url, ptr, 256);
	ptr += 256; // m_url[256];


	memcpy(m_name, ptr, 256);
	ptr += 256; // char m_name[256];

	m_info_hash_string = ptr;
	ptr += m_info_hash_string.size() + 1; // m_info_hash_string

	b_is_valid = *((bool *)ptr);
	ptr += sizeof(bool);

	for(int i = 0; i < num_pieces; i++) {
		unsigned short * temp = new unsigned short[20];
		memcpy(temp, ptr, 20*sizeof(unsigned short));
		ptr += 20*sizeof(unsigned short);
		v_pieces.push_back(temp);
	}

	m_peers = NULL;

	int cur_pos = (int)(ptr - buf);

	while(cur_pos < (int)data_length) {
		if(buf[cur_pos] <= 0) break;
		PeerList * temp = new PeerList(&buf[cur_pos], &cur_pos);
		if(m_peers == NULL) m_peers = temp;
		else m_peers->Add(temp);
	}
}
