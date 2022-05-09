#include "Timer.h"

void Timer::Reset()
{
	mStartClock = steady_clock::now();		// 시작 시각을 현재 시각으로 초기화
}


double Timer::GetElapsedTime()
{
	return duration<double> (steady_clock::now() - mStartClock).count();	// 걸린 시간을 초 단위로 반환
}
