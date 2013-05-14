//AUTHOR:  Ty Heath
//Last Modified:  3.20.2003

#pragma once
#include "..\tkcom\object.h"

//This is a general 64 bit timer class for determining if time periods have passed etc.  The author is personally helping to make TrapperKeeper year 2038 compliant.
class Timer :
	public Object
{
private:
	__time64_t m_mark_time;
public:
	Timer(void);
	~Timer(void);
	bool HasTimedOut(unsigned long time_out); //call to dertermine if "time_out" seconds have elapsed since the last call to refresh or construction
	void Refresh(void); //call to reset the timer
	UINT Diff(Timer &other_timer);
};
