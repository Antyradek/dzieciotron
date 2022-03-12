#include "pipeline.hpp"
#include <cassert>
#include <thread>
#include <chrono>
#include <future>
#include <opencv2/imgproc.hpp>
#include <stddef.h>
#include "logger.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

#include <cstdio>

using namespace pipeline;
using namespace logger;

Pipeline::Pipeline(const defines::CameraCaptureParams& params, AtomicPipelineResult& pipelineResult, externals::Lucipher& lucipher, externals::BaseHubber& hubber):
dzieciotron::AsyncTask(),
params(params),
pipelineResult(pipelineResult),
lucipher(lucipher),
hubber(hubber),
videoCapture(),
gpioOutput(),
background(),
detectives(),
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
	
	//marnotrawi ramki przez określony czas
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
}

void Pipeline::updateBackground()
{	
	//zgaś diodę
	this->setDiode(false);
	
	//złap klatkę
	this->background = this->getFrame();
	
	//zapal diodę
	this->setDiode(true);
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
	std::vector<std::tuple<double, std::reference_wrapper<const cv::Point2f>, std::reference_wrapper<const std::optional<cv::Point2f>>, std::reference_wrapper<const cv::Point2f>>> distances;
	
	//umieść odległości
	for(const std::optional<cv::Point2f>& detective : this->detectives)
	{
		if(detective.has_value())
		{
			for(const cv::Point2f& cluster : clusters)
			{
				const double distance = cv::norm(detective.value() - cluster);
				distances.emplace_back(distance, std::cref(detective.value()), std::cref(detective), std::cref(cluster));
			}
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
		auto [distance, detective, detectiveRef, cluster] = distances.front();
		//znormalizuj dystans w stosunku do wielkości ekranu
		const unsigned int mediumScreenSize = (this->params.width + this->params.height) / 2.0;
		const double distanceNormalized = distance / mediumScreenSize;
		const double detectiveMoveNormalized = defines::detectiveFunction(distanceNormalized);
		const double deltaTime = static_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - this->lastFrameTime).count();
		const double detectiveMoveNormalizedClamped = std::clamp(detectiveMoveNormalized * deltaTime * defines::detectiveSpeed, 0.0, 1.0);
		const cv::Point2f detectiveMove = (cluster.get() - detective.get()) * detectiveMoveNormalizedClamped;
		//ustaw detektywa w nowym miejscu
		*std::find(this->detectives.begin(), this->detectives.end(), detectiveRef.get()) = detectiveRef.get().value() + detectiveMove;
		
		//usuń wszystkie z listy
		distances.erase(std::remove_if(distances.begin(), distances.end(), [&detectiveRef, &cluster](auto& v){
			return(std::get<2>(v).get() == detectiveRef.get() or std::get<3>(v).get() == cluster.get());
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

std::optional<cv::Point2f> Pipeline::createDetective(size_t index, const std::vector<cv::Point2f>& clusters)
{
	Logger::debug() << "Tworzenie detektywa " << index << " " << defines::detectiveNames.at(index) << " z klastrami " << clusters.size();
	//wyłączenie diody
	this->setDiode(false);
	
	//aktywacja oświetlenia w tle
	std::atomic_bool turnsOn = true;
	
	auto turnOnFuture = std::async([this, &turnsOn](){
		double value = 0.0;
		this->lucipher.light(0.0);
		while(turnsOn)
		{
			//poczekaj
			std::this_thread::sleep_for(defines::lightOnTime);
			//oblicz nową wartość
			const double sleepSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(defines::lightOnTime).count();
			value = std::min(1.0, value + sleepSeconds * defines::lightOnSpeed);
			//ustaw wartość
			this->lucipher.light(value);
		}
	});
	
	std::optional<cv::Point2f> returnDetective;
	
	//próbkowanie obszaru klastra aby znaleźć kolor
	while(true)
	{
		const cv::Mat frame = this->getFrame();
		
		
		//wyznaczenie koła wokół środków klastrów
		//TODO bardziej zaawansowany algorytm zakłada rozrośnięcie obszarów klastrów zamiast koła wokół środków
		const unsigned int circleRadius = std::max(frame.cols, frame.rows) * defines::detectiveProbeRadiusFraction;
		for(size_t cli = 0; cli < clusters.size(); cli++)
		{
			const cv::Point2f point = clusters.at(cli);
			unsigned int pixelCount = 0;
			cv::Vec3d sumColor(0, 0, 0);
			for(int x = point.x - circleRadius; x <= point.x + circleRadius; x++)
			{
				for(int y = point.y - circleRadius; y <= point.y + circleRadius; y++)
				{
					if(x >= 0 && x <= frame.cols && y >= 0 && y <= frame.rows)
					{
						const cv::Vec3b color = frame.at<cv::Vec3b>(cv::Point(x, y));
						sumColor += cv::Vec3d(color) / 255.0;
						pixelCount++;
					}
				}
			}
			const cv::Vec3d averageColor = sumColor / static_cast<double>(pixelCount);
			
			//konwersja kolorów działa tylko na macierzach, więc trzeba stworzyć jednopikselową macierz trzykolorową
			//dla konwersji bezpośrednio na wektorach funkcja się zawiesza
			cv::Mat onePixelMat(1,1, CV_8UC3);
			onePixelMat.at<cv::Vec3b>(0,0) = averageColor * 255;;
			cv::Mat onePixelMatHls;
			cv::cvtColor(onePixelMat, onePixelMatHls, cv::COLOR_BGR2HLS_FULL);
			const cv::Vec3b averageHlsColorByte = onePixelMatHls.at<cv::Vec3b>(0,0);

			//w macierzy H jest w zakresie 0..255, a chcemy w kątach
			const double colorAngle = 360.0 * averageHlsColorByte[0] / 255.0;
			const double colorLightness = averageHlsColorByte[1] / 255.0;
			const double colorTarget = defines::detectiveColors.at(index);
			
			Logger::debug() << "Detektyw " << index << " kolor " << averageColor << " " << colorAngle << "° jasność " << colorLightness << " cel " << colorTarget << "°";
			
			if(colorLightness < defines::detectiveMinLightness)
			{
				//jasność jest za mała i odczyt będzie zbyt niedokładny, trzeba poczekać na rozjaśnienie diód
				continue;
			}
			
			const double distance = std::abs(colorAngle - colorTarget);
			if(distance < defines::detectiveColorMaxDistance)
			{
				Logger::info() << "Detektyw " << index << " " << defines::detectiveNames.at(index) << " znaleziony na " << colorAngle << "°";
				returnDetective = point;
				break;
			}
		}
		
		if(returnDetective.has_value())
		{
			//już znaleziony
			break;
		}
		
		if(this->lucipher.light() >= 1.0)
		{
			Logger::warning() << "Detektyw " << index << " nie znaleziony po osiągnięciu maksymalnego oświetlenia";
			break;
		}
	}
	
	//zatrzymaj rozjaśnianie
	turnsOn = false;
	turnOnFuture.wait();
	
	return(returnDetective);
	
	//TODO jeśli dwoje detektywów jest w tym samym miejscu, unieważnij obu
}

void Pipeline::submitResult(const cv::Mat& displayFrame)
{
	//zmień wielkość obrazu do podglądu
	cv::Mat newFrame = displayFrame;
	if(displayFrame.rows != defines::viewHeight || displayFrame.cols != defines::viewWidth)
	{
		cv::resize(displayFrame, newFrame, cv::Size(defines::viewWidth, defines::viewHeight));
	}
	assert(displayFrame.rows == defines::viewHeight);
	assert(displayFrame.cols == defines::viewWidth);
	
	//synchronizuje dane
	PipelineResult result;
	result.view = newFrame;
	this->pipelineResult.store(result);
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
	
	//zlokalizuj detektywów
	for(size_t i = 0; i < this->detectives.size(); i++)
	{
		auto& detective = this->detectives.at(i);
		if(!detective.has_value())
		{
			//wyślij podgląd
			this->submitResult(displayFrame);
			//stwórz nowego
			detective = this->createDetective(i, clusters);
		}
	}
	
	//szukaj detektywów
	this->trackDetectives(clusters);
	
	//narysuj historię detektywów
	std::reference_wrapper<const typeof(this->detectives)> lastDetective = this->detectives;
	for(const auto& detectives : this->detectiveHistory)
	{
		for(size_t i = 0; i < detectives.size(); i++)
		{
			if(lastDetective.get().at(i).has_value() && detectives.at(i).has_value())
			{
				cv::line(displayFrame, lastDetective.get().at(i).value(), detectives.at(i).value(), cv::Scalar(127, 127, 255), markersWidth);
			}
		}
		
		lastDetective = detectives;
	}
	
	//narysuj detektywów
	for(size_t i = 0; i < this->detectives.size(); i++)
	{
		if(this->detectives.at(i).has_value())
		{
			cv::circle(displayFrame, this->detectives.at(i).value(), markersSize, cv::Scalar(127, 255.0 * i / this->detectives.size(), 255), markersWidth);
		}
	}
	
	//wyślij dane
	this->submitResult(displayFrame);
	
	//aktualizuj czas
	this->lastFrameTime = std::chrono::steady_clock::now();
}



