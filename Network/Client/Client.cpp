#include <iostream>
#include <string>
#include <thread>
#include "../../Utility/TCP.h"

using namespace std;
using namespace chrono_literals;





int main()
{
	string input;


	thread t([&input]() {for (;;) cin >> input; });


	TCP* tcp = new TCP(9510, "127.0.0.1");

	for (bool exit = false; !exit;)
	{
		switch (tcp->WaitEvent(100))
		{
		case TCP::WaitEventType::MESSAGE:
			cout << tcp->ReadMessage();
			break;

		case TCP::WaitEventType::DISCONNECT:
			exit = true;
			break;
		}

		if (!input.empty())
		{
			tcp->Send(input);
			input.clear();
		}
	}

	delete tcp;
	t.detach();


	return 0;
}