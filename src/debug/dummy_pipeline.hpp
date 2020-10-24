#pragma once
#include "task.hpp"
#include "pipeline_result.hpp"

namespace debug
{
/// Sztuczna klasa generująca podgląd
class DummyPipeline: public dzieciotron::AsyncTask
{
private:
	/// Miejsce gdzie wsadzamy rezultat
	pipeline::AtomicPipelineResult& pipelineResult;
	
public:
	/// Generuje sztuczne informacje
	DummyPipeline(pipeline::AtomicPipelineResult& pipelineResult);
	
	/// Generuje jedną ramkę raz na jakiś czas
	void runLoop() override;
};
}
