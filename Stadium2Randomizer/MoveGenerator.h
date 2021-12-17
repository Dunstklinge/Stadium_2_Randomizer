#pragma once

#include <vector>

#include "MoveTable.h"
#include "DiscreteDistribution.h"
#include "TextMods.h"

class MoveGenerator
{
public:
	MoveGenerator();
	~MoveGenerator();

	GameInfo::Move Generate(const GameInfo::Move& from, int moveId);

	bool balanced;
	bool randomType;
	bool randomAcc;
	DiscreteDistribution accDist;
	bool randomPp;
	DiscreteDistribution ppDist;
	bool randomBp;
	bool stayCloseToBp;
	DiscreteDistribution bpDist;
	bool randomSecondary;
	int gainSecondaryEffectChance;
	int looseSecondaryEffectChance;
	bool randomEffectChance;
	DiscreteDistribution ecDist;
	bool randomStatusMoveEffect;
	bool generateDescription;
	bool preciseDescription;

	TextMods textChanges;
private:

	struct AuxMaps {
		AuxMaps();
		std::vector<GameInfo::MoveEffect> secChanceEffects;
		std::vector<GameInfo::MoveEffect> secNonChanceEffects;
		std::vector<GameInfo::MoveEffect> primEffects;
	};
	static AuxMaps& Aux();
};

