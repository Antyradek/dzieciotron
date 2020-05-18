#include <iostream>
#include <algorithm>
#include <ctime>
#include <csignal>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <boost/program_options.hpp>

#include "exceptions.hpp"
#include "logger.hpp"

static void setSize(cv::VideoCapture& videoCapture, int width, int height, int fps)
{
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, width);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, height);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FPS, fps);
}

namespace po = boost::program_options;

const std::string helpText = "help";
const std::string fileText = "file";
const std::string cametaText = "camera";
const std::string outputText = "output";
using namespace dzieciotron;

int main(int argc, char** argv)
{
	po::options_description description("Opcje");
	description.add_options()
		(helpText.c_str(), "Wypisz pomoc")
		(fileText.c_str(), po::value<std::string>(), "Wczytaj dane z pliku wideo")
		(cametaText.c_str(), po::value<unsigned int>(), "Użyj tej kamery")
		(outputText.c_str(), po::value<std::string>(), "Zapisz dane do pliku wideo")
	;
	po::variables_map variables;
	po::store(po::parse_command_line(argc, argv, description), variables);
	po::notify(variables);
	
	if(variables.count(helpText) > 0)
	{
		std::cout << description << std::endl;
		return(0);
	}
	
	//główne połączenie z kamerą lub plikiem wideo
	cv::VideoCapture videoCapture;
	videoCapture.setExceptionMode(true);
	
	//główny wypisywacz
	cv::VideoWriter videoWriter;
	
	if(variables.count(fileText) > 0)
	{
		const std::string filename = variables[fileText].as<std::string>();
		Logger::debug() << "Odczyt z pliku: " << filename;
		videoCapture.open(filename);
	}
	else
	{
		unsigned int cameraId = 2;
		
		if(variables.count(cametaText) > 0)
		{
			cameraId = variables[cametaText].as<unsigned int>();
		}
				
		Logger::debug() << "Otwieranie kamery " << cameraId;
		videoCapture.open(cameraId, cv::VideoCaptureAPIs::CAP_V4L2);
		
		//odczyt pierwszej ramki
		cv::Mat firstFrame;
		videoCapture >> firstFrame;
		if(firstFrame.empty())
		{
			throw(CameraError("Pusty obraz"));
		}
		
// 			setSize(videoCapture, 1920, 1080, 30);
// 			setSize(videoCapture, 1028, 720, 30);
		// 	setSize(videoCapture, 2048, 1536, 30);
// 		setSize(videoCapture, 320, 240, 30);
		setSize(videoCapture, 1280, 720, 30);
		videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
		(void) setSize;
	}
	
	if(variables.count(outputText) > 0)
	{
		const std::string outputFilename = variables[outputText].as<std::string>();
		
		Logger::debug() << "Wyjście wideo będzie do " << outputFilename;
		const int width = videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
		const int height = videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
		
		//rozmiar zależy od wyjściowych kawałków
		videoWriter.open(outputFilename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(width * 2, height));
	}
	
	
	const std::string windowName = "Podgląd kamery";
	cv::namedWindow(windowName);
	
	timer_t fpsTimer;
	struct sigevent timerEvent = {};
	timerEvent.sigev_notify = SIGEV_NONE;
	if(timer_create(CLOCK_REALTIME, &timerEvent, &fpsTimer) != 0)
	{
		throw(TimerError("Błąd inicjalizacji"));
	}
	struct itimerspec timerTime = {};
	timerTime.it_value.tv_sec = 1;
	timerTime.it_interval.tv_sec = 1;
	if(timer_settime(fpsTimer, 0, &timerTime, nullptr) != 0)
	{
		throw(TimerError("Błąd ustawiania"));
	}
	
	std::cout << "Rozmiar: " << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH) << "×" << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT) << " " << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FPS) << " FPS" << std::endl;
	
	int fpsFrameCounter = 0;
	long lastTimerValue = 0;
	uintmax_t frameCounter = 0;
	cv::Mat oneFrame;
	while(true)
	{
		try
		{
			videoCapture >> oneFrame;
		}
		catch(const cv::Exception& ex)
		{
			Logger::debug() << "Koniec wejścia";
			Logger::debug() << "Przeczytano ramek: " << frameCounter;
			break;
		}
		
		fpsFrameCounter++;
		frameCounter++;
		
		
		//zmień rozmiar, obraz jest zawsze 4:3, ale czasami rozciągnięty
// 		const int properWidth = 4.0/3.0 * oneFrame.rows;
// 		cv::resize(oneFrame, oneFrame, cv::Size(), 1.0 * properWidth / oneFrame.cols, 1);
		
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
		
		//narysuj kontury i oblicz środki
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
				cv::drawMarker(displayBinary, point, cv::Scalar(255, 255, 255), cv::MARKER_DIAMOND);
			}
		}
		
		//wyświetl
		cv::Mat displayFrame;
		cv::hconcat(oneFrame, displayBinary, displayFrame);
		cv::imshow(windowName, displayFrame);
		
		//zapisz
		if(videoWriter.isOpened())
		{
			videoWriter.write(displayFrame);
		}
		
		//steruje FPS
		if(cv::waitKey(5) >= 0)
		{
			break;
		}
		
		//oblicza FPS
		struct itimerspec retTime;
		timer_gettime(fpsTimer, &retTime);
		if(lastTimerValue < retTime.it_value.tv_nsec)
		{
			std::cout << "\e[G\e[K" << (1.0e9 * fpsFrameCounter / ((retTime.it_interval.tv_sec * 1.0e9 + retTime.it_interval.tv_nsec) - lastTimerValue)) << " FPS" << std::flush;
			fpsFrameCounter = 0;
		}
		lastTimerValue = retTime.it_value.tv_nsec;
		
		
	}
	timer_delete(fpsTimer);
	
}
