// ID3v2TagFrameHeader.h

#ifndef ID3V2_TAG_FRAME_HEADER_H
#define ID3V2_TAG_FRAME_HEADER_H

class ID3v2TagFrameHeader
{
public:
	ID3v2TagFrameHeader();
	void Clear();

	void ExtractFrameHeaderPointer(unsigned char *ptr);
	
	unsigned char *ReturnFrameIDPointer();
	unsigned int ReturnFrameSize();

	void SetFrameID(char *id);
	void SetFrameSize(unsigned int size);

private:
	unsigned char m_frame_id[4];
	unsigned char m_size[4];	// not synch safe integers
	unsigned char m_flags[2];
};

#endif // ID3V2_TAG_FRAME_HEADER_H