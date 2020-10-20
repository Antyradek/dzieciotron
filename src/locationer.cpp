#include "locationer.hpp"
#include <thread>
#include <chrono>
#include "logger.hpp"

using namespace locationer;
using namespace utils;

Locationer::Locationer(pipeline::AtomicPipelineResult& centerResult):
centerResult(centerResult){}

void Locationer::runLoop()
{
	Logger::debug() << "Obliczanie lokacji...";
	pipeline::PipelineResult result(this->centerResult.load());
	
	cv::Mat& displayFrame = result.view;
	fwrite(displayFrame.data, displayFrame.total(), 1, stdout);
	
	std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uintmax_t>(1000.0 / defines::viewFps)));
}

