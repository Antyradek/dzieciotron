#include "pipeline.hpp"
#include <cassert>
#include <thread>
#include <chrono>
#include "logger.hpp"

using namespace pipeline;
using namespace utils;

Pipeline::Pipeline(const std::string& cameraFile, std::atomic<PipelineResult>& pipelineResult):
dzieciotron::AsyncTask(),
cameraFile(cameraFile),
pipelineResult(pipelineResult){}

void Pipeline::runLoop()
{
	Logger::debug() << "Obliczanie kamery " << this->cameraFile << "...";
	
	std::this_thread::sleep_for(std::chrono::seconds(1));
}


