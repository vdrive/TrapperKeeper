#include "StdAfx.h"
#include "databuffer.h"
#include "TKComInterface.h"
#include <mmsystem.h>
#include "zlib.h"

//create it compressed and encrypted
DataBuffer::DataBuffer( byte *data , UINT data_length , UINT op_code )
{	
	//ZeroMemory(m_data_sample,COMDATABUFFERSAMPLE);
	//memcpy(m_data_sample,data,min(COMDATABUFFERSAMPLE,data_length));
	//do the compression
	//The beauty of it is that we will only actually use it if the compressed data is smaller than the original data length.
	UINT original_data_size=data_length;
	byte *compressed_data=new byte[data_length+4096];  //an extra 4k that is more than adequate for compression headers on data that is not very compressable via lossless compression or because of small size
	UINT compressed_length=data_length+4096;
	compress ((Bytef *)compressed_data, (uLongf *)&compressed_length, (const Bytef *)data, (uLong) data_length);

	bool b_compressed=false;
	if((compressed_length+4)<data_length){ //the 4 is for the extra header info for compressed data (the original length)
		b_compressed=true;
		data=compressed_data;  //reassign the data and length so that the rest is transparrent to compression being enabled or not.
		data_length=compressed_length;
	}

	srand(timeGetTime());
	UINT total_length=data_length+1+1+1+1+4+4;  //1 byte for key, 1 byte for start of header, 1 byte for message type, 1 byte for compressed flag, 4 bytes for length, 4 bytes for op_code
	if(b_compressed)
		total_length+=4;  //extra space for the original data length.

	m_data=new byte[total_length];

	byte xor= 150;//144+(rand()%30);

	m_data[0]=xor;  //encode a random byte used for encryption
	m_data[1]=TKCOMBUFFERKEY;
	m_data[2]=TKCOMMESSAGECODE;
	if(b_compressed)  //set the compression flag.
		m_data[3]=(byte)1;
	else
		m_data[3]=(byte)0;

	*((UINT*)(m_data+4))=total_length;
	*((UINT*)(m_data+8))=op_code;
	if(b_compressed)//if it was compressed we need to store an integer for the original data size.  To allocate a decompression buffer.
		*((UINT*)(m_data+12))=original_data_size;
	
	xor^=TKCOMBUFFERKEY;  //modify our encryption byte

	memcpy(m_data+12+((b_compressed)?4:0),data,data_length);

	//go through and encrypt the entire buffer, except for the first byte which is the key
	for(UINT i=1;i<total_length;i++)  //super Ty encryption.  Its pretty lame but its not bad if someone doesn't have a copy of trapper keeper.
		m_data[i]^=(xor++);  //encyrpt the entire thing with a rotating key, except the first byte which is the key.

	mb_sealed=true;

	if(compressed_data)
		delete []compressed_data;  //clean up any temporary bufferes we may have created for compression.
}

DataBuffer::DataBuffer()
{
	m_data=NULL;
	mb_sealed=false;
}

DataBuffer::~DataBuffer(void)
{
	//TRACE("Data Buffer %d deleting itself.\n",m_object_tag);
	if(m_data!=NULL)
		delete [] m_data;
	//TRACE("Data Buffer done deleting itself.\n");
}

