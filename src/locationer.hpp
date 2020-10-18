#pragma once
#include <atomic>
#include "pipeline_result.hpp"
#include "task.hpp"

/// Funkcje obliczania pozycji
namespace locationer
{

/// Oblicza pozycję 3D
class Locationer: public dzieciotron::AsyncTask
{
private:
	/// Rezultat przetwarzania środkowej kamery
	const std::atomic<pipeline::PipelineResult>& centerResult;
	
public:
	/// Oblicza pozycję na podstawie rezultatów
	Locationer(const std::atomic<pipeline::PipelineResult>& centerResult);
	
	/// Pusty destruktor
	virtual ~Locationer(){}
	
	/// Jedno przetworzenie
	void runLoop() override;
};
}
