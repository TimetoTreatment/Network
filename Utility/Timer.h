//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// 타이머
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/*
	1. 개요
		- 일반적인 스톱워치

	2. 기능
		- 시작 시각 설정
		- 걸린 시간 반환
*/

#pragma once
#include <chrono>

using namespace std::chrono;

class Timer
{
private:
	steady_clock::time_point mStartClock;

public:
	/* 초기화 */
	void Reset();

	/* 걸린 시간 반환 */
	double GetElapsedTime();
};
