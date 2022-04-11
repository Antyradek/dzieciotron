#pragma once
#include <fstream>
#include <string>

namespace utils
{
/// Otwórz plik
void openFile(const std::string& filename, std::ofstream& file);

/// Zamień podane parametry na kolor "#RRGGBB"
std::string printHexColor(double red, double green, double blue);
}

