#pragma once
#include <atomic>
#include <condition_variable>

namespace dzieciotron
{
/// Zadanie uruchamiane w tle
class AsyncTask
{
private:
	/// Czy główna pętla ciągle działa
	std::atomic_bool isWorkingFlag;
	
	/// Mutex do pauzowania zadania
	std::mutex pauseMutex;
	
	/// Warunek do spauzowania zadania
	std::condition_variable pauseCondition;
	
	/// Informacja czy zadanie jest spauzowane
	std::atomic_bool isPaused;
	
protected:
	/// Domyślny konstruktor
	AsyncTask();
	
	/// Pusty destruktor
	virtual ~AsyncTask() {}
	
	/// Ustawia czy wykonywanie ma być wstrzymane, może być wołane z osobnego wątku
	void pause(bool pause);
	
	/// Wchodzi w główną pętlę
	virtual void runLoop() = 0;
	
public:
	/// Asynchronicznie zatrzymuje przetwarzanie, wołane z innego wątku
	void stop();
	
	/// Wchodzi do głównego przetwarzania
	void run();
};
}
