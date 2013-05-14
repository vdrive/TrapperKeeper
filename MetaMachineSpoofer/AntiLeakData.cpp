#include "StdAfx.h"
#include "antileakdata.h"
#include "..\tkcom\Timer.h"
#include "SpoofingSystem.h"


AntiLeakData::AntiLeakData(void)
{
}

AntiLeakData::~AntiLeakData(void)
{
}

byte AntiLeakData::ConvertCharToInt(char ch)
{
	switch(ch){
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A': return 10;
		case 'a': return 10;
		case 'B': return 11;
		case 'b': return 11;
		case 'C': return 12;
		case 'c': return 12;
		case 'D': return 13;
		case 'd': return 13;
		case 'E': return 14;
		case 'e': return 14;
		case 'F': return 15;
		case 'f': return 15;
	}
	return 0;
}

void AntiLeakData::Update(void)
{
	/*
	static Timer last_update;
	static bool b_has_updated=false;

	if(last_update.HasTimedOut(60*60*4) || !b_has_updated){
		b_has_updated=true;
		last_update.Refresh();

		//send a request for rare files
		Buffer2000 response_packet1;
		response_packet1.WriteByte(1);  //version
		response_packet1.WriteByte(1);  //its an question for the server
		response_packet1.WriteByte(ED2KGETANTILEAKRARE);  //swarms
		response_packet1.WriteByte(ED2KCONTROLKEY);  //swarms
		response_packet.WriteWord(190);  //we will set this later to the number of swarms we added

		MetaSpooferReference ref;
		ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet1.GetBufferPtr(),response_packet1.GetLength());
		ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet1.GetBufferPtr(),response_packet1.GetLength());

		//send a request for popular files
		Buffer2000 response_packet2;
		response_packet2.WriteByte(1);  //version
		response_packet2.WriteByte(1);  //its an question for the server
		response_packet2.WriteByte(ED2KGETANTILEAKPOPULAR);  //swarms
		response_packet2.WriteByte(ED2KCONTROLKEY);  //swarms
		response_packet.WriteWord(10);  //we will set this later to the number of swarms we added

		MetaSpooferReference ref;
		ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSDEBUG,(void*)response_packet2.GetBufferPtr(),response_packet2.GetLength());
		ref.System()->m_com.SendReliableData(INTERDICTIONSERVERADDRESSMAIN,(void*)response_packet2.GetBufferPtr(),response_packet2.GetLength());

	}*/
}

void AntiLeakData::AddPopularAntiLeakFile(const char* file_name, const char* hash, UINT size)
{
	/*
	CString str_hash=hash;
	if(str_hash.GetLength()<32)
		return;  //not valid data
	byte byte_hash[16];
	byte *byte_hash_set=NULL;
	
	for(int j=0;j<16;j++){
		char ch1=str_hash[j*2];
		char ch2=str_hash[j*2+1];
		byte val1=ConvertCharToInt(ch1);
		byte val2=ConvertCharToInt(ch2);
		byte hash_val=((val1&0xf)<<4)|(val2&0xf);
		byte_hash[j]=hash_val;
	}

	mv_popular_anti_leak_files.Add(new AntiLeakFile(file_name,hash,byte_hash,size));*/
}

void AntiLeakData::AddRareAntiLeakFile(const char* file_name, const char* hash, UINT size)
{
	/*
	CString str_hash=hash;
	if(str_hash.GetLength()<32)
		return;  //not valid data
	byte byte_hash[16];
	byte *byte_hash_set=NULL;
	
	for(int j=0;j<16;j++){
		char ch1=str_hash[j*2];
		char ch2=str_hash[j*2+1];
		byte val1=ConvertCharToInt(ch1);
		byte val2=ConvertCharToInt(ch2);
		byte hash_val=((val1&0xf)<<4)|(val2&0xf);
		byte_hash[j]=hash_val;
	}

	mv_rare_anti_leak_files.Add(new AntiLeakFile(file_name,hash,byte_hash,size));*/
}

void AntiLeakData::ClearRareAntiLeakFiles(void)
{
	//mv_rare_anti_leak_files.Clear();
}

void AntiLeakData::ClearPopularAntiLeakFiles(void)
{
	//mv_popular_anti_leak_files.Clear();
}
