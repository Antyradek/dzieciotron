#pragma once
#include <fstream>
#include <string>

namespace utils
{
/// Otwórz plik
void openFile(const std::string& filename, std::ofstream& file);
}
