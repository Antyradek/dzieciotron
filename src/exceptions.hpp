#pragma once
#include <stdexcept>

/// Błąd Dzieciotrona
class DzieciotronError : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

/// Błąd kamery
class CameraError : public DzieciotronError
{
	using DzieciotronError::DzieciotronError;
};

/// Błąd wyjścia pliku
class OutputError : public DzieciotronError
{
	using DzieciotronError::DzieciotronError;
};

/// Błąd licznika czasu
class TimerError : public DzieciotronError
{
	using DzieciotronError::DzieciotronError;
};

/// Błąd otwierania pliku
class FileError: public DzieciotronError
{
	using DzieciotronError::DzieciotronError;
};

/// Błąd USB
class UsbError: public DzieciotronError
{
	using DzieciotronError::DzieciotronError;
};

