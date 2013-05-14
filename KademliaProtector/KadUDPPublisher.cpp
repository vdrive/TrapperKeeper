#include "StdAfx.h"
#include "kadudppublisher.h"
#include "KademliaProtectorSystem.h"
#include "KadFileKey.h"
#include "Tag.h"
#include "UInt128.h"
#include "..\tkcom\Buffer2000.h"
#include "StringConversion.h"


#define TAG_FILESIZE			"\x02"	// <uint32>
#define TAG_SOURCETYPE			"\xFF"	// <uint8>
#define TAG_FILENAME			"\x01"	// <string>
#define TAG_SOURCEPORT			"\xFD"	// <uint16>

using namespace Kademlia;

KadUDPPublisher::KadUDPPublisher(void)
{
	m_publish_index=0;
	m_decoy_publish_index=0;
	mp_socket=NULL;
}

KadUDPPublisher::~KadUDPPublisher(void)
{
}

UINT KadUDPPublisher::Run(void)
{
	TRACE("KadUDPPublisher::Run(void) THREAD STARTED\n");
	
	srand(GetTickCount());

	KademliaProtectorSystemReference ref;
	while(!this->b_killThread){
		Sleep(50);
		for(int i=0;i<4 && !this->b_killThread;i++){
			Sleep(50);
			PublishNextFile();
		}

		for(int i=0;i<10 && !this->b_killThread;i++){
			Sleep(100);
			PublishNextDecoy();
		}

	//	TRACE("Stepping through KadUDPPublisher::Run(void)\n");
	}


	TRACE("KadUDPPublisher::Run(void) THREAD TERMINATED\n");
	return 0;
}

void KadUDPPublisher::AddFileToPublishFileList(KadFile* file)
{
	CSingleLock lock(&m_lock,TRUE);  //TYDEBUG
	mv_files_to_publish.Add(file);

	TRACE("Queueing new file for publish: %s\n",file->m_name.c_str());
}

void KadUDPPublisher::StartPublishing(void)
{
	this->StartThreadLowPriority();
}

void KadUDPPublisher::StopPublishing(void)
{
	this->StopThread();
}

void KadUDPPublisher::GetNextFileToPublish(Vector& v_files)
{
	CSingleLock lock(&m_lock,TRUE);
	if(m_publish_index>=mv_files_to_publish.Size())
		m_publish_index=0;

	if(m_publish_index>=mv_files_to_publish.Size())
		return;

	v_files.Add(mv_files_to_publish.Get(m_publish_index++));
}

void KadUDPPublisher::GetNextDecoyToPublish(Vector& v_files)
{
	CSingleLock lock(&m_lock,TRUE);
	if(m_decoy_publish_index>=mv_decoys_to_publish.Size())
		m_decoy_publish_index=0;

	if(m_decoy_publish_index>=mv_decoys_to_publish.Size())
		return;

	v_files.Add(mv_decoys_to_publish.Get(m_decoy_publish_index++));
}

void KadUDPPublisher::SetSocket(CAsyncSocket* p_socket)
{
	mp_socket=p_socket;
}


