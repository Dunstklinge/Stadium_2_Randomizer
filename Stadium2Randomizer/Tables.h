#pragma once

#include "Constants.h"
#include "ItemTable.h"
#include "MoveTable.h"
#include "PokemonTable.h"

namespace GameInfo {

	inline void InitTables() {
		InitMoveTable();
		InitPokemonTable();
	}

}