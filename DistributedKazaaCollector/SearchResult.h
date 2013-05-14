// SearchResult.h

#ifndef SEARCH_RESULT_H
#define SEARCH_RESULT_H

#include <vector>
using namespace std;
/*
enum column_headers
{
	COL_FILENAME,
	COL_SIZE,
	COL_USER,
	COL_TITLE,
	COL_INTEGRITY,
	COL_ETA,
	COL_BANDWIDTH,
	COL_CATEGORY,
	COL_ARTIST,
	COL_MEDIA_TYPE,
	COL_LENGTH,
	COL_QUALITY,
	COL_ALBUM,
	COL_LANGUAGE,
	COL_TYPE,
	COL_RESOLUTION,
	COL_COLORS,
	COL_AUTHOR,
	COL_DATE,
	COL_VERSION,
	COL_DEVELOPER,
	COL_OS
};
*/
class SearchResult
{
public:
	// Public Member Functions
	SearchResult();
	void Clear();

	// Public Data Members
	HTREEITEM m_hTreeItem;
	unsigned int m_ip;
	string m_user;
	string m_title;
	string m_filename;
	unsigned int m_size;
	char m_hash[20];

	string m_artist;
	string m_album;

	unsigned int m_track_number;
	unsigned int m_port;
	unsigned int m_media_type;	// 0 - Unknown, 1 - Audio, 2 - Video
/*
	// Public Member Functions
	SearchResult();
	void Clear();

	static vector<int> ProcessColumnHeaders(vector<string> &columns);
	void ExtractSearchResultFromString(vector<int> &columns,char *str);

	// Public Data Members
	HTREEITEM m_hTreeItem;
	unsigned int m_ip;
	unsigned int m_port;
	char m_hash[20];

	// Everything	: 10 columns
	// Audio		: 12 columns
	// Video		: 13 columns
	// Images		: 13 columns
	// Documents	: 13 columns
	// Software		: 12 columns
	// Playlists	: 10 columns

	string m_filename;		// Everything	Audio	Video	Images	Documents	Software	Playlists
//	string m_size;			// Everything	Audio	Video	Images	Documents	Software	Playlists
	string m_user;			// Everything	Audio	Video	Images	Documents	Software	Playlists
	string m_title;			// Everything	Audio	Video	Images	Documents	Software	Playlists
	string m_integrity;		// Everything	Audio	Video	Images	Documents	Software	Playlists
	string m_eta;			// Everything	Audio	Video	Images	Documents	Software	Playlists
	string m_bandwidth;		// Everything	Audio	Video	Images	Documents	Software	Playlists
	string m_category;		// Everything	Audio	Video	Images	Documents	Software	Playlists
	string m_artist;		// Everything	Audio	Video	Images							Playlists
	string m_media_type;	// Everything
	string m_length;		//				Audio	Video
	string m_quality;		//				Audio
	string m_album;			//				Audio			Images							Playlists
	string m_language;		//						Video			Documents
	string m_type;			//						Video	Images	Documents
	string m_resolution;	//						Video	Images
	string m_colors;		//								Images
	string m_author;		//										Documents
	string m_date;			//										Documents	Software
	string m_version;		//										Documents	Software
	string m_developer;		//													Software
	string m_os;			//													Software

	unsigned int m_size;	// bytes
*/
};

#endif // SEARCH_RESULT_H