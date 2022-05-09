//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Ÿ�̸�
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/*
	1. ����
		- �Ϲ����� �����ġ

	2. ���
		- ���� �ð� ����
		- �ɸ� �ð� ��ȯ
*/

#pragma once
#include <chrono>

using namespace std::chrono;

class Timer
{
private:
	steady_clock::time_point mStartClock;

public:
	/* �ʱ�ȭ */
	void Reset();

	/* �ɸ� �ð� ��ȯ */
	double GetElapsedTime();
};
