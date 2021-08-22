#pragma once
#include <string>
#include <algorithm>

/// Generalne ustawienia programu
namespace defines
{
	/// Parametry działania kamery
	struct CameraCaptureParams
	{
		/// Plik kamery
		std::string cameraFile;
		
		/// Szerokość obrazu
		unsigned int width;
		
		/// Wysokość obrazu
		unsigned int height;
		
		/// Liczba klatek na sekundę
		unsigned int fps;
		
		/// Pin wyjścia GPIO do oświetlenia
		unsigned int gpioPin;
		
		/// Czy obrócić wyjście z kamery
		bool inverted;
		
		/// Próg jasności wyznaczania znaczników
		double luminanceThreshold;
		
		CameraCaptureParams(const std::string& cameraFile, unsigned int width, unsigned int height, unsigned int fps, unsigned int gpioPin, bool inverted, double threshold):
		cameraFile(cameraFile),
		width(width),
		height(height),
		fps(fps),
		gpioPin(gpioPin), 
		inverted(inverted),
		luminanceThreshold(threshold){}
	};
	
	/// Parametry centralnej kamery
	const CameraCaptureParams centerCameraParams("/dev/video0", 320, 240, 60, 15, false, 0.3);
	
	/// Parametry lewej kamery
	const CameraCaptureParams leftCameraParams("/dev/video3", 1280, 720, 30, 7, false, 0.5);
	
	/// Parametry prawej kamery
	const CameraCaptureParams rightCameraParams("/dev/video1", 1280, 720, 30, 21, true, 0.5);
	
	/// Czas przełączenia diody
	const std::chrono::milliseconds diodeToggleTime(100);
	
	/// Wielkość jądra wygładzania
	const unsigned int smoothKernelSize = 5;
	
	/// Wielkość jądra dla otwarcia i zamknięcia
	const unsigned int openCloseKernelSize = 3;
	
	/// Maksymalna ilość iteracji klastrowania
	const unsigned int clusteringMaxIterations = 10;
	
	/// Minimalna różnica pozycji przy klastrowaniu
	const double clusteringMinEpsilon = 1.0;
	
	/// Szerokość podglądu kamery
	const unsigned int viewWidth = 320;
	
	/// Wysokość podglądu kamery
	const unsigned int viewHeight = 240;
	
	/// Ilość klatek na sekundę podglądu
	const double viewFps = 10;
	
	/// Prędkość przetwarzania dla nieaktywnych potoków
	const double idleFps = 1;
	
	/// Ilość klatek na sekundę dla przetwarzania pozycji
	const double locationerFps = 60;
	
	/// Potok do którego zapisujemy podgląd
	const std::string viewPipe = "/run/dzieciotron.fifo";
	
	/// Długość historii detektywów do wyświetlania
	const unsigned int detectiveHistoryLength = 3;
	
	/// Funkcja do tworzenia nazwy pliku do sterowania GPIO (ponieważ numer jest w środku ścieżki)
	const std::function<std::string(unsigned int)> gpioControlFile = [](unsigned int gpio){return("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");};
	
	/// Współczynnik prędkości przemieszczania się detektywów
	const double detectiveSpeed = 5;
	
	/// Prędkość spadku oświetlenia
	const double lightFadeOffSpeed = 0.3;
	
	/// Ilość odświeżeń na sekundę światła
	const double lightFps = 5;
	
	/// Funkcja opóźniająca detektywa
	const std::function<double(double)> detectiveFunction = [](double x){
		//dla niskich wartości zwracamy 1 - detektyw się rusza natychmiastowo
		//dla wysokich wartości asymtotycznie do 0
		//const double value = 0.15 / (x + 0.1);
		//return(std::min(value, 1.0));
		return(0.1 / x);
		//const double width = 2.6;
		//return(2.1 * width * (x + 0.4) / std::pow(2, width * (x + 0.4)));
	};
	
}
