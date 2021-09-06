#include "lucipher.hpp"
#include <thread>
#include <map>
#include <initializer_list>
#include <functional>
#include "logger.hpp"
#include "defines.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

using namespace externals;
using namespace logger;

Lucipher::Lucipher(const defines::PwmParams& params):
params(params),
lightness(0),
lightMutex(),
lastLightChange(std::chrono::steady_clock::now()),
pwmOutput(),
pwmEnableOutput(),
lightEnabled(false)
{
	//PWM
	std::ofstream periodFile;
	for(auto& pair : std::vector<std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<std::ofstream>>>({{std::cref(this->params.periodFile), std::ref(periodFile)}, {std::cref(this->params.dutyCycleFile), std::ref(this->pwmOutput)}, {std::cref(this->params.enableFile), std::ref(this->pwmEnableOutput)}}))
	{
		auto& [filename, file] = pair;
		utils::openFile(filename, file);
	}
	this->pwmEnableOutput << "0" << std::endl;
	this->pwmOutput << "0" << std::endl;
	periodFile << this->params.period << std::endl;
}

Lucipher::~Lucipher()
{
	this->lightness = 0;
	this->setLight();
}

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
	this->pwmOutput << static_cast<uintmax_t>(this->lightness * this->params.period) << std::endl;
	if(this->lightness == 0 && this->lightEnabled)
	{
		this->pwmEnableOutput << "0" << std::endl;
		this->lightEnabled = false;
	}
	else if(this->lightness > 0 && !this->lightEnabled)
	{
		this->pwmEnableOutput << "1" << std::endl;
		this->lightEnabled = true;
	}
	
	this->lastLightChange = std::chrono::steady_clock::now();
}
