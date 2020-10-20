#include "view_sender.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "logger.hpp"
#include "exceptions.hpp"

using namespace view;
using namespace utils;

ViewSender::ViewSender(const std::string& pipeName):
pipeName(pipeName),
pipeHandle(0)
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

void ViewSender::send(const cv::Mat& image)
{
	//próbujemy otworzyć potok jeśli nie jest otwarty
	if(pipeHandle == 0)
	{
		//otwieranie na zapis
		const int openRet = open(this->pipeName.c_str(), O_WRONLY);
		if(openRet > 0)
		{
			this->pipeHandle = openRet;
		}
		else
		{
			Logger::warning() << "Błąd otwierania potoku " << this->pipeName << ": " << strerror(errno);
			return;
		}
	}
	
	const int writeRet = write(this->pipeHandle, image.data, image.total());
	if(writeRet < 0 && errno == EAGAIN)
	{
		Logger::warning() << "Zapis do potoku próbował blokować";
	}
	else if(writeRet < 0 || (writeRet > 0 && static_cast<size_t>(writeRet) != image.total()))
	{
		throw(OutputError(std::string("Błąd zapisu do potoku: ") + strerror(errno)));
	}
	Logger::debug() << "Zapisano: " << writeRet << "/" << image.total() << " B";
}
