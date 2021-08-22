// #include <iostream>
// #include <fstream>
// #include <ctime>
#include <csignal>
#include <cstring>
// #include <map>
#include <future>
#include <functional>
#include <array>
#include <algorithm>


// 
// #include <boost/program_options.hpp>
// 
// #include "exceptions.hpp"
#include "logger.hpp"
#include "pipeline.hpp"
#include "defines.hpp"
#include "locationer.hpp"
#include "view_sender.hpp"

#ifdef GUI_DEBUG
	#include "debug/dummy_pipeline.hpp"
	#include "debug/view_shower.hpp"
#endif

// static void setSize(cv::VideoCapture& videoCapture, int width, int height, int fps)
// {
// 	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, width);
// 	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, height);
// 	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FPS, fps);
// }
// 
// namespace po = boost::program_options;
// 
// const std::string helpText = "help";
// const std::string fileText = "file";
// const std::string cametaText = "camera";
// const std::string outputText = "output";
// const std::string markerText = "markerfile";
using namespace utils;
// 
// /// Jeden rekord do jednej ramki z pliku do oznaczania markerów
// struct MarkerRecord
// {
// 	struct MarkerPosition
// 	{
// 		double x;
// 		double y;
// 	};
// 	
// 	enum class MarkerType
// 	{
// 		WristLeft,
// 		WristRight,
// 		AnkleLeft,
// 		AnkleRight
// 	};
// 	
// 	typedef std::map<MarkerType, MarkerPosition> MarkersMapType;
// 	MarkersMapType markers;
// };
// 
// const std::map<std::string, MarkerRecord::MarkerType> markerTranslationMap({
// 	{"ankle_left", MarkerRecord::MarkerType::AnkleLeft},
// 	{"ankle_right", MarkerRecord::MarkerType::AnkleRight},
// 	{"wrist_left", MarkerRecord::MarkerType::WristLeft},
// 	{"wrist_right", MarkerRecord::MarkerType::WristRight}
// });
// 
// int main(int argc, char** argv)
// {
// 	po::options_description description("Opcje");
// 	description.add_options()
// 		(helpText.c_str(), "Wypisz pomoc")
// 		(fileText.c_str(), po::value<std::string>(), "Wczytaj dane z pliku wideo")
// 		(cametaText.c_str(), po::value<unsigned int>(), "Użyj tej kamery")
// 		(outputText.c_str(), po::value<std::string>(), "Zapisz dane do pliku wideo")
// 		(markerText.c_str(), po::value<std::string>(), "Narysuj markery z podanego pliku")
// 	;
// 	po::variables_map variables;
// 	po::store(po::parse_command_line(argc, argv, description), variables);
// 	po::notify(variables);
// 	
// 	if(variables.count(helpText) > 0)
// 	{
// 		std::cout << description << std::endl;
// 		return(0);
// 	}
// 	

