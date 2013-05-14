#pragma once
#include "..\tkcom\tkcominterface.h"

class SwarmerDownloader;
class SwarmerDownloaderComInterface :	public TKComInterface
{
public:
	SwarmerDownloaderComInterface(void);
	~SwarmerDownloaderComInterface(void);

	void InitParent(SwarmerDownloader * parent);

	protected:
	void DataReceived(char *source_ip, void *data, UINT data_length);

	private:

	SwarmerDownloader * p_parent;
};


