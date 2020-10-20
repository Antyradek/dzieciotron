#pragma once
#include <atomic>
#include "pipeline_result.hpp"
#include "task.hpp"
#include "view_sender.hpp"

/// Funkcje obliczania pozycji
namespace locationer
{

/// Oblicza pozycję 3D
class Locationer: public dzieciotron::AsyncTask
{
private:
	/// Rezultat przetwarzania środkowej kamery
	pipeline::AtomicPipelineResult& centerResult;
	
	/// Wysyłacz podglądu obrazu
	view::ViewSender viewSender;
	
public:
	/// Oblicza pozycję na podstawie rezultatów
	Locationer(pipeline::AtomicPipelineResult& centerResult);
	
	/// Pusty destruktor
	virtual ~Locationer(){}
	
	/// Jedno przetworzenie
	void runLoop() override;
};
}
