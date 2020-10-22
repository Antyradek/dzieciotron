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
	pipeline::AtomicPipelineResult& leftResult;
	
	/// Rezultat przetwarzania środkowej kamery
	pipeline::AtomicPipelineResult& centerResult;
	
	/// Rezultat przetwarzania środkowej kamery
	pipeline::AtomicPipelineResult& rightResult;
	
	/// Rezultat do wysyłania podglądu obrazu
	pipeline::AtomicPipelineResult& viewResult;
	
public:
	/// Oblicza pozycję na podstawie rezultatów
	Locationer(pipeline::AtomicPipelineResult& leftResult, pipeline::AtomicPipelineResult& centerResult, pipeline::AtomicPipelineResult& rightResult, pipeline::AtomicPipelineResult& viewResult);
	
	/// Pusty destruktor
	virtual ~Locationer(){}
	
	/// Jedno przetworzenie
	void runLoop() override;
};
}
