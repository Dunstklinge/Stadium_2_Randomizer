#pragma once

#include <stdint.h>
#include <fstream>

#include "Util.h"

/*
 * Represents a set of cup rules inside the rom so that a pointer to the file buffer can be cast to this struct
 */
class DefRule
{
public:
	uint8_t unknown1[0x14]; //always 0?
	uint8_t legalMons[0x20]; //bitfield, MSB FIRST, so 0x80 bans bulbasaur
	uint16_t thisIndex; //literally the index of this struct in the table
	uint8_t minLevel;
	uint8_t maxLevel;
	uint16_t maxTotalLevel;
	uint8_t unknown2[0x2];
	uint16_t rentalTableIndex;
	uint8_t unknown3[0x6];

	inline void Curate(bool forth) {
		SwitchEndianness(thisIndex);
		SwitchEndianness(maxTotalLevel);
		SwitchEndianness(rentalTableIndex);
	}
};

static_assert(sizeof(DefRule) == 68, "size missmatch");

/*
 * Represents all cup rules inside the rom so that a pointer to the file buffer can be cast to this struct
 */
class DefRules {
public:
	union {
		struct {
			DefRule littleCup;
			DefRule challengeCup1;
			DefRule challengeCup2;
			DefRule challengeCup3;
			DefRule challengeCup4;
			DefRule freeBattle;
			DefRule glc;
			DefRule pokeCupPoke;
			DefRule pokeCupSuper;
			DefRule pokeCupUltra;
			DefRule pokeCupMaster;
			DefRule rival;
			DefRule primeCupR1;
			DefRule primeCupR2;
		};
		DefRule rules[14];
	};

	inline void Curate(bool forth) {
		for (int i = 0; i < _countof(rules); i++) {
			rules[i].Curate(forth);
		}
	}

	static constexpr unsigned long offStart = 0xA0070;
	static constexpr unsigned long offEnd = 0xA0428;
	static constexpr unsigned long segSize = offEnd - offStart;


	static DefRules* FromFile(const char* file);
	static DefRules* FromFileStream(std::istream& in);
};