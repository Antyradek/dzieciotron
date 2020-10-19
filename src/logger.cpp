#include "logger.hpp" 
#include <iostream>

using namespace utils;

Logger::Logger(Logger::LoggerContext context): context(context) { }

Logger::~Logger()
{
	std::cerr << std::endl;
}

Logger Logger::debug()
{
	return(Logger(LoggerContext::Debug));
}

Logger Logger::info()
{
	return(Logger(LoggerContext::Info));
}

Logger Logger::error()
{
	return(Logger(LoggerContext::Error));
}