// 	
// 	//główny wypisywacz
// 	cv::VideoWriter videoWriter;
// 	
// 	if(variables.count(fileText) > 0)
// 	{
// 		const std::string filename = variables[fileText].as<std::string>();
// 		Logger::debug() << "Odczyt z pliku: " << filename;
// 		videoCapture.open(filename);
// 	}
// 	else
// 	{
// 		unsigned int cameraId = 2;
// 		
// 		if(variables.count(cametaText) > 0)
// 		{
// 			cameraId = variables[cametaText].as<unsigned int>();
// 		}
// 				
// 		Logger::debug() << "Otwieranie kamery " << cameraId;
// 		videoCapture.open(cameraId, cv::VideoCaptureAPIs::CAP_V4L2);
// 		
// 		//odczyt pierwszej ramki
// 		cv::Mat firstFrame;
// 		videoCapture >> firstFrame;
// 		if(firstFrame.empty())
// 		{
// 			throw(CameraError("Pusty obraz"));
// 		}
// 		
// // 			setSize(videoCapture, 1920, 1080, 30);
// // 			setSize(videoCapture, 1028, 720, 30);
// 		// 	setSize(videoCapture, 2048, 1536, 30);
// // 		setSize(videoCapture, 320, 240, 30);
// 		setSize(videoCapture, 1280, 720, 30);
// 		//TODO może H.265
// 		videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
// 		(void) setSize;
// 	}
// 	
// 	//wyjście wideo
// 	if(variables.count(outputText) > 0)
// 	{
// 		const std::string outputFilename = variables[outputText].as<std::string>();
// 		
// 		Logger::debug() << "Wyjście wideo będzie do " << outputFilename;
// 		const int width = videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
// 		const int height = videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
// 		
// 		//rozmiar zależy od wyjściowych kawałków
// 		//TODO wypróbować H.265
// 		videoWriter.open(outputFilename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(width * 2, height));
// 	}
// 	
// 	//markery
// 	std::vector<MarkerRecord> markers;
// 	if(variables.count(markerText) > 0)
// 	{
// 		const std::string markerFile = variables[markerText].as<std::string>();
// 		
// 		std::ifstream file;
// 		//TODO pełne wsparcie wyjątków
// 		file.exceptions(std::ifstream::badbit);
// 		file.open(markerFile);
// 		
// 		std::string line;
// 		int lastframeNo = -1;
// 		while(std::getline(file, line))
// 		{
// 			int frameNo;
// 			std::string lineId;
// 			double x;
// 			double y;
// 			double z;
// 			double screenX;
// 			double screenY;
// 			
// 			std::istringstream ss(line);
// 			
// 			ss >> frameNo >> lineId >> x >> y >> z >> screenX >> screenY;
// 			
// 			if(lastframeNo != frameNo)
// 			{
// 				Logger::debug() << "Klatka " << frameNo;
// 				lastframeNo = frameNo;
// 				
// 				markers.push_back(MarkerRecord());
// 			}
// 			
// 			Logger::debug() << "Wczytywanie rekordu: " << frameNo << " " << lineId << " " << screenX << " " << screenY;
// 			
// 			MarkerRecord::MarkerPosition position;
// 			position.x = screenX;
// 			//pion jest odwrócony
// 			position.y = 1 - screenY;
// 			MarkerRecord::MarkerType markerType = markerTranslationMap.at(lineId);
// 			
// 			markers.back().markers[markerType] = position;
// 		}
// 		
// 	}
// 	
// 	
// 	const std::string windowName = "Podgląd kamery";
// 	cv::namedWindow(windowName);
// 	
// 	timer_t fpsTimer;
// 	struct sigevent timerEvent = {};
// 	timerEvent.sigev_notify = SIGEV_NONE;
// 	if(timer_create(CLOCK_REALTIME, &timerEvent, &fpsTimer) != 0)
// 	{
// 		throw(TimerError("Błąd inicjalizacji"));
// 	}
// 	struct itimerspec timerTime = {};
// 	timerTime.it_value.tv_sec = 1;
// 	timerTime.it_interval.tv_sec = 1;
// 	if(timer_settime(fpsTimer, 0, &timerTime, nullptr) != 0)
// 	{
// 		throw(TimerError("Błąd ustawiania"));
// 	}
// 	
// 	std::cout << "Rozmiar: " << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH) << "×" << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT) << " " << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FPS) << " FPS" << std::endl;
// 	
// 	int fpsFrameCounter = 0;
// 	long lastTimerValue = 0;
// 	uintmax_t frameCounter = 0;
// 	cv::Mat oneFrame;
// 	while(true)
// 	{
// 		try
// 		{
// 			videoCapture >> oneFrame;
// 		}
// 		catch(const cv::Exception& ex)
// 		{
// 			Logger::debug() << "Koniec wejścia";
// 			Logger::debug() << "Przeczytano ramek: " << frameCounter;
// 			break;
// 		}
// 		
// 		fpsFrameCounter++;
// 		
// 		
// 		//zmień rozmiar, obraz jest zawsze 4:3, ale czasami rozciągnięty
// // 		const int properWidth = 4.0/3.0 * oneFrame.rows;
// // 		cv::resize(oneFrame, oneFrame, cv::Size(), 1.0 * properWidth / oneFrame.cols, 1);
// 		
// 		
// 		
// 		//oznaczenie markerów z pliku
// 		
// 		if(markers.size() > frameCounter)
// 		{
// 			MarkerRecord record = markers.at(frameCounter);
// 			for(const MarkerRecord::MarkersMapType::value_type& pair : record.markers)
// 			{
// 				Logger::debug() << "Markerów " << markers.size();
// 				const int x = pair.second.x * displayBinary.cols;
// 				const int y = pair.second.y * displayBinary.rows;
// 				
// 				//TODO pomarańczowy jako stała
// 				cv::drawMarker(displayBinary, cv::Point2f(x, y), cv::Scalar(0, 128, 255), cv::MARKER_DIAMOND);
// 				
// 				Logger::debug() << "Marker " << x << y;
// 			}
// 		}
// 		
// 		//zapisz
// 		if(videoWriter.isOpened())
// 		{
// 			videoWriter.write(displayFrame);
// 		}
// 		
// 		//oblicza FPS
// 		struct itimerspec retTime;
// 		timer_gettime(fpsTimer, &retTime);
// 		if(lastTimerValue < retTime.it_value.tv_nsec)
// 		{
// 			std::cout << "\e[G\e[K" << (1.0e9 * fpsFrameCounter / ((retTime.it_interval.tv_sec * 1.0e9 + retTime.it_interval.tv_nsec) - lastTimerValue)) << " FPS" << std::flush;
// 			fpsFrameCounter = 0;
// 		}
// 		lastTimerValue = retTime.it_value.tv_nsec;
// 		
// 		//ostatnie zwiększenie ramki
// 		frameCounter++;
// 	}
// 	timer_delete(fpsTimer);
// 	
// }

