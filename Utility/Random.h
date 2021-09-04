//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// 난수 반환
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/*
	1. 개요
		- 고품질의 난수 생성 및 반환

	2. 기능
		- 닫힌 구간 내 무작위 정수 반환
		- 닫힌 구간 내 무작위 실수 반환 (필요하면 구현 예정)
*/

#pragma once
#include <random>

class Random
{
private:

	static std::random_device mDevice;
	static std::mt19937 mEngine;

public:

	/* 범위 내 무작위 정수 반환 */
	static int Integer(int rangeStart, int rangeEnd);


private:
	static Random* sInstance;

public:
	static Random* Instance();
	static void Release();
};
