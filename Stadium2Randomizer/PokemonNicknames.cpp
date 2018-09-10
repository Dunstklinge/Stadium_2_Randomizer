#include "PokemonNicknames.h"

#include <fstream>
#include <sstream>

#include "StadiumStrings.h"


PokemonNicknames::PokemonNicknames()
{
}

/*
 * Format:
 * <number>::<pokemonname>
 * <nickname>+
 */
PokemonNicknames::PokemonNicknames(const char* filename)
{
	std::ifstream file(filename);
	std::string line;

	GameInfo::PokemonId currMon = GameInfo::NO_POKEMON;
	while (getline(file, line)) {
		const char* buff = line.c_str();
		const char* it = buff;

		//skip initial white spaces
		while (*it && isspace((unsigned char)buff[*it])) it++;
		const char* start = it;
		bool spaceAtStart = start == buff;
		const char* numberEnd;

		if (!*it) continue; //skip empty lines

		unsigned long number = strtoul(it, (char**)&numberEnd, 10);
		if (numberEnd != it) {
			//a number was found, might be a new mon definition
			if (*numberEnd == ':' && numberEnd[1] == ':') {
				//its a pokemon name
				const char* monName = numberEnd + 2;
				currMon = (GameInfo::PokemonId)number;
				continue;
				
			}
		}
		//its a nickname. find end of nickname excluding last whitespaces
		const char* end = it;
		while (*end) end++;
		while (end != it && isspace((unsigned char)end[-1])) end--;
		unsigned int nameLength = end - it;
		if (nameLength > MAX_NAME) {
			std::stringstream errorMsg;
			errorMsg << "Invalid Pokemon Name \"" << it << "\"!\r\n name is " << nameLength << " characters long, "
				"but max character length is " << MAX_NAME << ".";
			throw std::invalid_argument(errorMsg.str());
		}
		else {
			nicknameMap[currMon].push_back(std::string(it, end));
		}
	}

	Sanitize();
}


PokemonNicknames::~PokemonNicknames()
{
}

void PokemonNicknames::Sanitize()
{
	bool error = false;
	for (auto& vecs : nicknameMap) {
		for (std::string& str : vecs.second) {
			std::string converted = Utf8ToStadiumString(str);
			str = converted;
		}
	}
}

