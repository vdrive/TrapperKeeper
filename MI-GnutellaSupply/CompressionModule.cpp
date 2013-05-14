// CompressionModule.cpp

#include "stdafx.h"
#include "CompressionModule.h"

//#define COMPRESSION_ALGO_GZIP

#ifdef COMPRESSION_ALGO_GZIP
#include "gzip.h"
#else	// else use zlib instead of gzip
#include "zlib.h"
#endif // COMPRESSION_ALGO_GZIP

//
//
//
CompressionModule::CompressionModule()
{
	memset(this,0,sizeof(CompressionModule));
}

//
//
//
CompressionModule::~CompressionModule()
{
	if(p_compressed_buf!=NULL)
	{
		delete [] p_compressed_buf;
		p_compressed_buf=NULL;
		m_compressed_buf_len=0;
	}

	if(p_decompressed_buf!=NULL)
	{
		delete [] p_decompressed_buf;
		p_decompressed_buf=NULL;
		m_decompressed_buf_len=0;
	}
}

//
//
//
void CompressionModule::CompressBuffer(byte *buf,unsigned int buf_len)
{
	// Make sure the buffer is empty
	if(p_compressed_buf!=NULL)
	{
		delete [] p_compressed_buf;
		p_compressed_buf=NULL;
		m_compressed_buf_len=0;
	}
/*
#ifdef COMPRESSION_ALGO_GZIP
	CryptoPP::Gzip gzip;//(NULL,5);	compression level = 5 is default, 1 is fast, 9 is slow
#else
	CryptoPP::ZlibCompressor gzip;
#endif // COMPRESSION_ALGO_GZIP

	gzip.Put(buf,buf_len);
	//gzip.Close();
	
	m_compressed_buf_len=gzip.MaxRetrievable();
	p_compressed_buf=new unsigned char[m_compressed_buf_len];	
	gzip.Get(p_compressed_buf,m_compressed_buf_len);
*/
	UINT compressed_buffer_length = (UINT)(((float)buf_len * 1.001) + 12); //0.1 percent larger than orginal size plus 12 bytes 
	byte* compessed_buffer = new byte[compressed_buffer_length];
	compress((Bytef *)compessed_buffer, (uLongf *)&compressed_buffer_length, (const Bytef *)buf, (uLong) buf_len);
	m_compressed_buf_len = compressed_buffer_length;
	p_compressed_buf=new unsigned char[m_compressed_buf_len];	
	memcpy(p_compressed_buf, compessed_buffer,m_compressed_buf_len);
	delete [] compessed_buffer;
}

//
//
//
unsigned char *CompressionModule::ReturnCompressedBuffer()
{
	return p_compressed_buf;
}

//
//
//
unsigned int CompressionModule::ReturnCompressedBufferLength()
{
	return m_compressed_buf_len;
}

//
//
//
/*
void CompressionModule::DecompressBuffer(unsigned char *buf,unsigned int buf_len)
{
	// Make sure the buffer is empty
	if(p_decompressed_buf!=NULL)
	{
		delete [] p_decompressed_buf;
		p_decompressed_buf=NULL;
		m_decompressed_buf_len=0;
	}
	
#ifdef COMPRESSION_ALGO_GZIP
	CryptoPP::Gunzip gunzip;
#else
	CryptoPP::ZlibDecompressor gunzip;
#endif // COMPRESSION_ALGO_GZIP

	try
	{
		gunzip.Put(buf,buf_len);
//		gunzip.Close();
	}
	catch(CryptoPP::Exception &e)
	{

	}

	m_decompressed_buf_len=gunzip.MaxRetrievable();
	p_decompressed_buf=new unsigned char[m_decompressed_buf_len];

	gunzip.Get(p_decompressed_buf,m_decompressed_buf_len);
}
*/
//
//
//
unsigned char *CompressionModule::ReturnDecompressedBuffer()
{
	return p_decompressed_buf;
}

//
//
//
unsigned int CompressionModule::ReturnDecompressedBufferLength()
{
	return m_decompressed_buf_len;
}
