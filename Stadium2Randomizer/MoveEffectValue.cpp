#include "stdafx.h"
#include "MoveEffectValue.h"
#include "GlobalConfigs.h"

#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

MoveEffectValue::MoveEffectValue()
{

}

MoveEffectValue::MoveEffectValue(const char* filename)
{
	std::ifstream file(filename);
	std::string line;

	int lineCnt = 0;
	while (getline(file, line)) {
		lineCnt++;
		const char* it = line.c_str();

		//skip comments
		while (isspace(*it)) it++;
		if (!*it) continue;
		if (*it == '/' && *(it + 1) == '/') continue;

		//get name
		const char* nameStart = it;
		while (*it && !isspace(*it)) it++;
		const char* nameEnd = it;
		std::string_view name(nameStart, nameEnd - nameStart);
		
		//identify name
		auto foundIt = std::find_if(std::begin(GameInfo::MoveEffectInfos), std::end(GameInfo::MoveEffectInfos), 
			[name](const GameInfo::MoveEffectInfo& elem) {
				return elem.name == name;
			});
		if (foundIt == std::end(GameInfo::MoveEffectInfos)) {
			std::stringstream errorMsg;
			errorMsg << "Invalid Move effect name " << name << " in line " << lineCnt;
			throw std::invalid_argument(errorMsg.str());
		}
		GameInfo::MoveEffect id = (GameInfo::MoveEffect)(foundIt - GameInfo::MoveEffectInfos);

		//get value
		while (isspace(*it)) it++;
		if (!*it) {
			std::stringstream errorMsg;
			errorMsg << "Missing value in line " << lineCnt;
			throw std::invalid_argument(errorMsg.str());
		}

		const char* endPtr = it;
		Value val;

		do {
			double v = strtod(it, (char**)&endPtr);
			
			if (endPtr == it) {
				std::stringstream errorMsg;
				errorMsg << "Value in line " << lineCnt << " is not a valid number";
				throw std::invalid_argument(errorMsg.str());
			}
			char valType = *endPtr;
			if (valType == 'a') {
				val.affects |= Value::FLAT_BONUS;
				val.a = v;
			}
			else if (valType == 'm') {
				val.affects |= Value::MULT_BP;
				val.m = v;
			}
			else if (valType == 'c') {
				if (val.affects != 0) {
					std::stringstream errorMsg;
					errorMsg << "Error in value defintion in line " << lineCnt << ": Value modififer 'c' must only ever stay alone";
					throw std::invalid_argument(errorMsg.str());
				}
				val.affects |= Value::USE_VALUE;
				val.c = v;
			}
			else {
				std::stringstream errorMsg;
				errorMsg << "Value in line " << lineCnt << " has invalid type";
				throw std::invalid_argument(errorMsg.str());
			}

			it = endPtr + 1;
			//set up next number if exists
			if (isspace(*(it)) && isdigit(*(it+1))) {
				it = it+1;
			}
			else {
				break;
			}
		} while (true);

		while (isspace(*it)) it++;
		if (*it && !(*it == '/' && *(it + 1) == '/')) {
			std::stringstream errorMsg;
			errorMsg << "Stray characters at end of line " << lineCnt;
			throw std::invalid_argument(errorMsg.str());
		}

		//add data
		if (data.find(id) != data.end()) {
			std::stringstream errorMsg;
			errorMsg << "Line " << lineCnt << " defined id that was allready previously defined";
			throw std::invalid_argument(errorMsg.str());
		}
		data[id] = val;
	}
}


MoveEffectValue::~MoveEffectValue()
{
}

double RateMove(const GameInfo::Move& move)
{
	double result = 0;

	result = move.basePower * (move.accuracy / 255.0);
	if (move.pp < 20 && move.pp >= 15) {
		result *= 0.95;
	}
	else if (move.pp < 15 && move.pp >= 10) {
		result *= 0.9;
	}
	else if (move.pp < 10 && move.pp >= 7) {
		result *= 0.85;
	}
	else if (move.pp < 7 && move.pp >= 5) {
		result *= 0.8;
	}
	else if (move.pp < 5) {
		result *= move.pp / 5.0 * 0.8;
	}

	auto it = GlobalConfig::MoveEffectValues.data.find((GameInfo::MoveEffect)move.effectId);
	if (it != GlobalConfig::MoveEffectValues.data.end()) {
		auto info = it->second;
		bool chance = ((GameInfo::MoveEffectInfos[it->first].flags & GameInfo::MoveEffectInfo::CHANCE_PARAMETER) == GameInfo::MoveEffectInfo::CHANCE_PARAMETER);
		bool acc = ((GameInfo::MoveEffectInfos[it->first].flags & GameInfo::MoveEffectInfo::NOT_TARGETED) != GameInfo::MoveEffectInfo::NOT_TARGETED);
		const auto scaleIf = [&](double& d) {
			if (chance) { chance = false; d = d * (move.effectChance / 255.0); }
		};
		const auto scaleAccIf = [&](double& d) {
			if (acc) { acc = false; d = d * (move.accuracy / 255.0); }
		};

		if (info.affects & info.USE_VALUE) {
			scaleIf(info.c);
			scaleAccIf(info.c);
			result = info.c;
		}
		if (info.affects & info.FLAT_BONUS) {
			scaleIf(info.a);
			scaleAccIf(info.a);
			result += info.a;
		}
		if (info.affects & info.MULT_BP) {
			scaleIf(info.m);
			result += (info.m - 1) * move.basePower;
		}
	}
	return result;
}