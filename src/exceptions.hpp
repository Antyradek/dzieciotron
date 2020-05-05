#pragma once
#include <stdexcept>

/// Błąd kamery
class CameraError : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

/// Błąd licznika czasu
class TimerError : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};


