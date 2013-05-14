// SearchResult.cpp

#include "stdafx.h"
#include "SearchResult.h"

//
//
//
SearchResult::SearchResult()
{
	Clear();
}

//
//
//
void SearchResult::Clear()
{
	m_hTreeItem=NULL;
	m_ip=0;
	m_user.clear();
	m_title.clear();
	m_filename.clear();
	m_size=0;
	memset(m_hash,0,sizeof(m_hash));

	m_artist.clear();
	m_album.clear();

	m_track_number=0;
	m_port=0;
	m_media_type=0;
/*
	m_hTreeItem=NULL;
	m_ip=0;
	m_port=0;
	memset(m_hash,0,sizeof(m_hash));

	m_filename.clear();
	m_user.clear();
	m_title.clear();
	m_integrity.clear(); 
	m_eta.clear(); 
	m_bandwidth.clear(); 
	m_category.clear(); 
	m_artist.clear(); 
	m_media_type.clear(); 
	m_length.clear(); 
	m_quality.clear(); 
	m_album.clear(); 
	m_language.clear(); 
	m_type.clear(); 
	m_resolution.clear(); 
	m_colors.clear(); 
	m_author.clear(); 
	m_date.clear(); 
	m_version.clear(); 
	m_developer.clear(); 
	m_os.clear(); 

	m_size=0;	// bytes
*/
}
/*
//
//
//
vector<int> SearchResult::ProcessColumnHeaders(vector<string> &columns)
{
	unsigned int i;
	vector<int> vals;

	for(i=0;i<columns.size();i++)
	{
		// Trim whitespace from column header and compare
		CString cstring=columns[i].c_str();
		cstring.Trim();
		char *buf=new char[cstring.GetLength()+1];
		strcpy(buf,cstring);

		int val=-1;	// unknown
		if(strcmp(buf,"Filename")==0)
		{
			val=COL_FILENAME;
		}
		else if(strcmp(buf,"Integrity")==0)
		{
			val=COL_INTEGRITY;
		}
		else if(strcmp(buf,"Artist")==0)
		{
			val=COL_ARTIST;
		}
		else if(strcmp(buf,"Size")==0)
		{
			val=COL_SIZE;
		}
		else if(strcmp(buf,"User")==0)
		{
			val=COL_USER;
		}
		else if(strcmp(buf,"ETA")==0)
		{
			val=COL_ETA;
		}
		else if(strcmp(buf,"Bandwidth")==0)
		{
			val=COL_BANDWIDTH;
		}
		else if(strcmp(buf,"Media Type")==0)
		{
			val=COL_MEDIA_TYPE;
		}
		else if(strcmp(buf,"Category")==0)
		{
			val=COL_CATEGORY;
		}
		else if(strcmp(buf,"Title")==0)
		{
			val=COL_TITLE;
		}
		else if(strcmp(buf,"Album")==0)
		{
			val=COL_ALBUM;
		}
		else if(strcmp(buf,"Quality")==0)
		{
			val=COL_QUALITY;
		}
		else if(strcmp(buf,"Length")==0)
		{
			val=COL_LENGTH;
		}
		else if(strcmp(buf,"Language")==0)
		{
			val=COL_LANGUAGE;
		}
		else if(strcmp(buf,"Resolution")==0)
		{
			val=COL_RESOLUTION;
		}
		else if(strcmp(buf,"Colours")==0)
		{
			val=COL_COLORS;
		}
		else if(strcmp(buf,"Author")==0)
		{
			val=COL_AUTHOR;
		}
		else if(strcmp(buf,"Date")==0)
		{
			val=COL_DATE;
		}
		else if(strcmp(buf,"Version")==0)
		{
			val=COL_VERSION;
		}
		else if(strcmp(buf,"Developer")==0)
		{
			val=COL_DEVELOPER;
		}
		else if(strcmp(buf,"OS")==0)
		{
			val=COL_OS;
		}
		else if(strcmp(buf,"Type")==0)
		{
			val=COL_TYPE;
		}

		delete [] buf;

		vals.push_back(val);
	}

	return vals;
}
*/
/*
//
//
//
void SearchResult::ExtractSearchResultFromString(vector<int> &columns,char *str)
{
	int i;

	// Make copy of string
	char *buf=new char[strlen(str)+1];
	strcpy(buf,str);

	char *ptr=NULL;
	for(i=(int)(columns.size()-1);i>=0;i--)
	{
		if(i>0)
		{
			ptr=strrchr(buf,0xb6);
			*ptr='\0';
			ptr++;
		}
		else	// i==0
		{
			ptr=buf;
		}

		switch(columns[i])
		{
			case COL_FILENAME:
			{
				m_filename=ptr;
				break;
			}
			case COL_SIZE:
			{
//				m_size=ptr;
				break;
			}
			case COL_USER:
			{
				m_user=ptr;
				break;
			}
			case COL_TITLE:
			{
				m_title=ptr;
				break;
			}
			case COL_INTEGRITY:
			{
				m_integrity=ptr;
				break;
			}
			case COL_ETA:
			{
				m_eta=ptr;
				break;
			}
			case COL_BANDWIDTH:
			{
				m_bandwidth=ptr;
				break;
			}
			case COL_CATEGORY:
			{
				m_category=ptr;
				break;
			}
			case COL_ARTIST:
			{
				m_artist=ptr;
				break;
			}
			case COL_MEDIA_TYPE:
			{
				m_media_type=ptr;
				break;
			}
			case COL_LENGTH:
			{
				m_length=ptr;
				break;
			}
			case COL_QUALITY:
			{
				m_quality=ptr;
				break;
			}
			case COL_ALBUM:
			{
				m_album=ptr;
				break;
			}
			case COL_LANGUAGE:
			{
				m_language=ptr;
				break;
			}
			case COL_TYPE:
			{
				m_type=ptr;
				break;
			}
			case COL_RESOLUTION:
			{
				m_resolution=ptr;
				break;
			}
			case COL_COLORS:
			{
				m_colors=ptr;
				break;
			}
			case COL_AUTHOR:
			{
				m_author=ptr;
				break;
			}
			case COL_DATE:
			{
				m_date=ptr;
				break;
			}
			case COL_VERSION:
			{
				m_version=ptr;
				break;
			}
			case COL_DEVELOPER:
			{
				m_developer=ptr;
				break;
			}
			case COL_OS:
			{
				m_os=ptr;
				break;
			}
			default:
			{
				// Unknown
				break;
			}
		}
	}

	delete [] buf;
}
*/