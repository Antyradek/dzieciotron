#include "task.hpp"
#include <cassert>
#include <stdexcept>
#include "logger.hpp"

using namespace dzieciotron;
using namespace logger;

AsyncTask::AsyncTask():
isWorkingFlag(false),
pauseMutex(),
pauseCondition(),
isPaused(false) {}

void AsyncTask::stop()
{
	this->isWorkingFlag = false;
	this->pause(false);
}

void AsyncTask::pause(bool pause)
{
	//być może w C++20 dało by się użyć notify_all() ze std::atomic
	//wtedy nie trzeba by mutexa i zmiennej warunkowej
	if(pause)
	{
		this->isPaused = true;
	}
	else
	{
		std::lock_guard<std::mutex> lock(this->pauseMutex);
		this->isPaused = false;
		this->pauseCondition.notify_one();
	}
}

void AsyncTask::run()
{
	assert(!this->isWorkingFlag);
	this->isWorkingFlag = true;
	
	while(this->isWorkingFlag)
	{
		try
		{
			this->runLoop();
			if(this->isPaused)
			{
				//zawieszenie się na zmiennej
				std::unique_lock<std::mutex> lock(this->pauseMutex);
				this->pauseCondition.wait(lock, [this](){return(!this->isPaused);});
			}
		}
		catch(const std::exception& err)
		{
			Logger::error() << "Błąd pracy zadania: " << err.what();
			throw;
		}
	}
}

