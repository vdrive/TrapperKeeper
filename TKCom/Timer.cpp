#include "StdAfx.h"
#include "timer.h"

Timer::Timer(void)
{
	_time64( &m_mark_time );
}

Timer::~Timer(void)
{
}

//Call this to determine if the specified time_out period has elapsed since the last call to Refresh()
bool Timer::HasTimedOut(unsigned long time_out)
{
	__time64_t cur_time;
	_time64( &cur_time );
	if(cur_time-time_out>m_mark_time)
		return true;
	else return false;
}

//call this to set the mark time to the current time.
void Timer::Refresh(void)
{
	_time64( &m_mark_time );
}

UINT Timer::Diff(Timer &other_timer){
	if(m_mark_time>other_timer.m_mark_time)
		return (UINT)(m_mark_time-other_timer.m_mark_time);
	else
		return (UINT)(other_timer.m_mark_time-m_mark_time);
}
