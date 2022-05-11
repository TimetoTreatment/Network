#include "ProgramServer.h"


void ProgramServer::Run()
{
	using namespace std;
	using string = std::string;

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

			msgHistory.clear();

			for (string& msg : msgHistoryList)
				msgHistory += msg + '\n';

			tcp->SendMsg(msgHistory, TCP::SendTo::EVENT_SOURCE);

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
					string userName = operation.args.empty() ? "" : operation.args[0];

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
				string msgWithUsername = "[" + users[tcp->ReadSenderID()] + "] : " + msg;

				tcp->SendMsg(msgWithUsername, TCP::SendTo::ALL);
				cout << "[CLIENT] #" + tcp->ReadSenderID() + " send a message" << endl;

				if (msgHistoryList.size() > 500)
					msgHistoryList.pop_front();

				msgHistoryList.emplace_back(msgWithUsername);
			}

			break;
		}

		this_thread::sleep_for(1ms);
	}

	tcp->SendMsg("[SERVER] Server closed", TCP::SendTo::ALL);
}