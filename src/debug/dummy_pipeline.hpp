#pragma once
#include "task.hpp"
#include "pipeline_result.hpp"
#include "lucipher.hpp"

namespace debug
{
/// Sztuczna klasa generująca podgląd
class DummyPipeline: public dzieciotron::AsyncTask
{
private:
	/// Miejsce gdzie wsadzamy rezultat
	pipeline::AtomicPipelineResult& pipelineResult;
	
	/// Kontrola oświetlenia
	externals::Lucipher& lucipher;
	
	/// Generuje jedną ramkę raz na jakiś czas
	void runLoop() override;
	
public:
	/// Generuje sztuczne informacje
	DummyPipeline(pipeline::AtomicPipelineResult& pipelineResult, externals::Lucipher& lucipher);
	
	/// Pusty destruktor
	virtual ~DummyPipeline() {}
};
}
