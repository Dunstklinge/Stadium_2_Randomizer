#pragma once

#include <stdint.h>
#include <memory>

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


class CustomFaceInfo
{
public:
	uint32_t faceOffset;			//relative to start of original table
	uint32_t faceLength;			//table length

	inline void Curate(bool forth) {
		SwitchEndianness(faceOffset);
		SwitchEndianness(faceLength);
	}
};


//table of CustomStringInfo tables
class CustomStringTableInfo 
{
public:
	uint16_t tableId;
	uint16_t tableOffset;
	uint32_t tableSize;

	inline void Curate(bool forth) {
		SwitchEndianness(tableId);
		SwitchEndianness(tableOffset);
		SwitchEndianness(tableSize);
	}
};

static_assert(sizeof(CustomStringTableInfo) == 8, "size mismatch");

class CustomStringInfo
{
public:
	uint8_t start;
	uint8_t end;
	uint16_t padding;

	//uint32_t offsets[end-start]
	//string strings[end-start] 

	//warning: only call on finalized version
	inline void Curate(bool forth) {
		uint32_t* arr = (uint32_t*)(((uint8_t*)this) + 4);
		for (int i = start; i < end; i++) {
			SwitchEndianness(arr[i - start]);
		}
	}
};


static_assert(sizeof(CustomStringInfo) == 4, "size mismatch");

