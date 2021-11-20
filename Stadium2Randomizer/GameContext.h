#pragma once

#include "Constants.h"
#include "ItemTable.h"
#include "PokemonTable.h"
#include "MoveTable.h"
#include "MoveEffectValue.h"

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

	constexpr GameContext(const GameInfo::Item* istart, const GameInfo::Item* iend,
		const GameInfo::Pokemon* pstart, const GameInfo::Pokemon* pend,
		const GameInfo::Move* mstart, const GameInfo::Move* mend)
		: itemList(istart), itemListEnd(iend), pokeList(pstart), pokeListEnd(pend),
		moveList(mstart), moveListEnd(mend)
	{}

	double LowestMoveRating() const;
	double HighestMoveRating() const;
private:
	mutable struct {
		double lowestMoveRating = NAN;
		double highestMoveRating = NAN;
	} calcCache;
};

constexpr GameContext DefaultContext = GameContext(
	std::begin(GameInfo::ItemList), std::end(GameInfo::ItemList),
	std::begin(GameInfo::Pokemons), std::end(GameInfo::Pokemons),
	std::begin(GameInfo::Moves), std::end(GameInfo::Moves)
);