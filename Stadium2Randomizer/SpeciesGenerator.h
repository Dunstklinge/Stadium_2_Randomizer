#pragma once

#include "PokemonTable.h"
#include "DiscreteDistribution.h"

class SpeciesGenerator
{
public:
	SpeciesGenerator();
	~SpeciesGenerator();

	GameInfo::Pokemon Generate(int id, const GameInfo::Pokemon& from);

	bool randomType;
	int gainSecondTypeChance;
	int looseSecondTypeChance;

	bool randomStats;
	DiscreteDistribution bstDist;
	DiscreteDistribution statDist;
	bool dontDecreaseEvolutionBST;
	bool dontDecreaseEvolutionStats;
	bool keepBST;
	bool stayCloseBST;
	bool randomBST;

	const GameInfo::Pokemon* context;

private:
	uint8_t GetRandomType();
	uint8_t GetRandomType(uint8_t exclude);
};

