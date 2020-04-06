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
	videoCapture.open(2);
	
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
		
		//przerób na inną przestrzeń
		cv::Mat otherSpaceFrame;
		cv::cvtColor(oneFrame, otherSpaceFrame, cv::COLOR_BGR2HLS_FULL);
		
		//podziel na składowe
		cv::Mat channels[otherSpaceFrame.channels()];
		cv::split(otherSpaceFrame, channels);
		cv::Mat grayscaleFrame = channels[1];
		
		//progowanie
		cv::Mat binaryFrame;
		cv::threshold(grayscaleFrame, binaryFrame, 245, 255, cv::ThresholdTypes::THRESH_BINARY);
		
		//zamknięcie i otwarcie
		const int kernelSize = 3;
		cv::morphologyEx(binaryFrame, binaryFrame, cv::MorphTypes::MORPH_CLOSE, cv::getStructuringElement(cv::MorphShapes::MORPH_ELLIPSE, cv::Size(kernelSize, kernelSize)));
		cv::morphologyEx(binaryFrame, binaryFrame, cv::MorphTypes::MORPH_OPEN, cv::getStructuringElement(cv::MorphShapes::MORPH_ELLIPSE, cv::Size(kernelSize, kernelSize)));
		
		//wyświetl
		cv::imshow(windowName, binaryFrame);
		
		//steruje FPS
		if(cv::waitKey(5) >= 0)
		{
			break;
		}
	}
	
}
