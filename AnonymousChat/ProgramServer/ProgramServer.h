#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <map>
#include "../../Utility/TCP.h"
#include "../../Utility/Random.h"

using namespace std;
using namespace chrono_literals;


class ProgramServer
{
private:

	struct Operation
	{
		Operation(string line)
		{
			size_t whiteSpace = line.find(' ');

			command = line.substr(0, whiteSpace);

			if (whiteSpace != string::npos)
				argument = line.substr(whiteSpace + 1);
		}

		string command;
		string argument;
	};

	TCP* tcp = nullptr;
	map<string, string> users;
	int anonUserCount = 0;

public:

	ProgramServer(string port)
	{
		tcp = new TCP(port);
	}

	~ProgramServer()
	{
		delete tcp;
	}

	void Run()
	{
		cout << "****************" << endl;
		cout << "* Server Start *" << endl;
		cout << "****************" << endl;

		for (bool exit = false; !exit;)
		{
			switch (tcp->WaitEvent())
			{
			case TCP::WaitEventType::NEWCLIENT:

				tcp->AddClient();
				tcp->SendMsg("[SERVER] Your SocketID is #" + tcp->ReadSenderID(), TCP::SendTo::EVENT_SOURCE);
				tcp->SendMsg("[SERVER] #" + tcp->ReadSenderID() + " is connected", TCP::SendTo::OTHERS);
				tcp->SendMsg("까악!!!", TCP::SendTo::EVENT_SOURCE);
				cout << "[SERVER] #" + tcp->ReadSenderID() + " is connected." << endl;
				break;

			case TCP::WaitEventType::DISCONNECT:

				tcp->CloseClient();
				tcp->SendMsg("[SERVER] " + users[tcp->ReadSenderID()] + "님이 방을 나가셨습니다.", TCP::SendTo::OTHERS);
				users.erase(tcp->ReadSenderID());
				cout << "[SERVER] #" + tcp->ReadSenderID() + " is disconnected." << endl;
				break;

			case TCP::WaitEventType::MESSAGE:

				string msg = tcp->ReadMsg();

				if (msg.substr(0, 2) == "--")
				{
					Operation operation(msg);

					if (operation.command == "--shutdown")
						exit = true;
					else if (operation.command == "--username")
					{
						string userName = operation.argument;

						if (userName.empty() || userName.find("SERVER") != string::npos)
						{
							anonUserCount++;
							userName = "익명" + to_string(anonUserCount);
						}

						if (users.find(tcp->ReadSenderID()) == users.end())
							users.emplace(tcp->ReadSenderID(), userName);
						else
							users[tcp->ReadSenderID()] = userName;
					}
					else if (operation.command == "--dice")
					{
						string messageOut = "[SERVER] " + users[tcp->ReadSenderID()] + "님이 주사위를 굴려 " + std::to_string(Random::Integer(0, 100)) + "이(가) 나왔습니다!";
						tcp->SendMsg(messageOut, TCP::SendTo::ALL);
					}
				}
				else
				{
					tcp->SendMsg("[" + users[tcp->ReadSenderID()] + "] : " + msg, TCP::SendTo::ALL);
					cout << "[CLIENT] #" + tcp->ReadSenderID() + " send a message" << endl;
				}

				break;
			}

			this_thread::sleep_for(100us);
		}

		tcp->SendMsg("[SERVER] Server closed", TCP::SendTo::ALL);
	}
};
