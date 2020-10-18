#include "locationer.hpp"
#include <thread>
#include <chrono>
#include "logger.hpp"

using namespace locationer;
using namespace utils;

Locationer::Locationer(const std::atomic<pipeline::PipelineResult>& centerResult):
centerResult(centerResult){}

void Locationer::runLoop()
{
	Logger::debug() << "Obliczanie lokacji...";
	
	std::this_thread::sleep_for(std::chrono::seconds(2));
}

