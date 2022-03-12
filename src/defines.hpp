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
	
	/// Parametry resetowania Huba USB
	struct HubParams
	{
		/// Czas restartu Huba
		std::chrono::milliseconds hubRestartTime;
		
		/// VID PID huba sprzętowego
		std::pair<uint16_t, uint16_t> hardwareVidPid;
		
		/// VID PID huba systemowego
		std::pair<uint16_t, uint16_t> softwareVidPid;
		
		/// Flaga ustawień zasilania w kontroli USB
		unsigned int powerFlag;
		
		/// Port USB dla kontroli zasilania
		unsigned int port;
		
		HubParams(const std::chrono::milliseconds& restartTime, const std::pair<uint16_t, uint16_t>& hardwareVidPid, const std::pair<uint16_t, uint16_t>& softwareVidPid, unsigned int powerFlag, unsigned int port):
		hubRestartTime(restartTime),
		hardwareVidPid(hardwareVidPid),
		softwareVidPid(softwareVidPid),
		powerFlag(powerFlag),
		port(port) {}
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
	
	/// Parametry huba
	const HubParams hubParams(std::chrono::milliseconds(2000), {0x2109, 0x3431}, {0x1D6B, 0x0003}, 0b1000, 1);
	
	/// Timeout zmiany zasilania huba USB
	const std::chrono::milliseconds usbSetPowerTimeout(500);
	
	/// Czas przełączenia diody
	const std::chrono::milliseconds diodeToggleTime(100);
	
	/// Czas maksymalnego czasu inicjalizacji kamer
	const std::chrono::milliseconds maxVideoStartTime(5000);
	
	/// Czas oczekiwania po restarcie kamery
	const std::chrono::milliseconds cameraResetTime(1000);
	
	/// Czas bootowania się kamer
	const std::chrono::milliseconds cameraBootTime(2000);
	
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
	
	/// Prędkość spadku oświetlenia, spadek jasności na sekundę
	const double lightFadeOffSpeed = 0.3;
	
	/// Ilość odświeżeń na sekundę światła
	const double lightFps = 15;
	
	/// Prędkość wzrostu oświetlenia, wzrost jasności na sekundę
	const double lightOnSpeed = 0.1;
	
	/// Czas pomiędzy kolejnymi zwiększeniami jasności światła
	const std::chrono::milliseconds lightOnTime(500);
	
	/// Wielkość próbkowania koloru detektywa jako ułamek większego boku obrazu
	const double detectiveProbeRadiusFraction = 0.1;
	
	/// Kolory markerów jako kąt w przestrzeni H
	const std::array<double, 4> detectiveColors = {
		25, //pomarańczowy
		100, //zielony
		200, //niebieski
		250, //fioletowy
	};
	/// Nazwy kolejnych detektywów
	const std::array<std::string, 4> detectiveNames = {
		"Pomarańczowy",
		"Zielony",
		"Niebieski",
		"Fioletowy",
	};
	/// Maksymalna tolerancja różnicy kolorów w przestrzeni H
	const double detectiveColorMaxDistance = 5;
	/// Minimalna jasność detektywa w przestrzeni L
	const double detectiveMinLightness = 0.1;
	
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
