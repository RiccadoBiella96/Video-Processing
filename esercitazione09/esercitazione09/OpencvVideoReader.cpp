#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	VideoCapture cap("./../videoauto.mp4");
	if (!cap.isOpened())
		return -1;

	namedWindow("video", 1);
	Mat frame;
	for (;;)
	{
		cap >> frame;
		if (frame.rows == 0 && frame.cols == 0){
			cout << "last frame!" << endl;
			break;
		}

		imshow("video", frame);
		if (waitKey(40) >= 0) break;
	}

	return 0;
}