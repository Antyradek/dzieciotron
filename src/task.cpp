#include "task.hpp"
#include <cassert>
#include <stdexcept>
#include "logger.hpp"

using namespace dzieciotron;
using namespace utils;

AsyncTask::AsyncTask():
isWorkingFlag(false) {}

void AsyncTask::stop()
{
	this->isWorkingFlag = false;
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
		}
		catch(const std::exception& err)
		{
			Logger::error() << "Błąd pracy zadania: " << err.what();
			throw;
		}
	}
}

