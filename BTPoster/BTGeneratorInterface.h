#pragma once

class BTGeneratorInterface
{
public:
	BTGeneratorInterface(void);
	~BTGeneratorInterface(void);

	string GenerateTorrentFile(string filename, string name, string size);
	int GetTorrentFile(string id, char * torrent);
	bool IsReady(string id);
	bool GetInfo(string id, string * p_filename, string * p_name, int * p_size);

};
