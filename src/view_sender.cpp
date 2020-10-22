#include "view_sender.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <opencv2/imgproc.hpp>
#include "defines.hpp"
#include "logger.hpp"
#include "exceptions.hpp"

using namespace view;
using namespace utils;

ViewSender::ViewSender(const std::string& pipeName, pipeline::AtomicPipelineResult& viewResult):
dzieciotron::AsyncTask(),
pipeName(pipeName),
pipeHandle(0),
viewResult(viewResult)
{
	//tworzenie nowej rury
	const int pipeRet = mkfifo(this->pipeName.c_str(), S_IRUSR | S_IWUSR);
	if(pipeRet < 0 && errno == EEXIST)
	{
		Logger::warning() << "Plik potoku " << this->pipeName << " istnieje";
	}
	else if(pipeRet != 0)
	{
		throw(OutputError("Błąd tworzenia potoku " + this->pipeName + ": " + strerror(errno)));
	}
}

ViewSender::~ViewSender()
{
	//zamknij
	if(this->pipeHandle > 0)
	{
		close(pipeHandle);
	}
	
	//ta operacja może się nie udać jeśli ktoś czyta
	unlink(this->pipeName.c_str());
}

void ViewSender::runLoop()
{
	//próbujemy otworzyć potok jeśli nie jest otwarty
	if(pipeHandle == 0)
	{
		//zobacz czy byłby otwarty
		const int openNonblockRet = open(this->pipeName.c_str(), O_WRONLY | O_NONBLOCK);
		if(openNonblockRet > 0)
		{
			//otwieranie na zapis
			const int openRet = open(this->pipeName.c_str(), O_WRONLY);
			if(openRet > 0)
			{
				this->pipeHandle = openRet;
				Logger::debug() << "Potok podglądu otwarty";
			}
			else
			{
				throw(OutputError(std::string("Błąd otwierania potoku: ") + strerror(errno)));;
			}
			
			//zamknięcie starego
			close(openNonblockRet);
		}
		else
		{
			Logger::warning() << "Błąd otwierania potoku " << this->pipeName << ": " << strerror(errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uintmax_t>(1000.0 / defines::idleFps)));
			return;
		}
	}
	
	//bierzemy obraz
	pipeline::PipelineResult result(this->viewResult.load());
	cv::Mat image(result.view);
	if(image.empty())
	{
		//poczekamy trochę
		Logger::debug() << "Pusty obraz do wyjścia";
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uintmax_t>(1000.0 / defines::idleFps)));
		return;
	}
	//zmień format
	assert(image.rows == defines::viewHeight);
	assert(image.cols == defines::viewWidth * 3);
	//po zmianie typu obraz może mieć inny rozmiar, ale nadal powinien być czytany przez wysyłacza z zaprogramowanym rozmiarem
	cv::cvtColor(image, image, cv::COLOR_BGR2YUV_I420);
	
	//zapisujemy do potoku
	const int writeRet = write(this->pipeHandle, image.data, image.total());
	if(writeRet < 0 || (writeRet > 0 && static_cast<size_t>(writeRet) != image.total()))
	{
		throw(OutputError(std::string("Błąd zapisu do potoku: ") + strerror(errno)));
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uintmax_t>(1000.0 / defines::viewFps)));
}
