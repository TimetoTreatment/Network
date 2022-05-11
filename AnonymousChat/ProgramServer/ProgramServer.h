#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <map>
#include <list>

#include "Operation.h"

#include "../../Utility/TCP.h"
#include "../../Utility/Random.h"


class ProgramServer
{
private:

	TCP* tcp = nullptr;
	std::map<std::string, std::string> users;
	int anonUserCount = 0;

	std::list<std::string> msgHistoryList;
	std::string msgHistory;


public:

	ProgramServer(std::string port)
	{
		tcp = new TCP(port);
	}

	~ProgramServer()
	{
		delete tcp;
	}

	void Run();

	
};
