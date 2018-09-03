#pragma once

#include <stdint.h>

#include "Util.h"

/*
 * general start of all data parts of the rom
 */
class DefGenTable {
public:
	DefGenTable() = delete;
	DefGenTable(const DefGenTable& rhs) = delete;

	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t unknown3;
	uint32_t nTables;

	struct SubtableInfo {
		uint32_t tableOffset;
		uint32_t tableSize;	//including alignment padding but excluding unused space at the end
		uint32_t unknown2;
		uint32_t unknown3;
	};

	SubtableInfo subtableInfos[1]; //nTables times

	/*
	 * This iterator can iterate through a certain subsection of tables while pretending to have a different index
	 * than the actual table. E.g the roster table has a RentalIterator for the first 5 tables, which are pokemon lists,
	 * and a TrainerIterator for the rest of the tables, which are trainer lists. both of these iterators think they start at table 0
	 * and have their own end iterator for easy foreach loops.
	 * If all tables are of the same type, an operator[] might be easier, though the iterator can still be used for easy foreach loops.
	 */
	template<typename T, unsigned int offset>
	class Iterator {
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef T value_type;
		typedef void difference_type;
		typedef T* pointer;
		typedef T& reference;

		DefGenTable* tables;
		int n;

		inline Iterator& operator+=(int add) { n += add; return *this; }
		inline Iterator& operator-=(int sub) { n -= sub; return *this; }
		inline value_type& operator[](int index) {
			uint32_t tableOffset = tables->subtableInfos[offset + index + n].tableOffset;
			//if (tableOffset >= segSize) throw std::out_of_range(std::string("offset ") + std::to_string(tableOffset) + " went out of segment");
			return *(value_type*)(((uint8_t*)tables) + tableOffset);
		}
		inline value_type& operator*() {
			return this->operator[](0);
		}
		inline value_type* operator->() {
			return &(this->operator[](0));
		}

		inline Iterator operator+(const Iterator& rhs) { Iterator ret(*this); ret.n += rhs.n; return ret; }
		inline Iterator operator-(const Iterator& rhs) { Iterator ret(*this); ret.n -= rhs.n; return ret; }
		inline Iterator operator+(int rhs) { Iterator ret(*this); ret.n += rhs; return ret; }
		inline Iterator operator-(int rhs) { Iterator ret(*this); ret.n += rhs; return ret; }
		friend inline Iterator operator+(int lhs, const Iterator& rhs) { Iterator ret(rhs); ret.n += lhs; return ret; }
		friend inline Iterator operator-(int lhs, const Iterator& rhs) { Iterator ret(rhs); ret.n += lhs; return ret; }
		inline Iterator& operator++() { ++n; return *this; }
		inline Iterator& operator--() { --n; return *this; }
		inline Iterator operator++(int) { return (*this) + 1; }
		inline Iterator operator--(int) { return (*this) - 1; }

		inline bool operator==(const Iterator& rhs) { return n == rhs.n && tables == rhs.tables; }
		inline bool operator!=(const Iterator& rhs) { return n != rhs.n || tables != rhs.tables; }
		inline bool operator>(const Iterator& rhs) { return n > rhs.n; }
		inline bool operator<(const Iterator& rhs) { return n < rhs.n; }
		inline bool operator>=(const Iterator& rhs) { return n >= rhs.n; }
		inline bool operator<=(const Iterator& rhs) { return n <= rhs.n; }
	};



	inline void Curate(bool forth) {
		if (!forth) {
			for (unsigned int i = 0; i < nTables; i++) {
				SwitchEndianness(subtableInfos[i].tableOffset);
				SwitchEndianness(subtableInfos[i].tableSize);
			}
		}
		SwitchEndianness(nTables);
		if (forth) {
			for (unsigned int i = 0; i < nTables; i++) {
				SwitchEndianness(subtableInfos[i].tableOffset);
				SwitchEndianness(subtableInfos[i].tableSize);
			}
		}
	}
};