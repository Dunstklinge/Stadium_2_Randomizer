#pragma once

#include <stdint.h>
#include <string_view>
#include <vector>

#include "Constants.h"

namespace GameInfo {

	struct Move {
		uint8_t effectId;	//anything thats not dmg, e.g if multi-strike, chance to paralyze, lower accuracy, kappapride etc
		uint8_t basePower;
		uint8_t type;
		uint8_t accuracy; //out of 255
		uint8_t pp;
		uint8_t effectChance; //out of 255
	};

	//moves are defined from 0x98430 to 0x98a14
	uint8_t constexpr MoveRawData[] = { 
		#include "MoveRawData.txt" 
	};
	
	extern MoveId MoveIdList[250];

	//note: move ids are 1 based, this array is 0 based
	extern Move Moves[256];

	extern const MoveId TmsGlc[57];
	extern const MoveId TmsRby[55];


	struct MoveEffectInfo {
		std::string_view name;
		enum {
			NONE = 0,
			STATUS_EFFECT = 1,
			ATTACK_EFFECT = 2,				 //if set, this effect is used by damaging moves
			ATTACK_USING_BP = 4 | ATTACK_EFFECT, //things that use bp, like life drain and multi hits
			SPECIAL_ATTACK = 8 | ATTACK_EFFECT, //attack with 1bp, generates its own bp
			CHANCE_PARAMETER = 16,
			NOT_TARGETED = 32,			//usually selfboost moves
		};
		int flags;
	};
	extern const MoveEffectInfo MoveEffectInfos[157];
	

	void InitMoveTable();
}