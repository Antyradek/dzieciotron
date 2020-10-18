#pragma once
#include <string>
#include <iostream>

namespace utils
{

/// Proste logowanie zdarzeń
class Logger
{
public:
	/// Gdzie logować
	enum class LoggerContext
	{
		Debug,
		Info,
		Warning,
		Error
	};
	
	/// Wypisuje obiekt na wyjście
	template <class T>
	Logger& operator<<(const T& item)
	{
		std::cerr << item;
		return(*this);
	}
	
	/// Zwraca logger debugujący
	static Logger debug();
	
	/// Zwraca logger informujący
	static Logger info();
	
	/// Wypisuje nową linię
	virtual ~Logger();
	
private:
	/// Kontekst wypisywania
	const LoggerContext context;
	
	/// Konstruuje pojedyncze lognięcie
	Logger(LoggerContext context);
	

	
};

}
