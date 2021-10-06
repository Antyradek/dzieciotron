#include "pipeline.hpp"
#include <cassert>
#include <thread>
#include <chrono>
#include <future>
#include <opencv2/imgproc.hpp>
#include "logger.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

#include <cstdio>

using namespace pipeline;
using namespace logger;

Pipeline::Pipeline(const defines::CameraCaptureParams& params, AtomicPipelineResult& pipelineResult, externals::Lucipher& lucipher, externals::Hubber& hubber):
dzieciotron::AsyncTask(),
params(params),
pipelineResult(pipelineResult),
lucipher(lucipher),
hubber(hubber),
videoCapture(),
gpioOutput(),
background(),
detectives{{{0, 0}, {0, static_cast<float>(this->params.height)}, {static_cast<float>(this->params.width), static_cast<float>(this->params.height)}, {static_cast<float>(this->params.width), 0}}},
detectiveHistory(),
lastFrameTime(std::chrono::steady_clock::now())
{
	//wideo
	this->openVideo();
	
	//GPIO
	utils::openFile(defines::gpioControlFile(this->params.gpioPin), this->gpioOutput);
	
	//zaktualizuj tło
	this->updateBackground();
	
	//wypisanie
	Logger::debug() << "Ustawienia kamery: " << this->params.cameraFile << " " << this->params.width << "×" << this->params.height << "p" << this->params.fps << " → " << this->videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH) << "×" << this->videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT) << "p" << this->videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FPS) << " GPIO-" << this->params.gpioPin;
}

Pipeline::~Pipeline()
{
	this->setDiode(false);
}

void Pipeline::openVideo()
{
	const auto startTime = std::chrono::system_clock::now();
	bool set = false;
	do
	{
		try
		{
			//otwarcie wideo
			this->videoCapture.setExceptionMode(true);
			this->videoCapture.open(this->params.cameraFile, cv::VideoCaptureAPIs::CAP_V4L2);
			this->videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, this->params.width);
			this->videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, this->params.height);
			this->videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FPS, this->params.fps);
			//pierwsza klatka
			cv::Mat firstFrame;
			this->videoCapture >> firstFrame;
			if(firstFrame.empty())
			{
				throw(CameraError("Pusty obraz"));
			}
			
			set = true;
			break;
		}
		catch(const DzieciotronError& err)
		{
			Logger::error() << "Błąd inicjalizacji kamer: " << err.what();
			this->hubber.reset();
			std::this_thread::sleep_for(defines::cameraResetTime);
		}
	}
	while(std::chrono::system_clock::now() - startTime < defines::maxVideoStartTime);
	
	if(!set)
	{
		throw(CameraError("Wielokrotne próby inicjalizacji kamer się nie udały"));
	}
}

void Pipeline::setDiode(bool on)
{
	this->gpioOutput << (on ? "1" : "0") << std::endl;
}

void Pipeline::updateBackground()
{
	//potrzeba jest aktywnie czytać klatki aby nie buforować ich
	//można to zrobić na osobnym wątku, żeby nie wsadzać kodu do głównej pętli
	
	//marnotrawi ramki przez określony czas
	auto wasteFunction = [this](){
		std::atomic_bool wastesFrames = true;
		
		//uruchom marnotrawstwo klatek
		auto wasteFuture = std::async([this, &wastesFrames](){
			while(wastesFrames)
			{
				this->videoCapture.grab();
			}
		});
		//poczekaj na zmianę diody
		std::this_thread::sleep_for(defines::diodeToggleTime);
		//zatrzymaj marnowanie
		wastesFrames = false;
		wasteFuture.wait();
	};
	
	//zgaś diodę
	this->setDiode(false);
	//zmarnuj klatki
	wasteFunction();
	
	//złap klatkę
	this->background = this->getFrame();
	
	//zapal diodę
	this->setDiode(true);
	//i ponownie
	wasteFunction();
}

cv::Mat Pipeline::getBackground()
{
	return(this->background.clone());
}

cv::Mat Pipeline::getFrame()
{
	cv::Mat oneFrame;
	this->videoCapture >> oneFrame;
	//obróć obraz o 180°
	if(this->params.inverted)
	{
		cv::rotate(oneFrame, oneFrame, cv::RotateFlags::ROTATE_180);
	}
	return(oneFrame);
}

