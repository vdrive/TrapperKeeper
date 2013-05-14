#include "StdAfx.h"
#include "arespoolfile.h"
#include "..\tkcom\frameheader.h"
#include "..\tkcom\sha1.h"

AresPoolFile::AresPoolFile(void)
{
}

AresPoolFile::~AresPoolFile(void)
{
}

void AresPoolFile::FillBufferWithMP3Data(byte* mp3_buffer, UINT mp3_buf_len)
{
	int NOISE_BUFFER_LEN	=((4*1024)+1);	// 4K + 1
	int NOISE_DATA_LEN		=(1<<18)-NOISE_BUFFER_LEN;	// 256K


	// Else use random noise
	int i;

	byte* p_noise_data_buf=new byte[NOISE_DATA_LEN+NOISE_BUFFER_LEN];
	memset(p_noise_data_buf,0,NOISE_DATA_LEN+NOISE_BUFFER_LEN);
	
	FrameHeader hdr(32,44.1);
	unsigned int frame_len=hdr.ReturnFrameSize();

	unsigned int offset=0;
	while(1)
	{
		if((NOISE_DATA_LEN-offset)<sizeof(FrameHeader))
		{
			break;
		}
		memcpy(&p_noise_data_buf[offset],&hdr,sizeof(FrameHeader));
		offset+=sizeof(FrameHeader);

		if((NOISE_DATA_LEN-offset)<frame_len)
		{
			break;
		}
			
		for(i=0;i<(int)frame_len-(int)sizeof(FrameHeader);i++)
		{
//			p_noise_data_buf[offset+i]=(i+rand()%2)%256;	// noisy hell
//			p_noise_data_buf[offset+i]=rand()%256;			// less frequent noise (on winamp, but not on mediaplayer)
			
			p_noise_data_buf[offset+i]=rand()%12;			// quiet noise
		}

		offset+=frame_len-sizeof(FrameHeader);
	}

	// Init remainder of dookie
	for(i=offset;i<NOISE_DATA_LEN;i++)
	{
		p_noise_data_buf[i]=rand()%256;
	}

	// Copy out the buffer padding
	memcpy(&p_noise_data_buf[NOISE_DATA_LEN],&p_noise_data_buf[0],NOISE_BUFFER_LEN);

	if(mp3_buf_len<(UINT)(NOISE_DATA_LEN+NOISE_BUFFER_LEN))
		memcpy(mp3_buffer,p_noise_data_buf,mp3_buf_len);

	delete []p_noise_data_buf;
}

void AresPoolFile::Generate(UINT size)
{
	
	m_size=size;
	if(size<20000000){
		
		byte *buffer=new byte[size];
		//create a signature full of mp3 data
		FillBufferWithMP3Data(m_signature,SIGNATURELENGTH);
		
		//generate a file from this signature, so that we may compute a hash for it
		UINT init_copy_len=min(SIGNATURELENGTH,size);
		memcpy(buffer,m_signature,init_copy_len);
		int length=size;
		length-=init_copy_len;
		UINT cur_buf_pos=init_copy_len;
		while(length>0){
			memcpy(buffer+cur_buf_pos,m_signature,min(SIGNATURELENGTH,length));
			cur_buf_pos+=SIGNATURELENGTH;
			length-=min(SIGNATURELENGTH,length);
		}		


		SHA1 sha;
		sha.Update(buffer,size);
		sha.Final();
		sha.GetHash(m_hash);

		if(m_size>=HASHCHUNKSIZE){
			SHA1 sha_chunk;
			sha_chunk.Update(buffer,HASHCHUNKSIZE);
			sha_chunk.Final();
			sha_chunk.GetHash(m_part_hash);
		}

		//we also need to capture the last chunk
		int remainder=m_size%HASHCHUNKSIZE;
		SHA1 sha_last_chunk;
		sha_last_chunk.Update(buffer+m_size-remainder,remainder);
		sha_last_chunk.Final();
		sha_last_chunk.GetHash(m_last_part_hash);

		m_shash=EncodeBase16(m_hash,20);
		delete []buffer;
		m_sig_buf2k.WriteBytes(m_signature,SIGNATURELENGTH);
	}
	else{
		//big file, any data will suffice
		for(int i=0;i<SIGNATURELENGTH;i++){
			m_signature[i]=rand()&255;
		}
		int length=size;
		SHA1 sha;
		SHA1 sha_chunk;
		SHA1 sha_last_chunk;

		int last_chunk_size=m_size%HASHCHUNKSIZE; //get the arbitrary length of the last chunk

		int hash_chunk_length=HASHCHUNKSIZE;

		while(length>0){
			sha.Update(m_signature,min(length,SIGNATURELENGTH));
			
			if(length<=last_chunk_size){  
				//we are in range of the last chunk, which will have a different sha than all the other chunks because it is not the same length as the other chunks
				sha_last_chunk.Update(m_signature,min(length,SIGNATURELENGTH));
			}

			length-=min(length,SIGNATURELENGTH);

			if(hash_chunk_length>0){
				//get the hash for a part of the file, so we can serve an actual downloadable file to people
				sha_chunk.Update(m_signature,min(hash_chunk_length,SIGNATURELENGTH));
				hash_chunk_length-=min(hash_chunk_length,SIGNATURELENGTH);
			}
		}
		sha.Final();
		sha.GetHash(m_hash);
		m_shash=EncodeBase16(m_hash,20);

		sha_last_chunk.Final();
		sha_last_chunk.GetHash(m_last_part_hash);

		//get the hash for a part of the file, so we can serve a downloadable file
		sha_chunk.Final();
		sha_chunk.GetHash(m_part_hash);

		m_sig_buf2k.WriteBytes(m_signature,SIGNATURELENGTH);
	}

	//byte tmp_hash[20];
	//memcpy(tmp_hash,m_hash,20);
//	Init(tmp_hash,"transferinc","transferinc","transferinc.mp3","transferinc",m_size,600,false,true);
	
}