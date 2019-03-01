#pragma once

#include <stdint.h>

#include "Util.h"

/*
 * Contains additional Information used by injected code to make more
 * different rental and item sets available
 */
class CustomRosterInfo
{
public:
	uint32_t fightMin;			//the first trainer set this rule applies to (total index in roster table, so formatnotes index + 5)
	int32_t rentalOffset;		//rental offset that will be copied into the original gentable part of the roster table. !make sure its relative to that table!
	int32_t rentalLength;		//rental table length that will be copied into the original gentable part of the roster table.

	inline void Curate(bool forth) {
		SwitchEndianness(fightMin);
		SwitchEndianness(rentalOffset);
		SwitchEndianness(rentalLength);
	}
};

static_assert(sizeof(CustomRosterInfo) == 0xC, "missmatching size");

class CustomItemInfo
{
public:
	uint32_t fightMin;
	uint32_t itemPtr;			//pointer into rom of item list

	inline void Curate(bool forth) {
		SwitchEndianness(fightMin);
		SwitchEndianness(itemPtr);
	}
};