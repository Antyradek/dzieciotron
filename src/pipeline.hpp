#pragma once
#include <string>
#include <atomic>
#include <opencv2/videoio.hpp>
#include "pipeline_result.hpp"
#include "task.hpp"

/// Funkcje do przetwarzania potokowego obszarów z jednej kamery
namespace pipeline
{
	
/// Jeden potok do przetwarzania obrazu z kamery
class Pipeline: public dzieciotron::AsyncTask
{
private:
	
	/// Plik odczytu kamery
	std::string cameraFile;
	
	/// Miejsce gdzie wsadzamy rezultat
	std::atomic<PipelineResult>& pipelineResult;
	
	//główne połączenie z kamerą lub plikiem wideo
	cv::VideoCapture videoCapture;
	
	//FIXME
	cv::VideoWriter videoWriter;
	
public:
	/// Potrzebuje pliku i typu kamery, umieszcza wyjście w podanym pliku atomowym
	Pipeline(const std::string& cameraFile, std::atomic<PipelineResult>& pipelineResult);
	
	/// Zamyka połączenie do kamer
	virtual ~Pipeline() {}
	
	/// Główna pętla
	void runLoop() override;
};
}
