#pragma once
#include <string>
#include <atomic>
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
	
	//główne połączenie z kamerą lub plikiem wideo
	cv::VideoCapture videoCapture;
	
public:
	/// Potrzebuje pliku i typu kamery, umieszcza wyjście w podanym pliku atomowym
	Pipeline(const defines::CameraCaptureParams& params, AtomicPipelineResult& pipelineResult);
	
	/// Zamyka połączenie do kamer
	virtual ~Pipeline() {}
	
	/// Główna pętla
	void runLoop() override;
};
}