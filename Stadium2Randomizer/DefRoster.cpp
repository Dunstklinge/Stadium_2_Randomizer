#include "DefRoster.h"


#include <fstream>

#include "Util.h"

DefRoster* DefRoster::FromFile(const char* file) {
	std::ifstream stream(file, std::ifstream::binary);

	DefRoster* defRoster = FromFileStream(stream);

	return defRoster;
}

DefRoster * DefRoster::FromFileStream(std::istream& in)
{
	if (in.bad()) return nullptr;

	DefRoster* defRoster = (DefRoster*)new uint8_t[segSize];
	in.seekg(offStart);
	in.read((char*)defRoster, segSize);

	return defRoster;
}
