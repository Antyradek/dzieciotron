#include "view_shower.hpp"
#include <thread>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "logger.hpp"

using namespace debug;
using namespace utils;

ViewShower::ViewShower(pipeline::AtomicPipelineResult& viewResult):
viewResult(viewResult),
//nazwa okna nie może zawierać polskich liter, chyba że stworzymy okno przed aktualizacją, ale wtedy nie można tego robić z innego wątku
windowName("Kamery") { }

void ViewShower::runLoop()
{
	pipeline::PipelineResult result(this->viewResult.load());
	cv::Mat& image(result.view);
	
	if(image.empty())
	{
		//innym razem
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uintmax_t>(1000.0 / defines::idleFps)));
		return;
	}
	
	cv::imshow(this->windowName, image);
	//potrzebne aby odświeżyć obraz, blokuje na chwilę
	cv::waitKey(5);
}
