// ID3v2TagHeader.h

#ifndef ID3V2_TAG_HEADER_H
#define ID3V2_TAG_HEADER_H

class ID3v2TagHeader
{
public:
	ID3v2TagHeader();

	void Clear();
	int ExtractTag(const char *filename);	// 1 - read in tag, 0 - there is no tag in the file, -1 - error opening file
	bool IsValid();

	string ReturnVersion();
	
	bool IsUnsynchronizationUsed();
	bool IsExtendedHeaderPresent();
	bool IsTagExperimental();
	bool IsFooterPresent();
	
	unsigned int GetBodyLengthValue();
	string GetBodyLength();
	unsigned int GetTagLengthValue();
	string GetTagLength();

private:
	// 10 bytes
	unsigned char m_tag[3];	// "ID3"
	unsigned char m_version[2];
	unsigned char m_flags[1];
	unsigned char m_size[4];	// synch-safe integers
};

#endif // ID3V2_TAG_HEADER_H