#pragma once
#include <string>
#include <opencv2/core/mat.hpp>

namespace view
{
/// Wysyła podgląd w sieć
class ViewSender
{
private:
	/// Nazwa pliku potoku
	std::string pipeName;
	
	/// Uchwyt do otwartego potoku
	int pipeHandle;
	
public:
	/// Będzie wysyłał dane na podany potok
	ViewSender(const std::string& pipeName);
	
	/// Usuwa stworzone potoki
	virtual ~ViewSender();
	
	/// Zapisz jeden obraz
	void send(const cv::Mat& image);
};
}
