#pragma once

#include "DefGenTable.h"
#include "Constants.h"

class DefText
{
public:
	DefText() = delete;
	DefText(const DefText& rhs) = delete;

	DefGenTable tables;

	uint8_t textBuffer[1];

	struct StringList {
		uint32_t nStrings;
		uint32_t stringOffsets[1]; //nStrings times
		char strings[0]; //nStrings times, apparently without padding
		char* operator[](int n) { return ((char*)this) + stringOffsets[n]; }
	};


	inline void Curate(bool forth) {
		if (!forth) {
			for (auto it = begin(); it < end(); ++it) {
				auto& strList = *it;
				for (unsigned int i = 0; i < strList.nStrings; i++) {
					SwitchEndianness(strList.stringOffsets[i]);
				}
				SwitchEndianness(strList.nStrings);
			}
			tables.Curate(forth);
		}
		if (forth) {
			tables.Curate(forth);
			for (auto it = begin(); it < end(); ++it) {
				SwitchEndianness(it->nStrings);
				for (unsigned int i = 0; i < it->nStrings; i++) {
					SwitchEndianness(it->stringOffsets[i]);
				}
			}
		}
	}

	typedef DefGenTable::Iterator<StringList, 0> TextTableIterator;
	typedef DefGenTable::Iterator<StringList, TableInfo::TEXT_TRAINER_TEXT_FIRST> TrainerTextTableIterator;

	inline TextTableIterator begin() {
		TextTableIterator it;
		it.n = 0;
		it.tables = &tables;
		return it;
	}

	inline TextTableIterator end() {
		TextTableIterator it;
		it.n = tables.nTables;
		it.tables = &tables;
		return it;
	}

	inline TrainerTextTableIterator trainerTextBegin() {
		TrainerTextTableIterator it;
		it.n = 0;
		it.tables = &tables;
		return it;
	}

	inline TrainerTextTableIterator trainerTextEnd() {
		TrainerTextTableIterator it;
		it.n = tables.nTables - TableInfo::TEXT_TRAINER_TEXT_FIRST;
		it.tables = &tables;
		return it;
	}

	static constexpr unsigned long offStart = 0x1d70000;
	static constexpr unsigned long offEnd = 0x1e40000;
	static constexpr unsigned long segSize = offEnd - offStart;


	static DefText* FromFile(const char* file);
	static DefText* FromFileStream(std::istream& in);
};

