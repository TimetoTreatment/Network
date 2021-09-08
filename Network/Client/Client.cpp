#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include "../../Utility/TCP.h"

using namespace std;
using namespace cv;


int main()
{
	TCP* tcp = new TCP("9510", "127.0.0.1");

	tcp->WaitEvent(); // ACCEPT

	for (;;)
	{
		string filename;
		cin >> filename;

		Mat img = imread(filename + ".png");
		if (img.empty())
		{
			cout << "img.empty()" << endl;
			abort();
		}

		int size = img.total() * img.channels();

		tcp->Send("START", 6);
		tcp->Send("6220800", 8);
		tcp->Send((const char*)img.data, size);
	}

	return 0;
}