//This returns the amount read from the buffer, or -1 if there was an error.
//The returned value is NOT the length of the data contained within.
//if successful the GetBuffer() will return a pointer to data that is decrypted and uncompressed.
int DataBuffer::ReadFromBuffer(const char* source_ip , byte* buffer , UINT buffer_length)
{
	if(buffer_length<11) //need to be able to read two header bytes plus integer length
		return 0; //not enough to read yet

	//make a copy of the header and decrypt it.
	byte tmp[12];
	memcpy(tmp,buffer,12);
	byte xor=tmp[0];
	xor^=TKCOMBUFFERKEY;
	if(xor==0)
		return -1;  //this isn't possible if this is from a valid trapper keeper.

	for(int i=1;i<12;i++) //decrypt the header only to test
		tmp[i]^=(xor++);

	if((tmp[1])!=TKCOMBUFFERKEY)
		return -1; //-1 is an error, a valid DataBuffer object can never be read from this supplied buffer

	if(((tmp[2]) & TKCOMMESSAGECODE)==0)
		return -1;
	
	if(source_ip==NULL)
		return -1;

	if(m_data!=NULL)
		delete []m_data;
	m_data=NULL;

	bool b_compressed=(tmp[3])?true:false;	

	//read total length encoded
	UINT total_length=*(UINT*)(tmp+4);  //total length

	ASSERT(total_length<50000000);
	if(total_length>buffer_length)
		return 0; //not enough to read yet.

	//test is complete, we need to load the whole thing

	//we have real data, decompress and decrypt it.
	xor=tmp[0]; //reset our key
	xor^=TKCOMBUFFERKEY;

	//decrypt the entire buffer
	for(UINT i=1;i<total_length;i++)
		buffer[i]^=(xor++);

	UINT slen=(UINT)strlen(source_ip)+1;
	UINT data_size=total_length+slen-2;  //-2 to strip the key off the header and the is compressed flag

	if(b_compressed){
		UINT original_size=*(UINT*)(buffer+12);  //get the original size of our data.
		byte *uncompressed_data=new byte[original_size];
		UINT uncompress_buf_length=original_size;
		uncompress ((Bytef*)uncompressed_data, (uLongf *)&uncompress_buf_length, (const Bytef *)(buffer+16), (uLong)total_length-16);
		if(uncompress_buf_length!=original_size){ //check for wierd errors
			ASSERT(0);
			delete []uncompressed_data;
			return -1;
		}
		data_size=10+original_size+slen;
		m_data=new byte[data_size];  //make room to encode the source_ip, this buffer gets deleted by the destructor.
		memcpy(m_data+10+slen,uncompressed_data,original_size);  //append our uncompressed data.  
		*((UINT*)(m_data+2))=data_size; //encode the length of the stream
		delete []uncompressed_data;  //free out buffer that we temporary allocated to decompress stuff into.
	}
	else{
		m_data=new byte[data_size];  //make room to encode the source_ip
		memcpy(m_data+10+slen,buffer+12,total_length-12);  //append our uncompressed data.
		*((UINT*)(m_data+2))=data_size; //encode the length of the stream
	}

	//encode parts of the header
	memcpy(m_data,buffer+1,2);  //copy first 2 bytes for header
	memcpy(m_data+6,buffer+8,4);//move the op_code over, the length has already been encoded
	strcpy((char*)(m_data+10),source_ip);  //insert the source_ip
	
	return total_length;  //return the amount that we read from the buffer
}

UINT DataBuffer::GetDataLength()
{	
	if(m_data==NULL)
		return 0;

	if(!mb_sealed)
		return *((UINT*)(m_data+2));

	//make a copy of the header and decrypt it.
	byte tmp[8];
	memcpy(tmp,m_data,8);
	byte xor=tmp[0];
	xor^=TKCOMBUFFERKEY;
	for(int i=1;i<8;i++) //decrypt the header only to get the length, decrypting the whole thing for simplicity
		tmp[i]^=(xor++);

	return *((UINT*)(tmp+4));
}

UINT DataBuffer::GetOpCode(void)
{
	if(m_data==NULL)
		return 0;

	if(!mb_sealed)
		return *((UINT*)(m_data+6));

	//make a copy of the header and decrypt it.
	byte tmp[12];
	memcpy(tmp,m_data,12);
	byte xor=tmp[0];
	xor^=TKCOMBUFFERKEY;
	for(int i=1;i<12;i++) //decrypt the header only to get the length, decrypting the whole thing for simplicity
		tmp[i]^=(xor++);

	return *((UINT*)(tmp+8));
}

const char* DataBuffer::GetSource(void)
{
	if(m_data==NULL || mb_sealed)
		return NULL;

	return (const char*)(m_data+10);
}

byte* DataBuffer::GetBuffer(void)
{
	return m_data;
}
