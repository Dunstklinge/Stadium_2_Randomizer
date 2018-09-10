#include "TrainerNames.h"

#include <fstream>
#include <sstream>

#include "StadiumStrings.h"

TrainerNames::TrainerNames()
{
}

/*
 * Format:
 * <number>::<catname>(::<tnumber>::<trainerName>)
 * <nickname>+
 */
TrainerNames::TrainerNames(const char* filename)
{
	std::ifstream file(filename);
	std::string line;

	GameInfo::TrainerCat currCat = GameInfo::NO_CATEGORY;
	GameInfo::TrainerNames currTrainer = GameInfo::TrainerNames::NO_TRAINER;
	while (getline(file, line)) {
		const char* buff = line.c_str();
		const char* it = buff;

		//skip initial white spaces
		while (*it && isspace(*it)) it++;
		const char* start = it;
		bool spaceAtStart = start == buff;
		const char* numberEnd;

		if (!*start) continue; //blank line

		unsigned long catNum = strtoul(it, (char**)&numberEnd, 10);
		if (numberEnd != it) {
			//a number was found, might be a new mon definition
			if (*numberEnd == ':' && numberEnd[1] == ':') {
				//new cat, look if its also a trainer number with it
				it = numberEnd + 2;
				while (*it && !(*it == ':' && it[1] == ':')) it++;
				if (*it) {
					const char* catName = it + 2;
					unsigned long tNum = strtoul(catName, (char**)&numberEnd, 10);
					if (it != catName) {
						//also a trainer number
						currTrainer = (GameInfo::TrainerNames)tNum;
					}
				}
				else {
					//no trainer def
					currTrainer = GameInfo::TrainerNames::NO_TRAINER;
				}
				currCat = (GameInfo::TrainerCat)catNum;
				continue;
			}
		}

		//its a nickname. find end of nickname excluding last whitespaces
		const char* end = it;
		while (*end) end++;
		while (end != it && isspace(end[-1])) end--;
		unsigned int nameLength = end - it;
		if (nameLength > MAX_NAME) {
			std::stringstream errorMsg;
			errorMsg << "Invalid Pokemon Name \"" << it << "\"!\r\n name is " << nameLength << " characters long, "
				"but max character length is " << MAX_NAME << ".";
			throw std::invalid_argument(errorMsg.str());
		}
		else {
			if (currTrainer != GameInfo::TrainerNames::NO_TRAINER) {
				trainerNicknameMap[currTrainer].push_back(std::string(it, end));
			}
			else {
				catNicknameMap[currCat].push_back(std::string(it, end));
			}
			
		}
	}

	Sanitize();
}


TrainerNames::~TrainerNames()
{
}

void TrainerNames::Sanitize()
{
	bool error = false;
	for (auto& vecs : catNicknameMap) {
		for (std::string& str : vecs.second) {
			std::string converted = Utf8ToStadiumString(str);
			str = converted;
		}
	}
	for (auto& vecs : trainerNicknameMap) {
		for (std::string& str : vecs.second) {
			std::string converted = Utf8ToStadiumString(str);
			str = converted;
		}
	}
}
