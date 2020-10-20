#include "pipeline.hpp"
#include <cassert>
#include <thread>
#include <chrono>
#include <opencv2/imgproc.hpp>
#include "logger.hpp"
#include "exceptions.hpp"

#include <cstdio>

using namespace pipeline;
using namespace utils;

Pipeline::Pipeline(const std::string& cameraFile, AtomicPipelineResult& pipelineResult):
dzieciotron::AsyncTask(),
cameraFile(cameraFile),
pipelineResult(pipelineResult),
videoCapture()
{
	this->videoCapture.setExceptionMode(true);
	
	//FIXME
	videoCapture.open(this->cameraFile, cv::VideoCaptureAPIs::CAP_V4L2);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, 320);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, 240);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FPS, 60);
	cv::Mat firstFrame;
	videoCapture >> firstFrame;
	if(firstFrame.empty())
	{
		throw(CameraError("Pusty obraz"));
	}
}

void Pipeline::runLoop()
{
	//FIXME
	cv::Mat oneFrame;
	videoCapture >> oneFrame;
// 	cv::Mat otherSpaceFrame;
// 	cv::cvtColor(oneFrame, otherSpaceFrame, cv::COLOR_BGR2YUV_I420);
	
	cv::Mat displayFrame;
	displayFrame = oneFrame;
	cv::cvtColor(displayFrame, displayFrame, cv::COLOR_BGR2YUV_I420);
	
	//synchronizuje dane
	PipelineResult result;
	result.view = displayFrame;
	this->pipelineResult.store(result);
}


