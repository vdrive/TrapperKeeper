// CompressionModule.h
#pragma once

class CompressionModule
{
public:
	CompressionModule();
	~CompressionModule();

	void CompressBuffer(byte* buf,unsigned int buf_len);
	unsigned char *ReturnCompressedBuffer();
	unsigned int ReturnCompressedBufferLength();

//	void DecompressBuffer(unsigned char *buf,unsigned int buf_len);
	unsigned char *ReturnDecompressedBuffer();
	unsigned int ReturnDecompressedBufferLength();

	void KeywordsUpdated();

private:
	unsigned char *p_compressed_buf;
	unsigned int m_compressed_buf_len;

	unsigned char *p_decompressed_buf;
	unsigned int m_decompressed_buf_len;
};