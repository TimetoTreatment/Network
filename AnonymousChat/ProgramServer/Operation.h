#pragma once
#include <string>
#include <vector>
#include <sstream>


class Operation
{
private:


public:

	std::string command;
	std::vector<std::string> args;

	Operation(std::string line)
	{
		size_t whiteSpace = line.find(' ');

		command = line.substr(0, whiteSpace);

		if (whiteSpace != line.npos)
		{
			std::istringstream iss(line.substr(whiteSpace + 1));
			std::string argv;

			while (getline(iss, argv, ' '))
				args.emplace_back(argv);
		}
	}
};