cv::Mat Pipeline::thresholdLuminance(const cv::Mat& baseFrame) const
{
	//przerób na inną przestrzeń
	cv::Mat hslSpaceFrame;
	cv::cvtColor(baseFrame, hslSpaceFrame, cv::COLOR_BGR2HLS_FULL);
	
	//podziel na składowe
	cv::Mat channels[hslSpaceFrame.channels()];
	cv::split(hslSpaceFrame, channels);
	cv::Mat luminanceFrame = channels[1];
	
	//progowanie
	cv::Mat binaryFrame;
	cv::threshold(luminanceFrame, binaryFrame, this->params.luminanceThreshold * 255, 255, cv::ThresholdTypes::THRESH_BINARY);
	
	//zamknięcie i otwarcie
	cv::morphologyEx(binaryFrame, binaryFrame, cv::MorphTypes::MORPH_CLOSE, cv::getStructuringElement(cv::MorphShapes::MORPH_ELLIPSE, cv::Size(defines::openCloseKernelSize, defines::openCloseKernelSize)));
	cv::morphologyEx(binaryFrame, binaryFrame, cv::MorphTypes::MORPH_OPEN, cv::getStructuringElement(cv::MorphShapes::MORPH_ELLIPSE, cv::Size(defines::openCloseKernelSize, defines::openCloseKernelSize)));
	
	return(binaryFrame);
}

std::vector<cv::Point2f> Pipeline::findClusters(const cv::Mat& binaryFrame, std::vector<std::vector<cv::Point>>& contours) const
{
	//TODO kontur wewnętrzny jest traktowany jako osobny
	//kontury to zestawy zestawów punktów 2D, które są wokół znalezionego obszaru

	//zbiór ustalający kolejność konturów, każdy wektor zawiera 4 elementy oznaczające indeksy: kolejny, poprzedni, rodzica i dziecko
	std::vector<cv::Vec4i> hierarchy;
	//TODO dokładniejszy sposób obliczeń dla obwarzanków, które mają dodatkowe kontury wewnątrz
	//druga część to aproksymacja czy trzymać wszystkie piksele czy tylko otoczkę
	cv::findContours(binaryFrame, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	
	//narysuj kontury i oblicz środki
	//tablica do wypełnienia środkami konturów
	std::vector<cv::Point2f> contourCenters(contours.size());
	for(size_t idx = 0; idx < contours.size(); idx++)
	{
		//oblicz środek konturu
		cv::Moments moments = cv::moments(contours[idx], true);
		double posX = moments.m10 / moments.m00;
		double posY = moments.m01 / moments.m00;
		contourCenters.at(idx) = cv::Point2f(posX, posY);
	}
	
	//klastrowanie
	std::vector<cv::Point2f> clusterCenters;
	if(contourCenters.size() > 0)
	{
		cv::Mat bestLabels;
		//kryterium ilości powtórzeń bądź dokładności
		cv::kmeans(contourCenters, std::min<int>(4, contourCenters.size()), bestLabels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, defines::clusteringMaxIterations, defines::clusteringMinEpsilon), 5, cv::KMEANS_PP_CENTERS, clusterCenters);
	}
	
	return(clusterCenters);
}

