#pragma once
#include <string>

/// Generalne ustawienia programu
namespace defines
{
	/// Plik centralnej kamery
	const std::string centerCameraFile = "/dev/video0";
	
	/// Szerokość podglądu kamery
	const uintmax_t viewWidth = 320;
	
	/// Wysokość podglądu kamery
	const uintmax_t viewHeight = 240;
	
	/// Ilość klatek na sekundę podglądu
	const uintmax_t viewFps = 30;
	
}
