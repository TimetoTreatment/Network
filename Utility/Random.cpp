#include "Random.h"

std::random_device Random::mDevice;					// 시드 생성
std::mt19937 Random::mEngine(Random::mDevice());	// 메르센-트위스터 알고리즘 사용

int Random::Integer(int rangeStart, int rangeEnd)
{
	std::uniform_int_distribution<int> distribution(rangeStart, rangeEnd);	// 범위 설정
	return distribution(mEngine);											// 무작위 정수 반환
}


Random* Random::sInstance = nullptr;

Random* Random::Instance()
{
	if (sInstance == nullptr)
		sInstance = new Random;

	return sInstance;
}

void Random::Release()
{
	delete sInstance;
	sInstance = nullptr;
}