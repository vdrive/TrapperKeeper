// ID3v2Tag.h

#ifndef ID3V2_TAG_H
#define ID3V2_TAG_H

#include "ID3v2TagHeader.h"

class ID3v2Tag
{
public:
	ID3v2Tag();
	void Clear();

//	bool ExtractTag(char *filename);
	bool ExtractTag(CFile *file);
	unsigned int GetTagLength();

	int WriteTagToFile(CFile *file);

	string ReturnTitle();
	string ReturnArtist();
	string ReturnAlbum();
	string ReturnTrack();

	void SetTitle(char *title);
	void SetArtist(char *artist);
	void SetAlbum(char *album);
	void SetTrack(char *track);

	void SetComment(char *comment);
	void SetYear(char *year);

private:
	ID3v2TagHeader m_hdr;
//	unsigned char *p_data;
//	unsigned int m_len;

	string m_artist;
	string m_album;
	string m_title;
	string m_track;

	string m_comment;
	string m_year;

	bool ProcessTag(unsigned char *data,unsigned int len);
};

#endif // ID3V2_TAG_H