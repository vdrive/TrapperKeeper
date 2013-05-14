#include "StdAfx.h"
#include "syncherfiletransfer.h"

SyncherFileTransfer::SyncherFileTransfer(SyncherSubscriber* subscriber)
{
	p_subscriber=subscriber;
}

SyncherFileTransfer::~SyncherFileTransfer(void)
{
}

void SyncherFileTransfer::GetFileFailed(char* source_ip , char* local_path , char* remote_path, char *reason ){
	p_subscriber->FileDownloadFailed(local_path);
}

void SyncherFileTransfer::RecievedFile(char* source_ip , char* local_path , char* remote_path ){
	p_subscriber->ReceivedFile(local_path,remote_path);
}

void SyncherFileTransfer::P2PRecievedFile(char* local_path , const char* hash, UINT file_size ){
	p_subscriber->ReceivedFile(local_path,hash);
}

//progress is a fraction between 0 and 1.  1=100%=done.  This object will only recieve progress notifications for file transfers initiated via GetFile().
void SyncherFileTransfer::FileProgress(char *source_ip , char *local_path , char *remote_path , float progress ){

}