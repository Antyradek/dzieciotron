#include "logger.hpp" 
#include <iostream>

using namespace dzieciotron;

Logger::Logger(Logger::LoggerContext context): context(context) { }

Logger::~Logger()
{
	std::cerr << std::endl;
}

Logger Logger::debug()
{
	return(Logger(LoggerContext::Debug));
}





