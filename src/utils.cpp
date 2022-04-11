#include "utils.hpp"
#include "exceptions.hpp"
#include <sstream>
#include <iomanip>
using namespace utils;

void utils::openFile(const std::string& filename, std::ofstream& file)
{
	file.exceptions(std::ofstream::badbit | std::ofstream::failbit);
	try
	{
		file.open(filename, std::ofstream::out);
	}
	catch(const std::ofstream::failure& err)
	{
		throw(FileError("Błąd otwierania pliku " + filename));
	}
}

std::string utils::printHexColor(double red, double green, double blue)
{
	std::stringstream ss;
	ss << "#" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<unsigned int>(red * 255) << std::setw(2) << static_cast<unsigned int>(green * 255) << std::setw(2) << static_cast<unsigned int>(blue * 255);
	return(ss.str());
}
