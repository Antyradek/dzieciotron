#pragma once
#include <string>
#include <atomic>
#include <fstream>
#include <list>
#include <chrono>
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
	
	/// Śledzone pozycje tych samych klastrów w przestrzeni pikselowej
	std::array<cv::Point2f, 4> detectives;
	
	/// Historia kolejnych pozycji detektywów
	std::list<std::array<cv::Point2f, 4>> detectiveHistory;
	
	/// Czas po zakończeniu przetwarzania ostatniej klatki
	std::chrono::time_point<std::chrono::steady_clock> lastFrameTime;
	
	/// Włącza i wyłącza światło diody
	void setDiode(bool on);
	
	/// Operacja aktualizacji tła
	void updateBackground();
	
	/// Zwraca kopię aktualnego tła (konstruktor kopiujący robi tylko płytką kopię)
	cv::Mat getBackground();
	
	/// Zwraca następną klatkę
	cv::Mat getFrame();
	
	/// Proguje luminancję klatki, zwraca binarny obraz
	cv::Mat thresholdLuminance(const cv::Mat& baseFrame) const;
	
	/// Znajduje środki klastrów w podanym obrazie binarnym
	std::vector<cv::Point2f> findClusters(const cv::Mat& binaryFrame, std::vector<std::vector<cv::Point>>& contours) const;
	
	/// Śledzi klastry przez detektywów
	void trackDetectives(const std::vector<cv::Point2f>& clusters);
	
public:
	/// Potrzebuje pliku i typu kamery, umieszcza wyjście w podanym pliku atomowym
	Pipeline(const defines::CameraCaptureParams& params, AtomicPipelineResult& pipelineResult);
	
	/// Zamyka połączenie do kamer
	virtual ~Pipeline() {}
	
	/// Główna pętla
	void runLoop() override;
};
}
