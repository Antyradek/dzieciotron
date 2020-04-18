#include <iostream>
#include <algorithm>

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
		
		//kontury
		//NOTE kontury to zestawy zestawów punktów 2D, które są wokół znalezionego obszaru
		std::vector<std::vector<cv::Point>> contours;
		//NOTE zbiór ustalający kolejność konturów, każdy wektor zawiera 4 elementy oznaczające indeksy: kolejny, poprzedni, rodzica i dziecko
		std::vector<cv::Vec4i> hierarchy;
		//TODO dokładniejszy sposób obliczeń dla obwarzanków
		//NOTE druga część to aproksymacja czy trzymać wszystkie piksele czy tylko otoczkę
		cv::findContours(binaryFrame, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
		
		//narysuj kontury
		srand(0);
		cv::Mat displayBinary(binaryFrame);
		cv::cvtColor(displayBinary, displayBinary, cv::COLOR_GRAY2BGR);
		//tablica bierze kolejne kontury w hierarchii
// 		for(int idx = 0; idx >= 0; idx = hierarchy[idx][0])
		//tablica do wypełnienia środkami konturów
		std::vector<cv::Point2f> contourCenters(contours.size());
		for(size_t idx = 0; idx < contours.size(); idx++)
		{
			cv::Scalar color(rand() % 191, rand() % 191, rand() % 191);
			drawContours(displayBinary, contours, idx, color, -1);
			
			cv::Moments moments = cv::moments(contours[idx], true);
			double posX = moments.m10 / moments.m00;
			double posY = moments.m01 / moments.m00;
			contourCenters[idx] = cv::Point2f(posX, posY);
			cv::drawMarker(oneFrame, contourCenters[idx], color, cv::MARKER_CROSS, 5);
		}
		
		//klastrowanie
		if(contourCenters.size() > 0)
		{
			//TODO liczba klastrów od 1 do 4
			//TODO liczba powtórzeń jakaś
			cv::Mat bestLabels;
			std::vector<cv::Point2f> clusterCenters;
			cv::kmeans(contourCenters, std::min<int>(2, contourCenters.size()), bestLabels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0), 5, cv::KMEANS_PP_CENTERS, clusterCenters);
			for(const auto& point : clusterCenters)
			{
				cv::Scalar color(rand() % 191, rand() % 191, rand() % 191);
				cv::drawMarker(oneFrame, point, color, cv::MARKER_DIAMOND);
			}
		}
		
		//wyświetl
		cv::Mat displayFrame;
		cv::hconcat(oneFrame, displayBinary, displayFrame);
		cv::imshow(windowName, displayFrame);
		
		//steruje FPS
		if(cv::waitKey(5) >= 0)
		{
			break;
		}
	}
	
}