void KadUDPPublisher::PublishFile(KadFile* file, CUInt128* key,KadContact *dest)
{
	// <HASH (key) [16]> <CNT1 [2]> (<HASH (target) [16]> <CNT2 [2]> <META>*(CNT2))*(CNT1)
	// key <how many files> (<file hash> <how many meta tags>( meta tags)) 

	{  //create this file on the network...  this doesn't actually publish ourselves as a source
		//The emule comments lie concerning kad

		Buffer2000 publish_packet;
		publish_packet.WriteByte(0xE4);
		publish_packet.WriteByte(0x40);
		publish_packet.WriteBytes(key->getDataPtr(),16);
		publish_packet.WriteWord(1);  //1 file
		publish_packet.WriteBytes(file->m_hash.getDataPtr(),16);  //file hash
		
		string extension=GetExtension(file->m_name.c_str());
		string type=GetFileType(extension.c_str());
		int media_length=CalculateMediaLength(extension.c_str(),file->m_name.c_str(),file->m_size);
		int bitrate=GetBitrate(extension.c_str(),file->m_name.c_str());
		string codec=GetCodec(extension.c_str(),file->m_name.c_str());

		int num_tags=3;
		if(strlen(extension.c_str())>0)
			num_tags++;
		if(strlen(type.c_str())>0)
			num_tags++;
		if(strlen(codec.c_str())>0)
			num_tags++;

		if(media_length>0)
			num_tags++;
		if(bitrate>0)
			num_tags++;

		publish_packet.WriteByte(num_tags);  //number of tags


		//write out name
		WriteEmuleStringTag(file->m_name.c_str(),0x01,publish_packet);
		//write out file size
		WriteEmuleIntTag(file->m_size,0x02,publish_packet);

		if(strlen(type.c_str())>0)
			WriteEmuleStringTag(type.c_str(),0x03,publish_packet);
		if(strlen(extension.c_str())>0)
			WriteEmuleStringTag(extension.c_str(),0x04,publish_packet);

		//write avail
		WriteEmuleByteTag(1,0x15,publish_packet);
		if(media_length>0)
			WriteEmuleIntTag(media_length,0xD3,publish_packet);
		if(strlen(codec.c_str())>0)
			WriteEmuleStringTag(codec.c_str(),0xD5,publish_packet);
		if(bitrate>0)
			WriteEmuleIntTag(bitrate,0xd4,publish_packet);

		mp_socket->SendTo(publish_packet.GetBufferPtr(),publish_packet.GetLength(),dest->m_port,dest->m_sip.c_str());  //ask them to tell us about some clients they happen to know about
	}
}

/*
void KadUDPPublisher::WriteTag(CTag& tag, Buffer2000& buffer)
{

	uint8 type;
	if (tag.m_type == 0xFE)
	{
		if (tag.GetInt() <= 0xFF)
			type = TAGTYPE_UINT8;
		else if (tag.GetInt() <= 0xFFFF)
			type = TAGTYPE_UINT16;
		else
			type = TAGTYPE_UINT32;
	}
	else
		type = tag.m_type;

	buffer.WriteByte(type);
	//writeByte(type);

	const CTagNameString& name = tag.m_name;
	buffer.WriteWord(name.GetLength());
	buffer.WriteString((LPCSTR)name);
	//writeUInt16(name.GetLength());
	//writeArray((LPCSTR)name, name.GetLength());

	switch (type)
	{
		case TAGTYPE_HASH:
			// Do NOT use this to transfer any tags for at least half a year!!
			buffer.WriteBytes(tag.GetHash(),16);
			//writeHash(tag->GetHash());
			ASSERT(0);
			break;
		case TAGTYPE_STRING:
		{
			
			//AtlUnicodeToUTF8(rwstr, rwstr.GetLength(), pszUTF8, iChars);
			CUnicodeToUTF8 utf8(tag.GetStr());
			buffer.WriteWord(utf8.GetLength());
			buffer.WriteBytes((byte*)(LPCSTR)utf8,utf8.GetLength());
			//writeUInt16(utf8.GetLength());
			//writeArray(utf8, utf8.GetLength());
			break;
		}
		case TAGTYPE_UINT32:
			buffer.WriteDWord(tag.GetInt());
			//writeUInt32(tag->GetInt());
			break;
		case TAGTYPE_FLOAT32:
			buffer.WriteFloat(tag.GetFloat());
			//writeFloat(tag->GetFloat());
			break;
		case TAGTYPE_BSOB:
			// Do NOT use this to transfer any tags for at least half a year!!
			//writeBsob(tag->GetBsob(), tag->GetBsobSize());
			//writeUInt8(size);
			//writeArray(value, size);
			buffer.WriteByte(tag.GetBsobSize());
			buffer.WriteBytes(tag.GetBsob(),tag.GetBsobSize());
			ASSERT(0);
			break;
		case TAGTYPE_UINT16:
			buffer.WriteWord(tag.GetInt());
			//writeUInt16(tag->GetInt());
			break;
		case TAGTYPE_UINT8:
			buffer.WriteByte(tag.GetInt());
			//writeUInt8(tag->GetInt());
			break;
		default:  ASSERT(0);
	} 
}*/

void KadUDPPublisher::WriteEmuleIntTag(int val,byte cmd,Buffer2000& output){
	output.WriteByte(0x03);
	output.WriteWord(0x01);
	output.WriteByte(cmd);
	output.WriteDWord(val);
}

