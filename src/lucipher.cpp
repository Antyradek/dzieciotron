#include "lucipher.hpp"
#include <thread>
#include "logger.hpp"
#include "defines.hpp"

using namespace externals;
using namespace utils;

Lucipher::Lucipher():
lightness(0),
lightMutex(),
lastLightChange(std::chrono::steady_clock::now()) {}

void Lucipher::light(double value)
{
	//oświetlamy to natychmiast jaśniejszym światłem
	std::unique_lock<std::mutex> lock(this->lightMutex);
	if(value > this->lightness)
	{
		this->lightness = value;
		this->setLight();
		this->pause(false);
	}
}

void Lucipher::runLoop()
{
	{
		std::unique_lock<std::mutex> lock(this->lightMutex);
		const double deltaTime = static_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - this->lastLightChange).count();
		this->lightness = std::max(0.0, this->lightness - deltaTime * defines::lightFadeOffSpeed);
		this->setLight();
		if(this->lightness <= 0.0)
		{
			this->pause(true);
		}
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uintmax_t>(1000.0 / defines::lightFps)));
}

void Lucipher::setLight()
{
	Logger::debug() << "Ustawienie światła na " << this->lightness;
	//TODO
	this->lastLightChange = std::chrono::steady_clock::now();
}
