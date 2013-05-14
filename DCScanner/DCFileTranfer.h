#pragma once

//class will transfer a file
class DCFileTranfer
{
public:
	DCFileTranfer(void);
	~DCFileTranfer(void);
	char* GetChunk();
	bool GetFile();

};
