#include "DefText.h"

DefText* DefText::FromFile(const char* file)
{
	std::ifstream stream(file, std::ifstream::binary);
	DefText* defText = FromFileStream(stream);
	return defText;
}

DefText* DefText::FromFileStream(std::istream& in)
{
	if (in.bad()) return nullptr;

	DefText* ret = (DefText*)new uint8_t[segSize];
	in.seekg(offStart);
	in.read((char*)ret, segSize);

	return ret;
}
