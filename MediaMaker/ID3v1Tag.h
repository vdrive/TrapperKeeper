// ID3v1Tag.h

#ifndef ID3V1_TAG_H
#define ID3V1_TAG_H

class ID3v1Tag
{
public:
	ID3v1Tag();
	void Clear();
	void Init();

//	int ExtractTag(char *filename);	// 1 - read in tag, 0 - there is no tag in the file, -1 - error opening file
//	bool IsValid();

	string ReturnTitle();
	string ReturnArtist();
	string ReturnAlbum();
	string ReturnYear();
	string ReturnComment();
	string ReturnGenre();

	void SetTitle(char *title);
	void SetArtist(char *artist);
	void SetAlbum(char *album);
	void SetYear(char *year);
	void SetComment(char *comment);
	void SetGenre(unsigned char genre);

private:
	// 128 bytes
	unsigned char m_tag[3];		// "TAG"
	unsigned char m_title[30];
	unsigned char m_artist[30];
	unsigned char m_album[30];
	unsigned char m_year[4];
	unsigned char m_comment[30];
	unsigned char m_genre[1];
};

#endif // ID3V1_TAG_H