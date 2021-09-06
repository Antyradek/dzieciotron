#pragma once
#include <string>
#include <algorithm>
#include <cstdint>

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
		
		/// Pin wyjścia GPIO do oświetlenia (zapisać 0/1)
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
		luminanceThreshold(threshold) {}
	};
	
	/// Parametry działania PWM
	struct PwmParams
	{
		/// Plik włączenia (zapisać 0/1)
		std::string enableFile;
		
		/// Plik czasu wypełnienia (zapisać ilość ns wypełnienia)
		std::string dutyCycleFile;
		
		/// Plik okresu (zapisać ilość ns)
		std::string periodFile;
		
		/// Częstotliwość działania w Hz
		double frequency;
		
		/// Ilość ns w okresie, na podstawie częstotliwości
		const uint_least32_t period;
		
		PwmParams(const std::string& enableFile, const std::string& dutyCycleFile, const std::string& periodFile, double frequency):
		enableFile(enableFile),
		dutyCycleFile(dutyCycleFile),
		periodFile(periodFile),
		frequency(frequency),
		period(1'000'000'000 / frequency) {}
	};
	
	/// Parametry centralnej kamery
	const CameraCaptureParams centerCameraParams("/dev/video0", 320, 240, 60, 15, false, 0.3);
	
	/// Parametry lewej kamery
	const CameraCaptureParams leftCameraParams("/dev/video3", 1280, 720, 30, 7, false, 0.5);
	
	/// Parametry prawej kamery
	const CameraCaptureParams rightCameraParams("/dev/video1", 1280, 720, 30, 21, true, 0.5);
	
	/// Parametry oświetlenia
#ifndef GUI_DEBUG
	const PwmParams lucipherParams("/sys/class/pwm/pwmchip0/pwm0/enable", "/sys/class/pwm/pwmchip0/pwm0/duty_cycle", "/sys/class/pwm/pwmchip0/pwm0/period", 10'000);
#else
	const PwmParams lucipherParams("/tmp/pwm_enable", "/tmp/pwm_duty_cycle", "/tmp/pwm_period", 2);
#endif
	
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
	const std::function<std::string(unsigned int)> gpioControlFile = [](unsigned int gpio){
#ifndef GUI_DEBUG
		return("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");
#else
		return("/tmp/gui" + std::to_string(gpio) + "_value");
#endif
	};
	
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
