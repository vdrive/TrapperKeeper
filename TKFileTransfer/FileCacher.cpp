#include "StdAfx.h"
#include "filecacher.h"

FileCacher::FileCacher(void)
{
}

FileCacher::~FileCacher(void)
{
}

UINT FileCacher::Run()
{
	Timer timer;
	while(!this->b_killThread){
		if(timer.HasTimedOut(60)){
			CSingleLock lock(&this->m_data_lock,TRUE);
			for(UINT i=0;i<mv_file_segments.Size();i++){
				FileSegment *file=(FileSegment*)mv_file_segments.Get(i);
				if(file->IsOld()){
					mv_file_segments.Remove(i);
					i--;
				}
			}
			timer.Refresh();  //reset it so it won't time out for another 60 seconds.
		}
		Sleep(100);
	}
	return 0;
}

// Caches a piece of a file
void FileCacher::AddSegment(FileCacher::FileSegment *file_segment)
{
	//TRACE("FileCacher::AddSegment() Adding segment file=%s length=%d offset=%d\n",file_name,data_length,offset);
	CSingleLock lock(&this->m_data_lock,TRUE);
	mv_file_segments.Add(file_segment);  //push one on to the end of it
	if(mv_file_segments.Size()>300){
		mv_file_segments.Remove(0);
	}
}

bool FileCacher::GetSegment(const char* file_name, UINT offset,Vector &v_buffer)
{
	//TRACE("FileCacher::GetSegment() Getting segment file=%s length=%d offset=%d\n",file_name,data_length,offset);
	CSingleLock lock(&this->m_data_lock,TRUE);
	for(UINT i=0;i<mv_file_segments.Size();i++){
		FileSegment *file=(FileSegment*)mv_file_segments.Get(i);
		if(file->IsSegment(file_name,offset)){
			file->Hit();
			v_buffer.Add(file);
			//memcpy(data_buffer,file->m_data,file->m_length);
			//data_length=file->m_length;
			//TRACE("FileCacher::GetSegment() returning true\n");
			return true;
		}
	}

	//TRACE("FileCacher::GetSegment() returning false\n");
	return false;
}

UINT FileCacher::GetCount(void)
{
	return mv_file_segments.Size();
}
