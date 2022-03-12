#pragma once
#include <atomic>
#include <mutex>
#include <fstream>
#include "task.hpp"
#include "defines.hpp"

namespace externals
{
/// Pozwala kontrolować białe oświetlenie w łagodny sposób
class Lucipher: public dzieciotron::AsyncTask
{
private:
	/// Parametry działania
	const defines::PwmParams& params;
	
	/// Aktualna jasność oświetlenia w zakresie 0..1
	double lightness;
	
	/// Synchronizacja ustawiania jasności
	std::mutex lightMutex;
	
	/// Czas ostatniej zmiany oświetlenia
	std::chrono::time_point<std::chrono::steady_clock> lastLightChange;
	
	/// Wyjście do ustawiania jasności
	std::ofstream pwmOutput;
	
	/// Wyjście do aktywacji PWM
	std::ofstream pwmEnableOutput;
	
	/// Czy światło jest włączone
	bool lightEnabled;
	
	/// Główna pętla
	void runLoop() override;
	
	/// Ustawia obecną wartość światła
	void setLight();
	
public:
	/// Startuje w wyłączonym stanie
	Lucipher(const defines::PwmParams& params);
	
	/// Wyłącza co potrzebne
	virtual ~Lucipher();
	
	/// Rozjaśnij oświetlenie w zakresie 0..1, może być wołane z innych wątków
	void light(double value);
	
	/// Odczytaj aktualną jasność
	double light();
};
}
