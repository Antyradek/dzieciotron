#include "locationer.hpp"
#include <thread>
#include <chrono>
#include <cassert>
#include <opencv2/imgproc.hpp>
#include "logger.hpp"

using namespace locationer;
using namespace logger;

Locationer::Locationer(pipeline::AtomicPipelineResult& leftResult, pipeline::AtomicPipelineResult& centerResult, pipeline::AtomicPipelineResult& rightResult, pipeline::AtomicPipelineResult& viewResult):
leftResult(leftResult),
centerResult(centerResult),
rightResult(rightResult),
viewResult(viewResult) {}

void Locationer::runLoop()
{
	pipeline::PipelineResult leftFrame(this->leftResult.load());
	pipeline::PipelineResult centerFrame(this->centerResult.load());
	pipeline::PipelineResult rightFrame(this->rightResult.load());
	
	if(leftFrame.view.empty() || centerFrame.view.empty() || rightFrame.view.empty())
	{
		//pewnie jeszcze nie zainicjalizowane
		Logger::debug() << "Puste wyjścia z następujących potoków: lewy=" << leftFrame.view.empty() << " centralny=" << centerFrame.view.empty() << " prawy=" << rightFrame.view.empty();
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uintmax_t>(1000.0 / defines::idleFps)));
		return;
	}
	
	cv::Mat tripleFrame(leftFrame.view);
	cv::hconcat(tripleFrame, centerFrame.view, tripleFrame);
	cv::hconcat(tripleFrame, rightFrame.view, tripleFrame);
	assert(tripleFrame.rows == defines::viewHeight);
	assert(tripleFrame.cols == defines::viewWidth * 3);
	pipeline::PipelineResult tripleResult;
	tripleResult.view = tripleFrame;
	this->viewResult.store(tripleResult);
	
	std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uintmax_t>(1000.0 / defines::locationerFps)));
}

