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

Pipeline::Pipeline(const defines::CameraCaptureParams& params, AtomicPipelineResult& pipelineResult):
dzieciotron::AsyncTask(),
params(params),
pipelineResult(pipelineResult),
videoCapture()
{
	this->videoCapture.setExceptionMode(true);
	
	videoCapture.open(params.cameraFile, cv::VideoCaptureAPIs::CAP_V4L2);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, params.width);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, params.height);
	videoCapture.set(cv::VideoCaptureProperties::CAP_PROP_FPS, params.fps);
	cv::Mat firstFrame;
	videoCapture >> firstFrame;
	if(firstFrame.empty())
	{
		throw(CameraError("Pusty obraz"));
	}
	
	Logger::debug() << "Ustawienia kamery: " << params.cameraFile << " " << params.width << "×" << params.height << "p" << params.fps << " → " << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH) << "×" << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT) << "p" << videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FPS);
}

void Pipeline::runLoop()
{
	cv::Mat oneFrame;
	videoCapture >> oneFrame;
	
	cv::Mat displayFrame(oneFrame);
	//zmień wielkość obrazu
	if(displayFrame.rows != defines::viewHeight || displayFrame.cols != defines::viewWidth)
	{
		cv::resize(displayFrame, displayFrame, cv::Size(defines::viewWidth, defines::viewHeight));
	}
	assert(displayFrame.rows == defines::viewHeight);
	assert(displayFrame.cols == defines::viewWidth);
	
	//synchronizuje dane
	PipelineResult result;
	result.view = displayFrame;
	this->pipelineResult.store(result);
}


