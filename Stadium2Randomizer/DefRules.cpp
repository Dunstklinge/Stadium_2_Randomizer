#include "stdafx.h"
#include "DefRules.h"

DefRules* DefRules::FromFile(const char* file)
{
	std::ifstream stream(file, std::ifstream::binary);
	DefRules* defText = FromFileStream(stream);
	return defText;
}

DefRules* DefRules::FromFileStream(std::istream& in)
{
	if (in.bad()) return nullptr;

	DefRules* ret = (DefRules*)new uint8_t[segSize];
	in.seekg(offStart);
	in.read((char*)ret, segSize);

	return ret;
}