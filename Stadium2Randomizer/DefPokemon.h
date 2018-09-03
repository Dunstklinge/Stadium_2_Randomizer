#pragma once

#include "Constants.h"
#include "Util.h"

#include <iostream>
#include <stdint.h>

/*
 * Represents a pokemon as saved inside the rom so that a pointer into the file buffer can be casted to this struct.
 */
class DefPokemon
{
public:

	uint8_t level;
	GameInfo::PokemonId species;
	GameInfo::ItemId item;
	uint8_t unknown1;
	GameInfo::MoveId move1;
	GameInfo::MoveId move2;
	GameInfo::MoveId move3;
	GameInfo::MoveId move4;
	uint8_t unknown2;
	uint8_t happiness;

	uint16_t evHp;
	uint16_t evAtk;
	uint16_t evDef;
	uint16_t evSpd;
	uint16_t evSpc;
	uint16_t dvs;
	uint16_t unknown3;


	inline void Curate(bool forth) {
		SwitchEndianness(evHp);
		SwitchEndianness(evAtk);
		SwitchEndianness(evDef);
		SwitchEndianness(evSpd);
		SwitchEndianness(evSpc);
		SwitchEndianness(dvs);
	}

	void Print(std::ostream& o);
};

static_assert(sizeof(DefPokemon) == 24, "size missmatch");