/// Callback sygnału
namespace globals
{
std::function<void()> exitCallback;
static void signalCallback(int signum)
{
	Logger::info() << "Sygnał " << strsignal(signum);
	exitCallback();
}
}

int main()
{	
	//TODO Użyć "v4l2-sysfs-path -d" do ogarnięcia które kamery są które albo zajrzeć do /dev
	
	pipeline::AtomicPipelineResult leftResult;
	pipeline::AtomicPipelineResult centerResult;
	pipeline::AtomicPipelineResult rightResult;
	pipeline::AtomicPipelineResult viewResult;
	
	externals::Lucipher lucipher;
	
	pipeline::Pipeline centerPipeline(defines::centerCameraParams, centerResult, lucipher);
	locationer::Locationer locationer(leftResult, centerResult, rightResult, viewResult);
	
	//w debugu potoki dwóch pozostałych kamer są sztuczne
#ifndef GUI_DEBUG
	pipeline::Pipeline leftPipeline(defines::leftCameraParams, leftResult, lucipher);
	pipeline::Pipeline rightPipeline(defines::rightCameraParams, rightResult, lucipher);
	view::ViewSender viewSender(defines::viewPipe, viewResult);
#else
	debug::DummyPipeline leftPipeline(leftResult, lucipher);
	debug::DummyPipeline rightPipeline(rightResult, lucipher);
	debug::ViewShower viewSender(viewResult);
#endif
	
	std::array<std::reference_wrapper<dzieciotron::AsyncTask>, 6> tasks {{centerPipeline, leftPipeline, rightPipeline, locationer, viewSender, lucipher}};
	
	//obsługa sygnału
	globals::exitCallback = [&tasks](){
		for(dzieciotron::AsyncTask& task: tasks)
		{
			task.stop();
		}
	};
	signal(SIGINT, globals::signalCallback);
	signal(SIGTERM, globals::signalCallback);
	signal(SIGPIPE, SIG_IGN);
	
	//wystartuj
	std::array<std::future<void>, tasks.size()> futures;
	//dla każdego elementu w tablicy zadań, wywołaj funkcję, która stworzy przyszłościowe zadanie i wsadzi do drugiej tablicy
	std::transform(tasks.begin(), tasks.end(), futures.begin(), [](dzieciotron::AsyncTask& task) -> std::future<void>{
		//wystartuj zadanie w osobnym wątku
		return(std::async(std::launch::async, [&task](){
			task.run();
		}));
	});
	
	//czekaj na zakończenie
	for(std::future<void>& future: futures)
	{
		future.get();
	}
	
}
