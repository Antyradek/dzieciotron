#pragma once
#include <atomic>

namespace dzieciotron
{
/// Zadanie uruchamiane w tle
class AsyncTask
{
private:
	/// Czy główna pętla ciągle działa
	std::atomic_bool isWorkingFlag;
	
protected:
	/// Domyślny konstruktor
	AsyncTask();
	
	/// Pusty destruktor
	virtual ~AsyncTask() {}
	
	/// Wchodzi w główną pętlę
	virtual void runLoop() = 0;
	
public:
	/// Asynchronicznie zatrzymuje przetwarzanie, wołane z innego wątku
	void stop();
	
	/// Wchodzi do głównego przetwarzania
	void run();
};
}
