#include <iostream> 
#include "Color.h"
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main(int argc, char **argv) {
	unsigned long frameNumber = 0;

	Rect trackWindowLeft = Rect(60, 170, 100, 100);
	float xLeft = 70;
	int cntLeft = 0;
	int cntAutoLeft = 0;

	Rect trackWindowRight = Rect(810, 130, 100, 100);
	float xRight = 830;
	int cntRight = 0;
	int cntAutoRight = 0;


	VideoCapture cap("videoauto.mp4");
	if (!cap.isOpened())
		return -1;
	namedWindow("video", 1);
	Mat frame, greyFrame, erodeFrame, threSholdFrame, dilateFrame,blurFrame;
	Mat fgMaskMOG2;
	Rect trackWindow, trackWindow1;
	RotatedRect trackBoxPrec, trackBoxPrec1;
	trackWindow = Rect(880, 180, 20, 20);
	trackWindow1 = Rect(74, 210, 30, 30);
	Ptr<BackgroundSubtractor> pMOG2;
	pMOG2 = createBackgroundSubtractorMOG2();
	for (;;) {

		frameNumber++;
		cap >> frame;
		if (frame.rows == 0 && frame.cols == 0) {
			cout << "last frame!" << endl;
			break;
		} // 1. convert in grayscale 

		cvtColor(frame, greyFrame, CV_BGR2GRAY);
		// 2. use the background subtraction MOG2 algorithm to obatin the fore ground image 
		pMOG2->apply(greyFrame, fgMaskMOG2);

		// 3. filter the noise by using one or more morphological filters 
		Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
		cv:erode(fgMaskMOG2, erodeFrame, kernel);
		cv::dilate(erodeFrame, dilateFrame, kernel);
		cv::GaussianBlur(dilateFrame, blurFrame, Size(), 2);
		// 4. apply a threshold to better define the fore ground	
		cv::threshold(dilateFrame, threSholdFrame, 50, 255, cv::THRESH_BINARY);
		// 5. if needed, use one ore more morphological filters to improve the fore ground definition
		// 6. find a way to track the cars using the camshift algorithm (hint: implement an object to handle the tracker)
		
		Rect copyTrackWindowLeft = trackWindowLeft;
		Rect copyTrackWindowRight = trackWindowRight;

		RotatedRect TrackBoxLeft = CamShift(threSholdFrame, copyTrackWindowLeft,
			TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 10, 1));
		RotatedRect TrackBoxRight = CamShift(threSholdFrame, copyTrackWindowRight,
			TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 10, 1));

		//se la dimensione di quello che sto traccando è più grande di 0 e mi sto spostando verso destra 
		if (TrackBoxLeft.size.width > 0 && xLeft <= TrackBoxLeft.center.x) {
			cntLeft = 0;
			trackWindowLeft = TrackBoxLeft.boundingRect();

			//assegno a xLeft la posizione del centro del mio trackbox

			xLeft = TrackBoxLeft.center.x;
			if (TrackBoxLeft.center.x > 300) {
				
				//trovo il colore del macchina in maniera approssimativa con la media

				Mat frameAuto = frame(trackWindowLeft);
				Mat filteredAuto = dilateFrame(trackWindowLeft);
				//faccio un clone del frame

				Scalar color = mean(frameAuto, filteredAuto);
				Color colore{ int(color[0]),int(color[1]),int(color[2]) };
				string stri = colore.label();
				std::cout << color;
				Mat clone = frame.clone();
				ostringstream str;

				str << "Frame: " << frameNumber << " , colore: " << stri ;

				putText(clone, str.str(), Point(10, 30), FONT_ITALIC, 0.4, cvScalar(0, 0, 255));

				string name = "./sinistradestra" + to_string(cntAutoLeft) + ".jpg";

				cout << name << endl;

				imwrite(name, clone);

				//riporto la trackwindow in posizione di partenza
				trackWindowLeft = Rect(60, 170, 100, 100);
				xLeft = 70;

				//conteggio delle auto passate
				cntAutoLeft++;
			}
		}

		//in caso venga traccata un'auto proviente dal senso opposto di marcia
		if (cntLeft > 3) {
			trackWindowLeft = Rect(60, 170, 100, 100);
			xLeft = 70;
		}


		// 7. extract the requested information (number of vehicles, number of vehicles for each direction, timestamp, color)
		// 8. save ONE Nj for the vehicle with the requested information written on


		//se la dimensione è maggiore di zero e mi sto spostando verso sinstra
		if (TrackBoxRight.size.width > 0 && xRight >= TrackBoxRight.center.x) {
			cntRight = 0;
			//ritorno la nuova finestra di tracking
			trackWindowRight = TrackBoxRight.boundingRect();

			//aggiorno la x
			xRight = TrackBoxRight.center.x;
			if (TrackBoxRight.center.x < 600) {

				//per il colore della macchina 
				Mat frameAuto = frame(trackWindowRight);
				Mat filteredAuto = dilateFrame(trackWindowRight);
				Scalar color = mean(frameAuto, filteredAuto);
				Color colore{int(color[0]),int(color[1]),int(color[2])};
				string stri=colore.label();
	
				std::cout << color;
				Mat clone = frame.clone();
				std::ostringstream str;
				str << "Frame: " << frameNumber << " , colore: " << stri;



				putText(clone, str.str(), Point(10, 30), FONT_ITALIC, 0.4, cvScalar(0, 255 , 0 ));

				string name = "./destrasinistra" + to_string(cntAutoRight) + ".jpg";

				cout << name << endl;

				imwrite(name, clone);

				//RESETTO CURSORE
				trackWindowRight = Rect(810, 130, 100, 100);
				xRight = 830;

				//contatore di auto da destra a sinistra 
				cntAutoRight++;
			}
		}

		//RESETTO IN CASO CHE PRENDE LA MACCHINA DI SINISTRA
		if (cntRight > 3) {
			trackWindowRight = Rect(810, 120, 100, 100);
			xRight = 830;
		}

		//rettangoli i quali rappresentano la mia track window nel frame
		rectangle(frame, trackWindowLeft, Scalar(0, 0, 255), 3, LINE_AA);
		rectangle(frame, trackWindowRight, Scalar(0, 255, 0), 3, LINE_AA);
		
		//CONTEGGIO FRAME
		cntLeft++;
		cntRight++;

		imshow("Tracking", frame);
		if (waitKey(40) >= 0) break;
	}
	cout << "da sinistra a destra sono passate : " << cntAutoLeft << endl;
	cout << "da destra a sinistra sono passate : " << cntAutoRight << endl;

	return 0;
}