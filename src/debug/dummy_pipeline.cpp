#include "dummy_pipeline.hpp"
#include <thread>
#include <opencv2/imgproc.hpp>
#include "defines.hpp"

using namespace debug;

DummyPipeline::DummyPipeline(pipeline::AtomicPipelineResult& pipelineResult):
pipelineResult(pipelineResult) {}

void DummyPipeline::runLoop()
{
	cv::Mat image = cv::Mat::zeros(defines::viewHeight, defines::viewWidth, CV_8UC3);
	//kilka kolorowych kółek
	for(int i = 0; i < 5; i++)
	{
		cv::circle(image, cv::Point(image.cols / 2, image.rows / 2), rand() % image.rows, cv::Scalar(rand() % 255, rand() % 255, rand() % 255), 5);
	}
	
	pipeline::PipelineResult result;
	result.view = image;
	this->pipelineResult.store(result);
	
	//spanko
	std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000));
}

