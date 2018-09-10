#pragma once

#include <map>
#include <string>
#include <vector>

#include "Tables.h"

class TrainerNames
{
public:
	TrainerNames();
	TrainerNames(const char* filename);
	~TrainerNames();

	void Sanitize();

	std::map<GameInfo::TrainerCat, std::vector<std::string>> catNicknameMap;
	std::map<GameInfo::TrainerNames, std::vector<std::string>> trainerNicknameMap;

	static constexpr unsigned int MAX_NAME = 10;
};

