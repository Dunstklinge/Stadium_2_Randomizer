#include "TrainerNames.h"

#include <fstream>

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
			//TODO: give warning
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
}


TrainerNames::~TrainerNames()
{
}
