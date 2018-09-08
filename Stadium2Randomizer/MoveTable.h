#pragma once

#include <stdint.h>

#include "Constants.h"

namespace GameInfo {

	struct Move {
		uint8_t effectId;	//anything thats not dmg, e.g if multi-strike, chance to paralyze, lower accuracy, kappapride etc
		uint8_t basePower;
		uint8_t type;
		uint8_t accuracy; //out of 255
		uint8_t pp;
		uint8_t efffectChance; //out of 255
	};

	//moves are defined from 0x98430 to 0x98a14
	uint8_t constexpr MoveRawData[] = { 
		#include "MoveRawData.txt" 
	};
	
	extern MoveId MoveIdList[250];

	extern Move Moves[256];

	void InitMoveTable();
}