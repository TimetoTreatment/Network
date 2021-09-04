#include <iostream>
#include <string>
#include <thread>
#include "../../Utility/TCP.h"

using namespace std;
using namespace chrono_literals;


int main()
{
	TCP* tcp = new TCP(9510, "127.0.0.1");
	string cinBuffer;
	string message;
	string userName;

	cout << "**************************\n";
	cout << "* Anonymous Chat Program *\n";
	cout << "**************************\n" << endl;

	cout << "[SERVER] Enter your nickname : ";
	cin >> userName;

	tcp->Send("[USERNAME]" + userName);

	thread cinNonBlockThread([&cinBuffer]() {
		for (;;)
		{
			string line;
			getline(cin, line);
			cinBuffer = line;
		}});

	for (bool exit = false; !exit;)
	{
		// Send
		message = cinBuffer;

		if (!message.empty())
		{
			tcp->Send(message);

			message.clear();
			cinBuffer.clear();
		}

		// Receive
		switch (tcp->WaitEvent(100))
		{
		case TCP::WaitEventType::MESSAGE:
			cout << tcp->ReadMessage();
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