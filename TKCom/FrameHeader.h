// FrameHeader.h

#ifndef FRAME_HEADER_H
#define FRAME_HEADER_H

class FrameHeader
{
public:
	FrameHeader();
	FrameHeader(unsigned int bitrate,double sampling_rate=44.1);
	void Clear();
	void Init();

	int ExtractHeader(CFile *file);	// 1 - read in header, 0 - it was the start of an ID3v1 tag, -1 - error
	bool SetHeader(unsigned char *buf);
	bool IsValid();
	bool IsID3v1Tag();

	bool IsSynched();
	bool IsMPEG1();	
	bool IsLayerIII();
	bool IsProtected();

	unsigned int ReturnBitrate();
	unsigned int ReturnSamplingRate();
	bool IsPadded();
	void IsPadded(bool is_padded);

	unsigned int ReturnFrameSize();



private:
	unsigned char m_data[4];
};

#endif // FRAME_HEADER_H