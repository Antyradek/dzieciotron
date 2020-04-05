#include <iostream>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "exceptions.hpp"

int main()
{
	std::cout << "Dzieńdoberek" << std::endl;
	
	cv::VideoCapture videoCapture;
	videoCapture.setExceptionMode(true);
	videoCapture.open(0);
	
	cv::Mat oneFrame;
	
	//odczyt
	videoCapture >> oneFrame;
	if(oneFrame.empty())
	{
		throw(CameraError("Pusty obraz"));
	}
	
	const std::string windowName = "Podgląd kamery";
	cv::namedWindow(windowName);
	
	while(true)
	{
		videoCapture >> oneFrame;
		
		//progowanie
		cv::Mat binaryFrame;
		cv::threshold(oneFrame, binaryFrame, 30, 255, cv::ThresholdTypes::THRESH_BINARY);
		
		//wyświetl
		cv::imshow(windowName, binaryFrame);
		
		//steruje FPS
		if(cv::waitKey(5) >= 0)
		{
			break;
		}
	}
	
}
