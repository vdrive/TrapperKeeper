#pragma once

//
// DataPath is used by DataFile to define the path of each file in the torrent
//

class BString;
class DataPath
{
public:
	DataPath(const string &rDir);
	virtual ~DataPath(void);

	string GetDir(void);
	BString *GetBDir(void);

protected:
	string m_dir;
	BString *m_pBDir;
};
