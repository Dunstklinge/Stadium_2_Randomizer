#pragma once

#include <stdexcept>

#include "Constants.h"
#include "ItemTable.h"
#include "PokemonTable.h"
#include "MoveTable.h"
#include "MoveEffectValue.h"

/*
 * This class holds references to information about pokemon, moves and items.
 * Taken by generator classes in case uber randomization has changed these properties.
 * TODO: at some point ALL infos about the game should go through this.
 * The global arrays should not be used anywhere anymore.
 */
struct GameContext
{
	int ItemCount() const { return itemListEnd - itemList; }
	int PokeCount() const { return pokeListEnd - pokeList; }
	int MoveCount() const { return moveListEnd - moveList; }

	const GameInfo::Item& Item(GameInfo::ItemId id) const {
		if (id - 1 >= ItemCount()) throw std::invalid_argument("Invalid Item Id");
		return itemList[id - 1];
	}

	const GameInfo::Pokemon& Poke(GameInfo::PokemonId id) const {
		if (id - 1 >= PokeCount()) throw std::invalid_argument("Invalid Pokemon Id");
		return pokeList[id - 1];
	}

	const GameInfo::Move& Move(GameInfo::MoveId id) const {
		if (id - 1 >= MoveCount()) throw std::invalid_argument("Invalid Move Id");
		return moveList[id - 1];
	}

	constexpr GameContext(const GameInfo::Item* istart, const GameInfo::Item* iend,
		const GameInfo::Pokemon* pstart, const GameInfo::Pokemon* pend,
		const GameInfo::Move* mstart, const GameInfo::Move* mend)
		: itemList(istart), itemListEnd(iend), pokeList(pstart), pokeListEnd(pend),
		moveList(mstart), moveListEnd(mend)
	{}

	double LowestMoveRating() const;
	double HighestMoveRating() const;
	double LowestBst() const;
	double HighestBst() const;

	const GameInfo::Item* itemList;
	const GameInfo::Item* itemListEnd;
	const GameInfo::Pokemon* pokeList;
	const GameInfo::Pokemon* pokeListEnd;
	const GameInfo::Move* moveList;
	const GameInfo::Move* moveListEnd;
private:

	mutable struct {
		double lowestMoveRating = NAN;
		double highestMoveRating = NAN;
		double lowestBst = NAN;
		double highestBst = NAN;
	} calcCache;
};

constexpr GameContext DefaultContext = GameContext(
	std::begin(GameInfo::ItemList), std::end(GameInfo::ItemList),
	std::begin(GameInfo::Pokemons), std::end(GameInfo::Pokemons),
	std::begin(GameInfo::Moves), std::end(GameInfo::Moves)
);