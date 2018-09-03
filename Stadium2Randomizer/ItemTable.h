#pragma once

#include "Constants.h"

namespace GameInfo {
	struct Item {

	};

	extern ItemId ExistingItemMap[222];

	extern ItemId BattleItemMap[38];

	//excludes lightning ball, metal powder etc as well as type specific items
	extern ItemId GeneralBattleItemMap[17];

	extern ItemId NonPokemonBattleItemMap[33];

	//array[DITTO] = METALPOWDER etc
	extern ItemId SpeciesBattleItemMap[256];

};