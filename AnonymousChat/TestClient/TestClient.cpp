#include <iostream>
#include <string>
#include <thread>
#include "../../Utility/TCP.h"

using namespace std;
using namespace chrono_literals;


int main()
{
	TCP* tcp = new TCP("9510", "127.0.0.1");
	string cinBuffer;
	string message;
	string userName;

	cout << "**************************\n";
	cout << "* Anonymous Chat Program *\n";
	cout << "**************************\n" << endl;

	cout << "[SERVER] Enter your nickname : ";
	cin >> userName;

	tcp->SendMsg("[USERNAME]" + userName);

	thread cinNonBlockThread([&cinBuffer]() {
		for (;;)
		{
			string line;
			getline(cin, line);
			cinBuffer = line;
		}}
	);

	for (bool exit = false; !exit;)
	{
		// SendMsg
		message = cinBuffer;

		if (!message.empty())
		{
			tcp->SendMsg(message);

			message.clear();
			cinBuffer.clear();
		}
		else if (message == "EXIT")
		{
			exit = true;
			break;
		}

		// Receive
		switch (tcp->WaitEvent(100))
		{
		case TCP::WaitEventType::MESSAGE:
			cout << tcp->ReadMsg();
			break;

		case TCP::WaitEventType::DISCONNECT:
			exit = true;
			break;
		}
	}

	delete tcp;
	cinNonBlockThread.detach();

	return 0;
}