void KadUDPPublisher::WriteEmuleByteTag(byte val,byte cmd,Buffer2000& output){
	output.WriteByte(0x09);
	output.WriteWord(0x01);
	output.WriteByte(cmd);
	output.WriteByte(val);
}

void KadUDPPublisher::WriteEmuleWordTag(WORD val,byte cmd,Buffer2000& output){
	output.WriteByte(0x08);
	output.WriteWord(0x01);
	output.WriteByte(cmd);
	output.WriteWord(val);
}

void KadUDPPublisher::WriteEmuleStringTag(const char* val,byte cmd,Buffer2000& output){
	output.WriteByte(0x02);
	output.WriteWord(0x01);
	output.WriteByte(cmd);
	output.WriteWord((WORD)strlen(val));
	output.WriteString(val);
}


void KadUDPPublisher::PublishSource(byte *hash,KadContact *dest)
{
	//The emule comments lie concerning kad

	//when you are publishing yourself as a source, you put the file hash first instead of the keyword key
	Buffer2000 publish_packet;
	publish_packet.WriteByte(0xE4);
	publish_packet.WriteByte(0x40);
	publish_packet.WriteBytes(hash,16);//file->m_hash.getDataPtr()
	publish_packet.WriteWord(1);  //1 file

	byte rand_hash[16];
	for(int i=0;i<16;i++){
		rand_hash[i]=rand()%255;
	}

	publish_packet.WriteBytes(rand_hash,16);  //some hash
	
	publish_packet.WriteByte(2);  //number of tags
	//write out name
	
	//write out file size
	WriteEmuleByteTag(1,0xff,publish_packet);  //source type is 1, an unfirewalled source
	WriteEmuleWordTag(BASEPORT+(rand()%NUMBERPORTS),0xfd,publish_packet);

	mp_socket->SendTo(publish_packet.GetBufferPtr(),publish_packet.GetLength(),dest->m_port,dest->m_sip.c_str());  //tell this contact that we are a source for a particular file
}

void KadUDPPublisher::UnpublishFiles(void)
{
	CSingleLock lock(&m_lock,TRUE);
	mv_files_to_publish.Clear();
}

string KadUDPPublisher::GetExtension(const char* file_name){
	int str_len=(int)strlen(file_name);

	string extension="";
	for(int i=str_len-1;i>=0;i--){
		unsigned char ch=file_name[i];
		if(ch=='.'){
			return extension;
		}
		else{
			string tmp;
			tmp+=ch;
			tmp+=extension;
			extension=tmp;
		}
	}
	return extension;
}

const char* KadUDPPublisher::GetCodec(const char* extension,const char* file_name){
	if(stricmp(extension,"avi")==0){
		CString lc_file_name=file_name;
		lc_file_name=lc_file_name.MakeLower();

		if(lc_file_name.Find("xvid")!=-1)
			return "xvid";
		if(lc_file_name.Find("divx")!=-1)
			return "dx50";
		else return "xvid";
	}
	else if(stricmp(extension,"mpg")==0 || stricmp(extension,"mpeg")==0){
		return "yuy2";
	}
	else if(stricmp(extension,"wmv")==0){
		return "wmv2";
	}
	else return "";
}

int KadUDPPublisher::GetBitrate(const char* extension,const char* file_name){
	if(stricmp(extension,"avi")==0){
		return 600+(rand()%500);
	}
	else if(stricmp(extension,"mpg")==0 || stricmp(extension,"mpeg")==0){
		return 192;
	}
	else if(stricmp(extension,"wmv")==0){
		return 1440;
	}
	else if(stricmp(extension,"mp3")==0 || stricmp(extension,"wma")==0){
		if(rand()&1)
			return 192;
		else return 160;
	}
	else return 0;
}

