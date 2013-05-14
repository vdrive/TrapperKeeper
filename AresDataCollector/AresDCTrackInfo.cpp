#include "StdAfx.h"
#include "aresdctrackinfo.h"

AresDCTrackInfo::AresDCTrackInfo(const char* name,int track_number,bool b_single)
{
	m_name=name;
	m_track_number=track_number;
	mb_single=b_single;
}

AresDCTrackInfo::~AresDCTrackInfo(void)
{
}
