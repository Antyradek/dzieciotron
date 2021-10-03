#pragma once
#include <stdexcept>

/// Błąd kamery
class CameraError : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

/// Błąd wyjścia pliku
class OutputError : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

/// Błąd licznika czasu
class TimerError : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

/// Błąd otwierania pliku
class FileError: public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

/// Błąd USB
class UsbError: public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

