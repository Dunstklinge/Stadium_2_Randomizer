#pragma once

#include <stdint.h>
#include <vector>
#include <string>

#include "DefGenTable.h"
#include "DefTrainer.h"

/*
 * Represents all roster tables within the rom so that a buffer pointer can be cast.
 * Also offers iterators to iterate over the rental and trainer data.
 */
class DefRoster
{
public:
	DefRoster() = delete;
	DefRoster(const DefRoster&) = delete;

	DefGenTable tables;

	//all the rest
	uint8_t rosterBuffer[1];

	//PokemonList[5]
	//TrainerList[0]

	///////////////////

	//this is actually:
	//for the first 5 tables
	struct PokemonList {
		uint8_t nPokemon;
		uint8_t unknown1;
		uint8_t unknown2;
		uint8_t unknown3;
	
		DefPokemon pokemon[1];
	};

	struct TrainerList {
		uint8_t nTrainers;
		uint8_t unknown1;
		uint8_t unknown2;
		uint8_t unknown3;
	
		DefTrainer trainers[1];

	public:
		//I used to think that DefTrainer has as many pokemon as needed and thus varying sizes.
		//turns out it always has 6 pokemon and unused mons are filled with bulbasaurs.
		//thus, this forward iterator is not needed as the array can now be used regularily
		/*class Iterator {
		public:
			typedef std::forward_iterator_tag iterator_category;
			typedef DefTrainer value_type;
			typedef void difference_type;
			typedef DefTrainer* pointer;
			typedef DefTrainer& reference;

			TrainerList* trainerList;
			int n;
			int offset;

			inline value_type& operator*() { return *(value_type*)(((uint8_t*)trainerList) + offset); }
			inline value_type* operator->() { return (value_type*)(((uint8_t*)trainerList) + offset); }

			inline Iterator& operator++() { offset += sizeof(DefTrainer); n++; return *this; }
			inline Iterator operator++(int) { auto temp = *this; this->operator++(); return temp; }

			inline bool operator==(const Iterator& rhs) { return n == rhs.n && trainerList == rhs.trainerList; }
			inline bool operator!=(const Iterator& rhs) { return n != rhs.n || trainerList != rhs.trainerList; }
			inline bool operator>(const Iterator& rhs) { return n > rhs.n; }
			inline bool operator<(const Iterator& rhs) { return n < rhs.n; }
			inline bool operator>=(const Iterator& rhs) { return n >= rhs.n; }
			inline bool operator<=(const Iterator& rhs) { return n <= rhs.n; }
		};

		inline Iterator begin() {
			Iterator it;
			it.trainerList = this;
			it.n = 0;
			it.offset = sizeof(TrainerList);
			return it;
		}
		inline Iterator end() {
			Iterator it;
			it.trainerList = this;
			it.n = nTrainers;
			it.offset = -1;
			return it;
		}*/
	};

	inline void Curate(bool forth) {
		if (!forth) {
			for (auto it = rentalBegin(); it < rentalEnd(); ++it) {
				for (int i = 0; i < it->nPokemon; i++) it->pokemon[i].Curate(forth);
			}
			for (auto it = trainerBegin(); it < trainerEnd(); ++it) {
				for (int i = 0; i < it->nTrainers; i++) it->trainers[i].Curate(forth);
			}
			tables.Curate(forth);
		}
		if (forth) {
			tables.Curate(forth);
			for (auto it = rentalBegin(); it < rentalEnd(); ++it) {
				for (int i = 0; i < it->nPokemon; i++) it->pokemon[i].Curate(forth);
			}
			for (auto it = trainerBegin(); it < trainerEnd(); ++it) {
				for (int i = 0; i < it->nTrainers; i++) it->trainers[i].Curate(forth);
			}
		}
	}

public:
	

public:
	typedef DefGenTable::Iterator<PokemonList, 0> RentalIterator;
	typedef DefGenTable::Iterator<TrainerList, 5> TrainerIterator;

	inline RentalIterator rentalBegin() {
		RentalIterator it;
		it.n = 0;
		it.tables = &tables;
		return it;
	}

	inline RentalIterator rentalEnd() {
		RentalIterator it;
		//it.n = tables.nTables < 5 ? tables.nTables : 5;
		it.n = tables.nTables < 4 ? tables.nTables : 4; //skip last table as it points to the same twice
		it.tables = &tables;
		return it;
	}

	inline TrainerIterator trainerBegin() {
		TrainerIterator it;
		it.n = 0;
		it.tables = &tables;
		return it;
	}

	inline TrainerIterator trainerEnd() {
		TrainerIterator it;
		it.n = tables.nTables - 5;
		it.tables = &tables;
		return it;
	}

	static constexpr unsigned long offStart = 0x1708000;
	static constexpr unsigned long offEnd = 0x1718000;
	static constexpr unsigned long segSize = offEnd - offStart;

	static DefRoster* FromFile(const char* file);
	static DefRoster* FromFileStream(std::istream& in);
};

