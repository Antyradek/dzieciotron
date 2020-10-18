#include "task.hpp"
#include <cassert>

using namespace dzieciotron;

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
		this->runLoop();
	}
}

