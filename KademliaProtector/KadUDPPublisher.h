#pragma once
#include <afxsock.h>		// MFC socket extensions
#include "ThreadedObject.h"
#include "..\tkcom\vector.h"
#include "KadFile.h"
#include "KadContact.h"
#include "UInt128.h"
#include "tag.h"
#include "..\tkcom\buffer2000.h"
#include "..\MetaMachineSpoofer\DonkeyFile.h"

#include <afxmt.h>

using namespace Kademlia;

class KadUDPPublisher : protected ThreadedObject
{
private:
	CCriticalSection m_lock;
	CAsyncSocket *mp_socket;
	Vector mv_files_to_publish;
	Vector mv_decoys_to_publish;
	UINT m_publish_index;
	UINT m_decoy_publish_index;

	void WriteEmuleIntTag(int val,byte cmd,Buffer2000& output);

	void WriteEmuleByteTag(byte val,byte cmd,Buffer2000& output);

	void WriteEmuleWordTag(WORD val,byte cmd,Buffer2000& output);

	void WriteEmuleStringTag(const char* val,byte cmd,Buffer2000& output);

	string GetExtension(const char* file_name);
	const char* GetCodec(const char* extension,const char* file_name);
	int GetBitrate(const char* extension,const char* file_name);
	int CalculateMediaLength(const char* extension,const char* file_name,unsigned int file_size);
	const char* GetFileType(const char* extension);

public:
	KadUDPPublisher(void);
	~KadUDPPublisher(void);

	
protected:
	UINT Run(void);
	

public:
	void AddFileToPublishFileList(KadFile* file);
	void StartPublishing(void);
	void StopPublishing(void);
	void GetNextFileToPublish(Vector& v_files);
	void GetNextDecoyToPublish(Vector& v_files);
	void PublishFile(KadFile* file, CUInt128* key,KadContact *dest);
	void SetSocket(CAsyncSocket* p_socket);
	
protected:
	void PublishSource(byte *hash,KadContact *dest);
public:
	void UnpublishFiles(void);
	void UnpublishDecoys(void);
	void AddDecoyToPublishFileList(DonkeyFile* df);
private:
	void PublishNextFile(void);
public:
	void PublishNextDecoy(void);
};
