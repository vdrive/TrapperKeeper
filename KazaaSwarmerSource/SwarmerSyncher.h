#pragma once
#include "..\tksyncher\tksyncherinterface.h"

class SwarmerSource;

class MediaSyncher :
	public TKSyncherInterface
{
public:
	MediaSyncher(void);
	~MediaSyncher(void);

	void InitParent(SwarmerSource * parent);
	void KickOffSyncher();

private:



	SwarmerSource * p_parent;
};
