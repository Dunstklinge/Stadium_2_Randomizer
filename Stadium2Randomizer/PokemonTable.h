#pragma once

#include <stdint.h>

#include "Constants.h"

namespace GameInfo {

	struct Pokemon {
		uint8_t johtoDexNumber;
		uint8_t baseHp;
		uint8_t baseAtk;
		uint8_t baseDef;
		uint8_t baseSpd;
		uint8_t baseSpA;
		uint8_t baseSpD;
		uint8_t type1;
		uint8_t type2;
		uint8_t genderRatio;
		uint8_t unknowns[12];

		unsigned int CalcBST() { return baseHp + baseAtk + baseDef + baseSpd + baseSpA + baseSpD; }
	};
	static_assert(sizeof(Pokemon) == 22, "size missmatch");


	//moves are defined from 0x98f20 to 0x9a4b2
	uint8_t constexpr PokemonRawData[] = {
		#include "PokemonRawData.txt" 
	};

	extern Pokemon Pokemons[256];

	extern PokemonId PokemonIds[256];

	void InitPokemonTable();
}