void Pipeline::trackDetectives(const std::vector<cv::Point2f>& clusters)
{
	//odległości detektyw i klaster
	std::vector<std::tuple<double, std::reference_wrapper<const cv::Point2f>, std::reference_wrapper<const cv::Point2f>>> distances;
	
	//umieść odległości
	for(const cv::Point2f& detective : this->detectives)
	{
		for(const cv::Point2f& cluster : clusters)
		{
			const double distance = cv::norm(detective - cluster);
			distances.emplace_back(distance, std::cref(detective), std::cref(cluster));
		}
	}
	
	//posortuj
	std::sort(distances.begin(), distances.end(), [](auto& a, auto& b){
		return(std::get<0>(a) < std::get<0>(b));
	});
	
	//weź kolejne najkrótsze
	//jeśli klastrów jest mniej, to niektórzy detektywni zostaną nieruszeni
	while(not distances.empty())
	{
		auto [distance, detective, cluster] = distances.front();
		//znormalizuj dystans w stosunku do wielkości ekranu
		const unsigned int mediumScreenSize = (this->params.width + this->params.height) / 2.0;
		const double distanceNormalized = distance / mediumScreenSize;
		const double detectiveMoveNormalized = defines::detectiveFunction(distanceNormalized);
		const double deltaTime = static_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - this->lastFrameTime).count();
		const double detectiveMoveNormalizedClamped = std::clamp(detectiveMoveNormalized * deltaTime * defines::detectiveSpeed, 0.0, 1.0);
		const cv::Point2f detectiveMove = (cluster.get() - detective.get()) * detectiveMoveNormalizedClamped;
		//ustaw detektywa w nowym miejscu
		*std::find(this->detectives.begin(), this->detectives.end(), detective.get()) = detective.get() + detectiveMove;
		
		//usuń wszystkie z listy
		distances.erase(std::remove_if(distances.begin(), distances.end(), [&detective, &cluster](auto& v){
			return(std::get<1>(v).get() == detective.get() or std::get<2>(v).get() == cluster.get());
		}), distances.end());
	}
	
	//umieść pozycje detektywów w historii
	this->detectiveHistory.emplace_front(this->detectives);
	
	//usuń najstarszego detektywa
	if(this->detectiveHistory.size() > defines::detectiveHistoryLength)
	{
		this->detectiveHistory.pop_back();
	}
}

void Pipeline::runLoop()
{	
	cv::Mat oneFrame = this->getFrame();
	
	//klatka do podglądu
	cv::Mat displayFrame = oneFrame.clone();
	const unsigned int markersWidth = displayFrame.cols / 150;
	const unsigned int markersSize = displayFrame.cols / 40;
	
	//odejmij tło od obrazu
	oneFrame = oneFrame - this->getBackground();
	
	//filtr medianowy
	//TODO to jest strasznie ciężkie, może dało by się bez albo sprzętowo NEONem
	//TODO filtr bilateralny?
	static_assert(defines::smoothKernelSize % 2 == 1);
// 	cv::medianBlur(oneFrame, smoothFrame, defines::smoothKernelSize);
	//FIXME na razie zwykłe rozmycie
	cv::blur(oneFrame, oneFrame, cv::Size(5, 5));
	
	//przekształcenia kolorów i progowanie
	cv::Mat binaryFrame = this->thresholdLuminance(oneFrame);
	
	//znajdź środki klastrów
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Point2f> clusters = this->findClusters(binaryFrame, contours);
	
	//narysuj kontury
	for(size_t idx = 0; idx < contours.size(); idx++)
	{
		//kolor zmienia się z każdym kolejnym konturem (format BGR)
		const cv::Scalar color(255, 0, 255.0 * idx / contours.size());
		cv::drawContours(displayFrame, contours, idx, color, markersWidth);
	}
	
	//narysuj środki klastrów
	for(const cv::Point2f& point : clusters)
	{
		cv::circle(displayFrame, point, 2 * markersSize, cv::Scalar(255, 255, 255), markersWidth);
	}
	
	//szukaj detektywów
	this->trackDetectives(clusters);
	
	//narysuj historię detektywów
	std::reference_wrapper<const typeof(this->detectives)> lastDetective = this->detectives;
	for(const auto& detectives : this->detectiveHistory)
	{
		for(size_t i = 0; i < detectives.size(); i++)
		{
			cv::line(displayFrame, lastDetective.get().at(i), detectives.at(i), cv::Scalar(127, 127, 255), markersWidth);
		}
		
		lastDetective = detectives;
	}
	
	//narysuj detektywów
	for(size_t i = 0; i < this->detectives.size(); i++)
	{
		cv::circle(displayFrame, this->detectives.at(i), markersSize, cv::Scalar(127, 255.0 * i / this->detectives.size(), 255), markersWidth);
	}
	
	//FIXME
	//oświetlenie
	if (rand() % 100 < 3)
	{
		this->lucipher.light(rand() % 100 / 100.0);
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
	
	//aktualizuj czas
	this->lastFrameTime = std::chrono::steady_clock::now();
}



