#pragma once
#include <string>

/// Generalne ustawienia programu
namespace defines
{
	/// Parametry działania kamery
	struct CameraCaptureParams
	{
		/// Plik kamery
		std::string cameraFile;
		
		/// Szerokość obrazu
		uintmax_t width;
		
		/// Wysokość obrazu
		uintmax_t height;
		
		/// Liczba klatek na sekundę
		uintmax_t fps;
		
		CameraCaptureParams(const std::string& cameraFile, uintmax_t width, uintmax_t height, uintmax_t fps):
		cameraFile(cameraFile),
		width(width),
		height(height),
		fps(fps) {}
	};
	
	/// Parametry centralnej kamery
	const CameraCaptureParams centerCameraParams("/dev/video0", 320, 240, 60);
	
	/// Parametry lewej kamery
	const CameraCaptureParams leftCameraParams("/dev/video1", 1280, 720, 30);
	
	/// Parametry prawej kamery
	const CameraCaptureParams rightCameraParams("/dev/video3", 1280, 720, 30);
	
	/// Szerokość podglądu kamery
	const uintmax_t viewWidth = 320;
	
	/// Wysokość podglądu kamery
	const uintmax_t viewHeight = 240;
	
	/// Ilość klatek na sekundę podglądu
	const double viewFps = 10;
	
	/// Prędkość przetwarzania dla nieaktywnych potoków
	const double idleFps = 1;
	
	/// Ilość klatek na sekundę dla przetwarzania pozycji
	const double locationerFps = 60;
	
	/// Potok do którego zapisujemy podgląd
	const std::string viewPipe = "/tmp/dzieciotron.fifo";
	
}
