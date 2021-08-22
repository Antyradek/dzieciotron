#pragma once
#include <atomic>
#include <mutex>
#include <task.hpp>

namespace externals
{
/// Pozwala kontrolować białe oświetlenie w łagodny sposób
class Lucipher: public dzieciotron::AsyncTask
{
private:
	/// Aktualna jasność oświetlenia w zakresie 0..1
	double lightness;
	
	/// Synchronizacja ustawiania jasności
	std::mutex lightMutex;
	
	/// Czas ostatniej zmiany oświetlenia
	std::chrono::time_point<std::chrono::steady_clock> lastLightChange;
	
	/// Główna pętla
	void runLoop() override;
	
	/// Ustawia obecną wartość światła
	void setLight();
	
public:
	/// Startuje w wyłączonym stanie
	Lucipher();
	
	/// Pusty destruktor
	virtual ~Lucipher() {};
	
	/// Rozjaśnij oświetlenie, może być wołane z innych wątków
	void light(double value);
};
}
