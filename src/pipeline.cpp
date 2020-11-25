#include "pipeline.hpp"
#include <cassert>
#include <thread>
#include <chrono>
#include <opencv2/imgproc.hpp>
#include "logger.hpp"
#include "exceptions.hpp"

#include <cstdio>

using namespace pipeline;
using namespace utils;

Pipeline::Pipeline(const defines::CameraCaptureParams& params, AtomicPipelineResult& pipelineResult):
dzieciotron::AsyncTask(),
params(params),
pipelineResult(pipelineResult),
videoCapture(),
gpioOutput()
{
	//otwarcie wideo
	this->videoCapture.setExceptionMode(true);
	videoCapture.open(params.cameraFile, cv::VideoCaptureAPIs::CAP_V4L2);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, params.width);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, params.height);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FPS, params.fps);
	cv::Mat firstFrame;
	videoCapture >> firstFrame;
	if(firstFrame.empty())
	{
		throw(CameraError("Pusty obraz"));
	}
	
	//GPIO
	gpioOutput.exceptions(std::ofstream::badbit | std::ofstream::failbit);
	gpioOutput.open(defines::gpioControlFile(params.gpioPin), std::ofstream::out);
	
	//wypisanie
	Logger::debug() << "Ustawienia kamery: " << params.cameraFile << " " << params.width << "×" << params.height << "p" << params.fps << " → " << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH) << "×" << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT) << "p" << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FPS) << " GPIO-" << params.gpioPin;
}

void Pipeline::setDiode(bool on)
{
	this->gpioOutput << (on ? "1" : "0") << std::endl;
}

void Pipeline::runLoop()
{
	//FIXME debugowo diody
	this->setDiode((rand() % 100 < 50));
	
	cv::Mat oneFrame;
	videoCapture >> oneFrame;
	cv::Mat smoothFrame(oneFrame);
	
	//filtr medianowy
	//TODO to jest strasznie ciężkie, może dało by się bez albo sprzętowo NEONem
	static_assert(defines::smoothKernelSize % 2 == 1);
// 	cv::medianBlur(oneFrame, smoothFrame, defines::smoothKernelSize);
	//FIXME na razie zwykłe rozmycie
	cv::blur(oneFrame, smoothFrame, cv::Size(5, 5));
	cv::Mat displayFrame(smoothFrame);
	const unsigned int markersWidth = displayFrame.cols / 150;
	const unsigned int markersSize = displayFrame.cols / 40;
	
	//przerób na inną przestrzeń
	cv::Mat hslSpaceFrame;
	cv::cvtColor(smoothFrame, hslSpaceFrame, cv::COLOR_BGR2HLS_FULL);
	
	//podziel na składowe
	cv::Mat channels[hslSpaceFrame.channels()];
	cv::split(hslSpaceFrame, channels);
	cv::Mat luminanceFrame = channels[1];
	
	//progowanie
	cv::Mat binaryFrame;
	cv::threshold(luminanceFrame, binaryFrame, defines::luminanceThreshold * 255, 255, cv::ThresholdTypes::THRESH_BINARY);
	
	//zamknięcie i otwarcie
	cv::morphologyEx(binaryFrame, binaryFrame, cv::MorphTypes::MORPH_CLOSE, cv::getStructuringElement(cv::MorphShapes::MORPH_ELLIPSE, cv::Size(defines::openCloseKernelSize, defines::openCloseKernelSize)));
	cv::morphologyEx(binaryFrame, binaryFrame, cv::MorphTypes::MORPH_OPEN, cv::getStructuringElement(cv::MorphShapes::MORPH_ELLIPSE, cv::Size(defines::openCloseKernelSize, defines::openCloseKernelSize)));
	
	//kontury
	//TODO kontur wewnętrzny jest traktowany jako osobny
	//NOTE kontury to zestawy zestawów punktów 2D, które są wokół znalezionego obszaru
	std::vector<std::vector<cv::Point>> contours;
	//NOTE zbiór ustalający kolejność konturów, każdy wektor zawiera 4 elementy oznaczające indeksy: kolejny, poprzedni, rodzica i dziecko
	std::vector<cv::Vec4i> hierarchy;
	//TODO dokładniejszy sposób obliczeń dla obwarzanków
	//NOTE druga część to aproksymacja czy trzymać wszystkie piksele czy tylko otoczkę
	cv::findContours(binaryFrame, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	
	//narysuj kontury i oblicz środki
	//tablica do wypełnienia środkami konturów
	std::vector<cv::Point2f> contourCenters(contours.size());
	for(size_t idx = 0; idx < contours.size(); idx++)
	{
		//kolor zmienia się z każdym kolejnym konturem
		const cv::Scalar color(128 + 128 * (idx / contours.size()), 0, 255);
		cv::drawContours(displayFrame, contours, idx, color, markersWidth);
		
		//oblicz środek konturu
		cv::Moments moments = cv::moments(contours[idx], true);
		double posX = moments.m10 / moments.m00;
		double posY = moments.m01 / moments.m00;
		contourCenters.at(idx) = cv::Point2f(posX, posY);
		cv::circle(displayFrame, contourCenters.at(idx), markersSize, color * 0.2, markersWidth);
// 		cv::drawMarker(displayFrame, contourCenters.at(idx), color * 0.2, cv::MARKER_STAR, 20);
	}
	
	//klastrowanie
	if(contourCenters.size() > 0)
	{
		cv::Mat bestLabels;
		std::vector<cv::Point2f> clusterCenters;
		//kryterium ilości powtórzeń bądź dokładności
		cv::kmeans(contourCenters, std::min<int>(4, contourCenters.size()), bestLabels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, defines::clusteringMaxIterations, defines::clusteringMinEpsilon), 5, cv::KMEANS_PP_CENTERS, clusterCenters);
		for(const cv::Point2f& point : clusterCenters)
		{
			//narysuj markery
// 			cv::drawMarker(displayFrame, point, cv::Scalar(255, 255, 255), cv::MARKER_DIAMOND);
			cv::circle(displayFrame, point, 2 * markersSize, cv::Scalar(255, 255, 255), markersWidth);
		}
	}
	
	//zmień wielkość obrazu do podglądu
	if(displayFrame.rows != defines::viewHeight || displayFrame.cols != defines::viewWidth)
	{
		cv::resize(displayFrame, displayFrame, cv::Size(defines::viewWidth, defines::viewHeight));
	}
	assert(displayFrame.rows == defines::viewHeight);
	assert(displayFrame.cols == defines::viewWidth);
	
	//synchronizuje dane
	PipelineResult result;
	result.view = displayFrame;
	this->pipelineResult.store(result);
}