int KadUDPPublisher::CalculateMediaLength(const char* extension,const char* file_name,unsigned int file_size){
	CString lc_file_name=file_name;
	lc_file_name=lc_file_name.MakeLower();

	if((stricmp(extension,"avi")==0 || stricmp(extension,"mpg")==0 || stricmp(extension,"wmv")==0) || stricmp(extension,"mpeg")==0 ){
		if (lc_file_name.Find("cd1") != -1 ||
			lc_file_name.Find("cd 1") != -1 ||
			lc_file_name.Find("cd2") != -1 ||
			lc_file_name.Find("cd 2") != -1){

			return 3000+(rand()%600);//make it shorter for a cd 1 or cd 2
		}
		else{
			return 3600+2200+(rand()%1400);
		}
	}
	else if(stricmp(extension,"mp3")==0 || stricmp(extension,"wma")==0 || stricmp(extension,"wav")==0){
		srand(file_size/50000);  //generate the media length according to the size of the file
		return 180+(rand()%120);//new_random.nextInt(120); //between 3 and 5 minutes long
	}

	return 0;
}

const char* KadUDPPublisher::GetFileType(const char* extension){
	if(stricmp(extension,"avi")==0 || stricmp(extension,"mpg")==0 || stricmp(extension,"wmv")==0 || stricmp(extension,"mpeg")==0 ){
		return "Video";
	}
	else if(stricmp(extension,"mp3")==0 || stricmp(extension,"wma")==0 || stricmp(extension,"wav")==0){
		return "Audio";
	}
	else if(stricmp(extension,"par")==0 || stricmp(extension,"zip")==0 || stricmp(extension,"rar")==0 || stricmp(extension,"iso")==0 || stricmp(extension,"bin")==0 || stricmp(extension,"cue")==0){
		return "Pro";
	}
	else if(stricmp(extension,"txt")==0 || stricmp(extension,"doc")==0 || stricmp(extension,"rtf")==0 || stricmp(extension,"nfo")==0){
		return "Doc";
	}
	return "";
}
void KadUDPPublisher::UnpublishDecoys(void)
{
	CSingleLock lock(&m_lock,TRUE);
	mv_decoys_to_publish.Clear();
}

void KadUDPPublisher::AddDecoyToPublishFileList(DonkeyFile* df)
{
	CSingleLock lock(&m_lock,TRUE);
	mv_decoys_to_publish.Add(df);

	TRACE("Queueing new decoy for publish: %s\n",df->m_shash.c_str());
}

void KadUDPPublisher::PublishNextFile(void)
{
	KademliaProtectorSystemReference ref;
	Vector v_files;

	GetNextFileToPublish(v_files);

	if(v_files.Size()==0)
		return;

	KadFile *file=(KadFile*)v_files.Get(0);

	if(!file->m_last_publish.HasTimedOut(60*60)){
		return;
	}

	ref.System()->LogToFile("KadUDPPublisher::Run(void) BEGIN");

	file->m_last_publish.Refresh();

	Vector v_contacts;
	//publish ourselves as a source for this file
	ref.System()->GetContactManager()->GetContactsForKey(file->m_hash,v_contacts);

	while(v_contacts.Size()>100){
		v_contacts.Remove(rand()%v_contacts.Size());
	}

	for(int i=0;i<(int)v_contacts.Size();i++){
		if(rand()&1)
			continue;
		PublishSource(file->m_hash.getDataPtr(),(KadContact*)v_contacts.Get(i));
	}

}

void KadUDPPublisher::PublishNextDecoy(void)
{
	KademliaProtectorSystemReference ref;
	Vector v_files;

	GetNextDecoyToPublish(v_files);

	if(v_files.Size()==0)
		return;

	DonkeyFile *file=(DonkeyFile*)v_files.Get(0);

	//if(!file->m_last_publish.HasTimedOut(60*60)){
	//	return;
	//}

	ref.System()->LogToFile("KadUDPPublisher::PublishNextDecoy(void) BEGIN");

	//file->m_last_publish.Refresh();

	Vector v_contacts;
	//publish ourselves as a source for this file
	CUInt128 key;
	key.setValueBE(file->m_hash);
	ref.System()->GetContactManager()->GetContactsForKey(key,v_contacts);

	while(v_contacts.Size()>20){  //TYDEBUG
		v_contacts.Remove(rand()%v_contacts.Size());
	}

	TRACE("Publishing decoy %s\n",file->m_shash.c_str());
	for(int i=0;i<(int)v_contacts.Size();i++){
		if(rand()&1)
			continue;
		PublishSource(key.getDataPtr(),(KadContact*)v_contacts.Get(i));
	}
}
