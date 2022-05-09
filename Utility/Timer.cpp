#include "Timer.h"

void Timer::Reset()
{
	mStartClock = steady_clock::now();		// ���� �ð��� ���� �ð����� �ʱ�ȭ
}


double Timer::GetElapsedTime()
{
	return duration<double> (steady_clock::now() - mStartClock).count();	// �ɸ� �ð��� �� ������ ��ȯ
}
