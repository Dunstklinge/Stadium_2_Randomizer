#pragma once

#include <random>
#include <functional>

#include "DefPokemon.h"
#include "Tables.h"
#include "Constants.h"

class PokemonGenerator
{
public:
	PokemonGenerator();
	~PokemonGenerator();

	DefPokemon Generate();
	DefPokemon Generate(GameInfo::PokemonId species); //generates mon of this species
	DefPokemon Generate(const DefPokemon& from); //may keep info from this mon

	/* for Generate from an existing pokemon */
	bool changeSpecies;
	bool changeEvsIvs;
	bool changeLevel;
	bool changeMoves;
	bool changeItem;

	/* new generation info */ 
	
	//for species only, BOTH filter and buffer are used
	std::function<bool(GameInfo::PokemonId)> speciesFilter = nullptr; //for std::bind compatibility, even though its painfully slow
	const GameInfo::PokemonId* speciesFilterBuffer = nullptr;
	unsigned int speciesFilterBufferN;


	int minLevel;
	int maxLevel;
	bool levelUniformDistribution; //use binomial distribution if false

	//bool (*itemFilter)(GameInfo::ItemId) = nullptr;
	std::function<bool(GameInfo::ItemId, GameInfo::PokemonId)> itemFilter = nullptr;
	const GameInfo::ItemId* itemFilterBuffer = nullptr;
	unsigned int itemFilterBufferN;
	bool includeTypeSpeciesSpecific;

	//bool (*minOneMoveFilter)(GameInfo::MoveId move, GameInfo::PokemonId mon) = nullptr;
	std::function<bool(GameInfo::MoveId, GameInfo::PokemonId)> minOneMoveFilter = nullptr;
	const GameInfo::MoveId* minOneMoveFilterBuffer = nullptr;
	unsigned int minOneMoveFilterBufferN;

	std::function<bool(GameInfo::MoveId, GameInfo::PokemonId)> generalMoveFilter = nullptr;
	const GameInfo::MoveId* generalMoveFilterBuffer = nullptr;
	unsigned int generalMoveFilterBufferN = 0;
	
	bool randEvs;
	bool randIvs;
	bool bstEvIvs;
	bool statsUniformDistribution;

	inline void ClearAllFilters() {
		speciesFilter = nullptr, speciesFilterBuffer = nullptr;
		itemFilter = nullptr, itemFilterBuffer = nullptr;
		minOneMoveFilter = nullptr, minOneMoveFilterBuffer = nullptr;
		generalMoveFilter = nullptr, generalMoveFilterBuffer = nullptr;
	}
private:
	void GenSpecies(DefPokemon& mon);
	void GenLevel(DefPokemon& mon);
	void GenEvsIvs(DefPokemon& mon);
	void GenMoves(DefPokemon& mon);
	void GenItem(DefPokemon& mon);
};

