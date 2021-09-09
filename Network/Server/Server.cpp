#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include "../../Utility/TCP.h"

using namespace std;
using namespace cv;
using namespace std::chrono_literals;


int main()
{
	TCP* tcp = new TCP("9510");

	int size = 0;

	for (;;)
	{
		switch (tcp->WaitEvent())
		{
		case TCP::WaitEventType::NEWCLIENT:
			tcp->AddClient();
			tcp->Send("ACCEPT",7);
			break;

		case TCP::WaitEventType::MESSAGE:

			if (((string)tcp->ReadMessage()) == "START")
			{
				tcp->WaitEvent();

				size = atoi(tcp->ReadMessage());

				tcp->WaitEvent();
				Mat image(Size(1920, 1080), CV_8UC3, (char*)tcp->ReadBuffer(size));

				imshow("mat", image);
				
				waitKey(0);
				destroyAllWindows();
			}

			break;

		case TCP::WaitEventType::DISCONNECT:
			tcp->CloseClient();
			break;

		}




	}










	return 0;
}