#pragma once

#include <stdint.h>
#include <vector>

#include "Constants.h"

namespace GameInfo {

	extern const PokemonId PokemonIds[251];

	struct Pokemon {
		uint8_t johtoDexNumber;
		uint8_t baseHp;
		uint8_t baseAtk;
		uint8_t baseDef;
		uint8_t baseSpd;
		uint8_t baseSpA;
		uint8_t baseSpD;
		uint8_t type1;
		uint8_t type2; //for single types, both types are the same here
		uint8_t genderRatio;
		uint8_t unknowns[2];
		uint8_t growthRate;
		struct {
			uint8_t eggGroup1 : 4;
			uint8_t eggGroup2 : 4;
		};
		uint8_t tms[8];	//glc only; lobit to hibit, left to right

		unsigned int CalcBST() const { return baseHp + baseAtk + baseDef + baseSpd + baseSpA + baseSpD; }
	};
	static_assert(sizeof(Pokemon) == 22, "size missmatch");

	//not actually part of the game, i made this myself
	/*struct EvoInfo {
		PokemonId preEvo;
		int lineLength;
	};*/
	extern PokemonId PokemonPrevEvo[256];

	//moves are defined from 0x98f20 to 0x9a4b2
	uint8_t constexpr PokemonRawData[] = {
		#include "PokemonRawData.txt" 
	};
	extern Pokemon Pokemons[256];

	//multiple of these might exist for the same move if multiple ways to optain it are present.
	struct LevelupMoveEntry {
		uint8_t level;	//level on which this is optained. 5 for egg moves, 1 for tut/events
		MoveId move;	//move this entry describes
		struct {		//the method of optaining of this move
			uint8_t rb : 1; //level-up in red/blue
			uint8_t y : 1;
			uint8_t gs : 1;
			uint8_t c : 1; //level-up in crystal
			uint8_t tut : 1; //move tutor (the goldenrod one in crystal)
			uint8_t eggC : 1; //egg move, but only in crystal
			uint8_t egg : 1; //egg move, also in g/s
			uint8_t event : 1; //special events, like surfing pikachu
		} flags;
	};

	uint8_t constexpr PokemonLevelupMovesRawData[] = {
		#include "LevelupMovesRawData.txt"
	};
	struct LevelupMoves {
		const LevelupMoveEntry* beginIt;	inline auto begin() { return beginIt; }
		const LevelupMoveEntry* endIt;		inline auto end() { return endIt; }
	};
	extern LevelupMoves PokemonLevelupMoveEntries[251];


	//one per pokemon with no padding
	struct PokemonGen1TMsEntry {
		uint8_t unknown;
		uint8_t tms[7]; //lo to hi, left to right
		uint8_t end;	//FF if last mon in list
	};

	constexpr uint8_t PokemonGen1TmsRawData[] = {
		#include "PokemonRBYTmList.txt"
	};

	extern PokemonGen1TMsEntry PokemonGen1Tms[151]; //actually rb only, no y




	extern std::vector<MoveId> PokemonLegalMoves_BaseList;
	struct PokemonLegalMovesEntry {
		struct {
			int beginIt;		inline std::vector<MoveId>::const_iterator begin() { return PokemonLegalMoves_BaseList.begin() + beginIt; }
			int endIt;			inline std::vector<MoveId>::const_iterator end() { return PokemonLegalMoves_BaseList.begin() + endIt; }
		} normal;
		struct {
			int beginIt;		inline std::vector<MoveId>::const_iterator begin() { return PokemonLegalMoves_BaseList.begin() + beginIt; }
			int endIt;			inline std::vector<MoveId>::const_iterator end() { return PokemonLegalMoves_BaseList.begin() + endIt; }

		} egg;
		int beginIt;		inline std::vector<MoveId>::const_iterator begin() { return PokemonLegalMoves_BaseList.begin() + beginIt; }
		int endIt;			inline std::vector<MoveId>::const_iterator end() { return PokemonLegalMoves_BaseList.begin() + endIt; }
	};
	extern PokemonLegalMovesEntry PokemonLegalMoves[251];

	void InitPokemonTable();
}