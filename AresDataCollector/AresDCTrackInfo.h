#pragma once
#include "..\tkcom\object.h"

class AresDCTrackInfo : public Object
{
public:
	AresDCTrackInfo(const char* name,int track_number,bool b_single);
	~AresDCTrackInfo(void);

	string m_name;
	int m_track_number;
	bool mb_single;

	vector <string> mv_keywords;
};
