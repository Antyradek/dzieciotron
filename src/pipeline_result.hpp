#pragma once
#include <array>
#include <mutex>
#include <opencv2/core/mat.hpp>
#include "defines.hpp"

namespace pipeline
{
/// Rezultat obliczeń jednego potoku
struct PipelineResult
{
	/// Podgląd obrazu
	cv::Mat view;
	
	PipelineResult():
	view() {}
};

/// Otoczenie rezultatu operacją atomową (std::atomic coś nie chce generować kodu)
class AtomicPipelineResult
{
private:
	/// Główna wartość
	PipelineResult result;
	
	/// Blokada na ustawianie i wczytywanie wartości
	std::mutex lock;
	
public:
	/// Atomowo umieść wartość
	void store(const PipelineResult& result);
	
	/// Atomowo wczytaj wartość
	PipelineResult load();
};
}
