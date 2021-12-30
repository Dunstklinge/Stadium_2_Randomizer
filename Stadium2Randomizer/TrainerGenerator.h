#pragma once

#include "GameContext.h"
#include "DefTrainer.h"
#include "TextMods.h"
#include "PokemonGenerator.h"

class TrainerGenerator
{
public:
	TrainerGenerator();
	TrainerGenerator(GameContext context);
	~TrainerGenerator();

	DefTrainer Generate(const DefTrainer& from);

	bool changeName;
	bool changePokemonNicknames;

	bool changePokes;
	int minLevel;
	int maxLevel;
	int levelSum;

	//deprecated; currently filters are set directly
	enum MoveFilter {
		NONE = 0,
		MIN_ONE_ATTACK = 1,
		MIN_ONE_STRONG_ATTACK = 3,
		MIN_ONE_STAB = 5,
		MIN_ONE_STRONG_STAB = 7
	} minOneMove [[deprecated]];

	bool usefulItem;

	PokemonGenerator gen;

	TextMods textChanges;
private:
	GameContext context;
	

};

