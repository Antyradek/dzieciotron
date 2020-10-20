#include "pipeline_result.hpp"

using namespace pipeline;

void AtomicPipelineResult::store(const PipelineResult& result)
{
	const std::lock_guard guard(this->lock);
	
	this->result = result;
}

PipelineResult AtomicPipelineResult::load()
{
	const std::lock_guard guard(this->lock);
	return(this->result);
}
