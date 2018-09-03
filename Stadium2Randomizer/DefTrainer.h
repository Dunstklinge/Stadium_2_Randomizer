#pragma once

#include <iostream>

#include "DefText.h"
#include "DefPokemon.h"

/*
 * Represents a trainer inside the rom so that a pointer to the file buffer can be cast to this struct
 */
class DefTrainer
{
public:

	uint8_t trainerCat;
	uint8_t trainerId;
	uint8_t unknown1;
	uint8_t textId;
	uint8_t nPokes;
	uint8_t unknown2;
	uint8_t unknown3;
	uint8_t unknown4;

	DefPokemon pokemon[6]; //always 6 in this array; unused mons are filled with bulbasaurs

	inline void Curate(bool forth) {
		for (int i = 0; i < nPokes; i++) pokemon[i].Curate(forth);
	}

	void Print(DefText* texts, std::ostream& o);
};

static_assert(sizeof(DefTrainer) == 8 + sizeof(DefPokemon)*6, "size missmatch");