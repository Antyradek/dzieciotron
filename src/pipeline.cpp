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

Pipeline::Pipeline(const std::string& cameraFile, std::atomic<PipelineResult>& pipelineResult):
dzieciotron::AsyncTask(),
cameraFile(cameraFile),
pipelineResult(pipelineResult),
videoCapture(),
videoWriter()
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
// 	const int width = videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
// 	const int height = videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
// 	if(!videoWriter.open("/tmp/out", cv::CAP_FFMPEG, cv::VideoWriter::fourcc('I', 'Y', 'U', 'V'), 60, cv::Size(width, height)))
// 	{
// 		throw(OutputError("Nie daje się otworzyć wyjścia"));
// 	}
}

void Pipeline::runLoop()
{
	//FIXME
	cv::Mat oneFrame;
	videoCapture >> oneFrame;
// 	cv::Mat otherSpaceFrame;
// 	cv::cvtColor(oneFrame, otherSpaceFrame, cv::COLOR_BGR2YUV_I420);
// 	fwrite(otherSpaceFrame.data, otherSpaceFrame.total(), 1, stdout);
	
	cv::Mat displayFrame;
	cv::hconcat(oneFrame, oneFrame, displayFrame);
	cv::cvtColor(displayFrame, displayFrame, cv::COLOR_BGR2YUV_I420);
	fwrite(displayFrame.data, displayFrame.total(), 1, stdout);
	
// 	videoWriter.write(oneFrame);
}


