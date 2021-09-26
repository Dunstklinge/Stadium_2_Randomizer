#include "stdafx.h"
#include "SpeciesGenerator.h"
#include "PokemonTable.h"
#include "GlobalRandom.h"

SpeciesGenerator::SpeciesGenerator()
{
	randomType = true;
	gainSecondTypeChance = 0;
	looseSecondTypeChance = 0;

	randomStats = false;
	dontDecreaseEvolutionBST = true;
	dontDecreaseEvolutionStats = true;
	keepBST = true;
	stayCloseBST = false;
	randomBST = false;
	context = nullptr;
}


SpeciesGenerator::~SpeciesGenerator()
{
}

GameInfo::Pokemon SpeciesGenerator::Generate(int id, const GameInfo::Pokemon& from)
{
	using namespace GameInfo;
	Pokemon ret = from;
	if (randomType) {
		bool dualType = ret.type1 != ret.type2;
		ret.type1 = GetRandomType();
		std::uniform_int_distribution<int> percentDist(0, 99);
		int rand = percentDist(Random::Generator);
		if (!dualType && rand < gainSecondTypeChance || dualType && rand >= looseSecondTypeChance) {
			ret.type2 = GetRandomType(ret.type1);
		}
		else if (dualType && rand < looseSecondTypeChance) {
			ret.type2 = ret.type1;
		}
	}
	const Pokemon* preEvo = nullptr;
	if (dontDecreaseEvolutionBST || dontDecreaseEvolutionStats && PokemonPrevEvo[id] != GameInfo::NO_POKEMON) {
		preEvo = &context[PokemonPrevEvo[id] - 1];
	}
	if (randomStats) {
		int bst = ret.CalcBST();
		if (keepBST) {
			//keep it, nothing to do
		}
		else if (stayCloseBST) {
			int maxDelta = bst * 0.1f;
			int min = (dontDecreaseEvolutionBST && preEvo) ? bst : bst - maxDelta;
			bst = bstDist(Random::Generator, min, bst + maxDelta);
		}
		else if (randomBST) {
			if (dontDecreaseEvolutionBST && preEvo) {
				bst = bstDist(Random::Generator, preEvo->CalcBST(), bstDist.Max());
			}
			else {
				bst = bstDist(Random::Generator);
			}
		}
		uint8_t Pokemon::*stats[] = { &Pokemon::baseHp, &Pokemon::baseAtk, &Pokemon::baseDef, &Pokemon::baseSpA, &Pokemon::baseSpD, &Pokemon::baseSpd };
		int weights[6] = {};
		int weightSum = 0;
		statDist.SetMinMax(0, 1000);
		for (int& weight : weights) {
			weight = statDist(Random::Generator);
			weightSum += weight;
		}
		int bstLeft = bst;
		if (dontDecreaseEvolutionStats && preEvo) {
			bstLeft = bst - preEvo->CalcBST();
		}
		for (int i = 0; i < _countof(stats); i++) {
			int stat = weights[i] * bstLeft / weightSum;
			if (stat > 255) stat = 255;
			ret.*stats[i] = stat;
			if (dontDecreaseEvolutionStats && preEvo) {
				ret.*stats[i] += preEvo->*stats[i];
			}
			bstLeft -= stat;
		}
	}

	return ret;
}

uint8_t SpeciesGenerator::GetRandomType()
{
	uint8_t rand = Random::GetInt(0, 16);
	if (rand > GameInfo::ROCK) rand += 1;
	if (rand > GameInfo::STEEL) rand += 0x0A;
	return rand;
}

uint8_t SpeciesGenerator::GetRandomType(uint8_t exclude)
{
	uint8_t rand = Random::GetInt(0, 15);
	if (rand >= exclude) rand++;
	if (rand > GameInfo::ROCK) rand += 1;
	if (rand > GameInfo::STEEL) rand += 0x0A;
	return rand;
}
