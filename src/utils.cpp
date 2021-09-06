#include "utils.hpp"
#include "exceptions.hpp"
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
