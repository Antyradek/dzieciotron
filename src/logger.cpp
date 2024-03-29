#include "logger.hpp" 
#include <iostream>

using namespace logger;

Logger::Logger(Logger::LoggerContext context): context(context) { }

Logger::~Logger()
{
	(void) context;
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

Logger Logger::warning()
{
	return(Logger(LoggerContext::Warning));
}

Logger Logger::error()
{
	return(Logger(LoggerContext::Error));
}





