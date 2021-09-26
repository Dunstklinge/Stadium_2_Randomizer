#pragma once

#include "Constants.h"
#include "ItemTable.h"
#include "PokemonTable.h"
#include "MoveTable.h"


/*
 * This class holds references to information about pokemon, moves and items.
 * Taken by generator classes in case uber randomization has changed these properties.
 */
struct GameContext
{
	const GameInfo::Item* itemList;
	const GameInfo::Item* itemListEnd;
	const GameInfo::Pokemon* pokeList;
	const GameInfo::Pokemon* pokeListEnd;
	const GameInfo::Move* moveList;
	const GameInfo::Move* moveListEnd;
};

constexpr GameContext DefaultContext { 
	std::begin(GameInfo::ItemList), std::end(GameInfo::ItemList),
	std::begin(GameInfo::Pokemons), std::end(GameInfo::Pokemons),
	std::begin(GameInfo::Moves), std::end(GameInfo::Moves),
};