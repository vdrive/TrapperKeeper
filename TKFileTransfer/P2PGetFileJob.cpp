#include "StdAfx.h"
#include "p2pgetfilejob.h"

P2PGetFileJob::P2PGetFileJob( UINT session_id,UINT app_id,const char* hash,const char* local_path)
{
	m_local_path=local_path;
	m_file_hash=hash;
	m_session_id=session_id;
	m_app_id=app_id;
	mb_inited=false;
}

P2PGetFileJob::~P2PGetFileJob(void)
{
}

void P2PGetFileJob::Inited(UINT file_size)
{
	m_file_size=file_size;
	mb_inited=true;
}

bool P2PGetFileJob::IsInited(void)
{
	return mb_inited;
}
