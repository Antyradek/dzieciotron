#pragma once
#include <string>
#include <opencv2/core/mat.hpp>
#include "task.hpp"
#include "pipeline_result.hpp"

namespace view
{
/// Wysyła podgląd w sieć
class ViewSender: public dzieciotron::AsyncTask
{
private:
	/// Nazwa pliku potoku
	std::string pipeName;
	
	/// Uchwyt do otwartego potoku
	int pipeHandle;
	
	/// Synchronizacja z której bierzemy obraz
	pipeline::AtomicPipelineResult& viewResult;
	
public:
	/// Będzie wysyłał dane na podany potok, synchronizowane
	ViewSender(const std::string& pipeName, pipeline::AtomicPipelineResult& viewResult);
	
	/// Usuwa stworzone potoki
	virtual ~ViewSender();
	
	/// Wyślij jedną ramkę
	void runLoop() override;
};
}
