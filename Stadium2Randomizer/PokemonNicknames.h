#pragma once

#include <map>
#include <string>
#include <vector>

#include "Tables.h"

class PokemonNicknames
{
public:
	PokemonNicknames();
	PokemonNicknames(const char* filename);
	~PokemonNicknames();

	std::map<GameInfo::PokemonId, std::vector<std::string>> nicknameMap;

	bool keepOriginalNames = true;
	static constexpr unsigned int MAX_NAME = 10;
};

