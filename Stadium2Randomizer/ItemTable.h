#pragma once

#include "Constants.h"

namespace GameInfo {
	struct Item {
		ItemId id;
		enum {
			BattleItem = 1,
			TypeItem = 2,
			SpeciesItem = 4
		};
		int category;
		GameInfo::Type enhancedType;
		GameInfo::PokemonId enhancedSpecies;

		Item(ItemId id, int cat) : id(id), category(cat), enhancedType(Type::QUESTIONMARK_TYPE), enhancedSpecies(PokemonId::NO_POKEMON) {}
		Item(ItemId id, int cat, GameInfo::PokemonId poke) 
			: id(id), category(cat), enhancedType(Type::QUESTIONMARK_TYPE), enhancedSpecies(poke) {}
		Item(ItemId id, int cat, GameInfo::Type type)
			: id(id), category(cat), enhancedType(type), enhancedSpecies(PokemonId::NO_POKEMON) {}

	};
	extern const Item ItemList[222];

	extern const ItemId ExistingItemMap[222];

	extern const ItemId BattleItemMap[38];

	//excludes lightning ball, metal powder etc as well as type specific items
	extern const ItemId GeneralBattleItemMap[17];

	extern const ItemId NonPokemonBattleItemMap[33];

	//array[DITTO] = METALPOWDER etc
	extern const ItemId SpeciesBattleItemMap[256];

	//array[TYPE] = nevermeltice
	extern const ItemId TypeBattleItemMap[32];
};