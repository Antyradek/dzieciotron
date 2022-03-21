// #include <iostream>
// #include <fstream>
// #include <ctime>
#include <csignal>
#include <cstring>
// #include <map>
#include <future>
#include <functional>
#include <array>
#include <algorithm>


// 
// #include <boost/program_options.hpp>
// 
// #include "exceptions.hpp"
#include "logger.hpp"
#include "pipeline.hpp"
#include "defines.hpp"
#include "locationer.hpp"
#include "view_sender.hpp"
#include "hubber.hpp"

#ifdef GUI_DEBUG
	#include "debug/dummy_pipeline.hpp"
	#include "debug/view_shower.hpp"
	#include "debug/dummy_hubber.hpp"
#endif

using namespace logger;

/// Callback sygnału
namespace globals
{
std::function<void()> exitCallback;
static void signalCallback(int signum)
{
	Logger::info() << "Sygnał " << strsignal(signum);
	exitCallback();
}
}

int main()
{	
	//TODO Użyć "v4l2-sysfs-path -d" do ogarnięcia które kamery są które albo zajrzeć do /dev
	
	pipeline::AtomicPipelineResult leftResult;
	pipeline::AtomicPipelineResult centerResult;
	pipeline::AtomicPipelineResult rightResult;
	pipeline::AtomicPipelineResult viewResult;
	
	externals::Lucipher lucipher(defines::lucipherParams);
#ifndef GUI_DEBUG
	externals::Hubber hubber(defines::hubParams);
#else
	debug::DummyHubber hubber;
#endif
	
	pipeline::Pipeline centerPipeline(defines::centerCameraParams, centerResult, lucipher, hubber);
	locationer::Locationer locationer(leftResult, centerResult, rightResult, viewResult);
	
	//w debugu potoki dwóch pozostałych kamer są sztuczne
#ifndef GUI_DEBUG
	pipeline::Pipeline leftPipeline(defines::leftCameraParams, leftResult, lucipher, hubber);
	pipeline::Pipeline rightPipeline(defines::rightCameraParams, rightResult, lucipher, hubber);
	view::ViewSender viewSender(defines::viewPipe, viewResult);
#else
	debug::DummyPipeline leftPipeline(leftResult, lucipher);
	debug::DummyPipeline rightPipeline(rightResult, lucipher);
	debug::ViewShower viewSender(viewResult);
#endif
	
	std::array<std::reference_wrapper<dzieciotron::AsyncTask>, 7> tasks {{centerPipeline, leftPipeline, rightPipeline, locationer, viewSender, lucipher, hubber}};
	
	//obsługa sygnału
	globals::exitCallback = [&tasks](){
		for(dzieciotron::AsyncTask& task: tasks)
		{
			task.stop();
		}
	};
	signal(SIGINT, globals::signalCallback);
	signal(SIGTERM, globals::signalCallback);
	signal(SIGPIPE, SIG_IGN);
	
	//wystartuj
	std::array<std::future<void>, tasks.size()> futures;
	//dla każdego elementu w tablicy zadań, wywołaj funkcję, która stworzy przyszłościowe zadanie i wsadzi do drugiej tablicy
	std::transform(tasks.begin(), tasks.end(), futures.begin(), [](dzieciotron::AsyncTask& task) -> std::future<void>{
		//wystartuj zadanie w osobnym wątku
		return(std::async(std::launch::async, [&task](){
			task.run();
		}));
	});
	
	//czekaj na zakończenie
	for(std::future<void>& future: futures)
	{
		future.get();
	}
	
}
