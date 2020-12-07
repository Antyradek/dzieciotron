#pragma once
#include <string>
#include <atomic>
#include <fstream>
#include <opencv2/videoio.hpp>
#include "pipeline_result.hpp"
#include "task.hpp"
#include "defines.hpp"

/// Funkcje do przetwarzania potokowego obszarów z jednej kamery
namespace pipeline
{
	
/// Jeden potok do przetwarzania obrazu z kamery
class Pipeline: public dzieciotron::AsyncTask
{
private:
	
	/// Plik odczytu kamery
	const defines::CameraCaptureParams& params;
	
	/// Miejsce gdzie wsadzamy rezultat
	AtomicPipelineResult& pipelineResult;
	
	/// Główne połączenie z kamerą lub plikiem wideo
	cv::VideoCapture videoCapture;
	
	/// Wyjście sterowania GPIO
	std::ofstream gpioOutput;
	
	/// Tło do odejmowania 
	cv::Mat background;
	
	/// Włącza i wyłącza światło diody
	void setDiode(bool on);
	
	/// Operacja aktualizacji tła
	void updateBackground();
	
	/// Zwraca kopię aktualnego tła (konstruktor kopiujący robi tylko płytką kopię)
	cv::Mat getBackground();
	
	/// Zwraca następną klatkę
	cv::Mat getFrame();
	
public:
	/// Potrzebuje pliku i typu kamery, umieszcza wyjście w podanym pliku atomowym
	Pipeline(const defines::CameraCaptureParams& params, AtomicPipelineResult& pipelineResult);
	
	/// Zamyka połączenie do kamer
	virtual ~Pipeline() {}
	
	/// Główna pętla
	void runLoop() override;
};
}
