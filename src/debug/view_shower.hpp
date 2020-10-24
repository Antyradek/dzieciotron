#pragma once
#include "task.hpp"
#include "pipeline_result.hpp"

namespace debug
{
/// Widok pokazuje wyjście na ekranie zamiast gdzieś wysyłać
class ViewShower: public dzieciotron::AsyncTask
{
private:
	/// Synchronizacja z której bierzemy obraz
	pipeline::AtomicPipelineResult& viewResult;
	
	/// Nazwa okna robi także za identyfikator
	const std::string windowName;
	
public:
	/// Pokazuje widok na ekranie
	ViewShower(pipeline::AtomicPipelineResult& viewResult);
	
	/// Wyświetla okienko
	void runLoop() override;
};
}
