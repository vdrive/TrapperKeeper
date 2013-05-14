#pragma once
#include "tkfiletransferinterface.h"
#include "SyncherSubscriber.h"
class SyncherSubscriber;
class SyncherFileTransfer :
	public TKFileTransferInterface
{
private:
	SyncherSubscriber *p_subscriber;
public:
	SyncherFileTransfer(SyncherSubscriber* subscriber);
	~SyncherFileTransfer(void);

	void GetFileFailed(char* source_ip , char* local_path , char* remote_path, char *reason );

	void RecievedFile(char* source_ip , char* local_path , char* remote_path );

	//received p2p file
	void P2PRecievedFile(char* local_path , const char* hash, UINT file_size );

	//progress is a fraction between 0 and 1.  1=100%=done.  This object will only recieve progress notifications for file transfers initiated via GetFile().
	void FileProgress(char *source_ip , char *local_path , char *remote_path , float progress );
};
