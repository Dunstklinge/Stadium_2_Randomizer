#include "PokemonGenerator.h"

#include "Constants.h"
#include "Tables.h"
#include "GlobalRandom.h"
#include "GeneratorUtil.h"


PokemonGenerator::PokemonGenerator()
{
	changeSpecies = true;
	changeEvsIvs = true;
	changeLevel = true;
	changeMoves = true;
	changeItem = true;

	speciesFilter = nullptr;
	speciesFilterBuffer = nullptr;

	minLevel = 100;
	maxLevel = 100;
	levelUniformDistribution = true;

	itemFilter = nullptr;
	itemFilterBuffer = nullptr;
	includeTypeSpeciesSpecific = false;

	minOneMoveFilter = nullptr;
	minOneMoveFilterBuffer = nullptr;

	generalMoveFilter = nullptr;
	generalMoveFilterBuffer = nullptr;

	randEvs = true;
	randIvs = true;
	bstEvIvs = true;
	statsUniformDistribution = false;
}


PokemonGenerator::~PokemonGenerator()
{
}

DefPokemon PokemonGenerator::Generate() 
{
	DefPokemon mon;
	
	GenSpecies(mon);
	GenLevel(mon);
	GenEvsIvs(mon);
	GenMoves(mon);
	GenItem(mon);

	return mon;
}

DefPokemon PokemonGenerator::Generate(GameInfo::PokemonId species)
{
	DefPokemon mon;

	mon.species = species;
	GenLevel(mon);
	GenEvsIvs(mon);
	GenMoves(mon);
	GenItem(mon);

	return mon;
}

DefPokemon PokemonGenerator::Generate(const DefPokemon & from)
{
	DefPokemon mon = from;

	if(changeSpecies)
		GenSpecies(mon);
	if(changeLevel)
		GenLevel(mon);
	if(changeEvsIvs)
		GenEvsIvs(mon);
	if(changeMoves)
		GenMoves(mon);
	if (changeItem)
		GenItem(mon);

	return mon;
}

void PokemonGenerator::GenSpecies(DefPokemon & mon)
{
	const GameInfo::PokemonId* monList = nullptr;
	unsigned int monListN;
	if (speciesFilterBuffer) {
		monList = speciesFilterBuffer;
		monListN = speciesFilterBufferN;
	}
	else {
		monList = GameInfo::PokemonIds;
		monListN = _countof(GameInfo::PokemonIds);
	}

	if (speciesFilter) {
		unsigned int legalMonsN = 0;
		GameInfo::PokemonId legalMons[256];
		for (int i = 0; i < monListN; i++) {
			if (speciesFilter(monList[i])) {
				legalMons[legalMonsN++] = monList[i];
			}
		}
		if (legalMonsN > 0) {
			std::uniform_int_distribution<int> dist(0, (int)legalMonsN-1);
			mon.species = legalMons[dist(Random::Generator)];
			return;
		}
	}
	{
		std::uniform_int_distribution<int> dist(0, monListN-1);
		mon.species = monList[dist(Random::Generator)];
		return;
	}
	
}

void PokemonGenerator::GenLevel(DefPokemon& mon)
{
	if (minLevel == maxLevel) {
		mon.level = minLevel;
		return;
	}
	if (levelUniformDistribution) {
		std::uniform_int_distribution<int> dist(minLevel, maxLevel);
		mon.level = dist(Random::Generator);
	}
	else {
		std::binomial_distribution dist(maxLevel - minLevel, 0.5);
		mon.level = dist(Random::Generator) + minLevel;
	}

}

void PokemonGenerator::GenEvsIvs(DefPokemon & mon)
{
	if (bstEvIvs) {
		SatUAr bst = GameInfo::Pokemons[mon.species].CalcBST();
		//200 bst should always be FFFF, 680bst should always be 0. note: 136.53125 * 480 = 0xFFFF
		SatUAr<unsigned int> max = (680u - (bst - 200u + 200u)) * 136.53125;
		//200 and 680 should have 0 derivation, while towards the middle (440) derivation should be high
		constexpr double maxDerivation = 0.6;
		unsigned int derivation = (240 - std::abs(240 - (int)(bst - 200u))) * (maxDerivation * 0xFFFF / 240.0);
		unsigned int minEv = max - derivation;
		unsigned int maxEv = max;
		if (statsUniformDistribution || minEv == maxEv) {
			std::uniform_int_distribution<int> dist(minEv, maxEv);
			mon.evHp = dist(Random::Generator);
			mon.evAtk = dist(Random::Generator);
			mon.evDef = dist(Random::Generator);
			mon.evSpd = dist(Random::Generator);
			mon.evSpc = dist(Random::Generator);

		}
		else {
			double percent = 0xFFFF / 2 / 9000;
			std::normal_distribution<double> gdist((maxEv - minEv) / 2, (maxEv - minEv) / 2 * percent);

			mon.evHp = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);
			mon.evAtk = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);
			mon.evDef = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);
			mon.evSpd = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);
			mon.evSpc = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);

		}
		SatUAr<unsigned int> maxIv = (680u - bst) * 0.03125;
		derivation = (240 - std::abs(240 - (int)(bst - 200u))) * (maxDerivation * 15 / 240.0);
		unsigned int minIv = maxIv - derivation;
		if (statsUniformDistribution || minIv == maxIv) {
			std::uniform_int_distribution<int> dist(minIv, maxIv);
			int dvs[4];
			for (int i = 0; i < 4; i++) dvs[i] = dist(Random::Generator);
			mon.dvs = dvs[0] | (dvs[1] << 4) | (dvs[2] << 8) | (dvs[3] << 12);
		}
		else {
			double percent = 0xFFFF / 2 / 9000;
			std::normal_distribution<double> dvdist((maxEv - minEv) / 2, (maxEv - minEv) / 2 * percent);
			int dvs[4];
			for (int i = 0; i < 4; i++) dvs[i] = std::clamp((int)std::round(dvdist(Random::Generator)), 0, 15);
			mon.dvs = dvs[0] | (dvs[1] << 4) | (dvs[2] << 8) | (dvs[3] << 12);
		}
	}
	else {
		if (statsUniformDistribution) {
			std::uniform_int_distribution<int> dist(0, 0xFFFF);
			mon.evHp = dist(Random::Generator);
			mon.evAtk = dist(Random::Generator);
			mon.evDef = dist(Random::Generator);
			mon.evSpd = dist(Random::Generator);
			mon.evSpc = dist(Random::Generator);

		}
		else {
			double percent = 0xFFFF / 2 / 9000;
			std::normal_distribution<double> gdist(0x7FFF, 9000);

			mon.evHp = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);
			mon.evAtk = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);
			mon.evDef = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);
			mon.evSpd = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);
			mon.evSpc = std::clamp((int)std::round(gdist(Random::Generator)), 0, 0xFFFF);

		}
		if (statsUniformDistribution) {
			std::uniform_int_distribution<int> dist(0, 0xFFFF);
			mon.dvs = dist(Random::Generator);
		}
		else {
			std::binomial_distribution dvdist(15, 0.5);
			int dvs[4];
			for (int i = 0; i < 4; i++) dvs[i] = dvdist(Random::Generator);
			mon.dvs = dvs[0] | (dvs[1] << 4) | (dvs[2] << 8) | (dvs[3] << 12);
		}
	}
	
}

void PokemonGenerator::GenMoves(DefPokemon & mon)
{
	static GameInfo::MoveId genMoveList[256];
	static unsigned int genMoveListN;
	genMoveListN = 0;

	if (generalMoveFilter) {
		for (int i = 1; i <= GameInfo::LAST_MOVE; i++) {
			if (generalMoveFilter((GameInfo::MoveId)i, mon.species)) {
				genMoveList[genMoveListN++] = (GameInfo::MoveId)i;
			}
		}
	}
	mon.move1 = mon.move2 = mon.move3 = mon.move4 = (GameInfo::MoveId)0;

	const GameInfo::MoveId* baseMoveList = nullptr;
	unsigned int baseMoveListN;
	if (generalMoveFilter) {
		baseMoveList = genMoveList;
		baseMoveListN = genMoveListN;
	}
	else if (generalMoveFilterBuffer) {
		baseMoveList = generalMoveFilterBuffer;
		baseMoveListN = generalMoveFilterBufferN;
	}
	else {
		baseMoveList = GameInfo::MoveIdList;
		baseMoveListN = _countof(GameInfo::MoveIdList);
	}

	std::uniform_int_distribution<int> dist(0, baseMoveListN-1);
	//generate first move specially if requested
	if (minOneMoveFilter) {
		static unsigned int legalMovesN;
		static GameInfo::MoveId legalMoves[256];
		legalMovesN = 0;
		for (unsigned int i = 0; i <= baseMoveListN; i++) {
			GameInfo::MoveId move = baseMoveList[i];
			if (minOneMoveFilter(move, mon.species)) {
				legalMoves[legalMovesN++] = move;
			}
		}
		if (legalMovesN > 0) {
			std::uniform_int_distribution<int> dist(0, (int)legalMovesN-1);
			mon.move1 = legalMoves[dist(Random::Generator)];
		}
		
	}
	else if (minOneMoveFilterBuffer && minOneMoveFilterBufferN > 0) {
		std::uniform_int_distribution<int> dist(0, (int)minOneMoveFilterBufferN - 1);
		mon.move1 = minOneMoveFilterBuffer[dist(Random::Generator)];
	}
	if (mon.move1 == 0) {
		mon.move1 = baseMoveList[dist(Random::Generator)];
	}

	//prevent having the same move twice. note that this only works if every entry in basemovelist is unique
	if (dist.b() == 0) return;
	dist = std::uniform_int_distribution<int>(0, dist.b() - 1);
	int iMove2 = dist(Random::Generator);
	while(baseMoveList[iMove2] == mon.move1) iMove2++;
	mon.move2 = baseMoveList[iMove2];
	
	if (dist.b() == 0) return;
	dist = std::uniform_int_distribution<int>(0, dist.b() - 1);
	int iMove3 = dist(Random::Generator);
	while (baseMoveList[iMove3] == mon.move1 || baseMoveList[iMove3] == mon.move2) iMove3++;
	mon.move3 = baseMoveList[iMove3];

	if (dist.b() == 0) return;
	dist = std::uniform_int_distribution<int>(0, dist.b() - 1);
	int iMove4 = dist(Random::Generator);
	while (baseMoveList[iMove4] == mon.move1 || baseMoveList[iMove4] == mon.move2
		|| baseMoveList[iMove4] == mon.move3) iMove4++;
	mon.move4 = baseMoveList[iMove4];
}

void PokemonGenerator::GenItem(DefPokemon & mon)
{

	union {
		struct {
			GameInfo::ItemId speciesItem;
			GameInfo::ItemId typeItem1;
			GameInfo::ItemId typeItem2;
		};
		GameInfo::ItemId items[3];
	} extraItems;
	extraItems.speciesItem = GameInfo::SpeciesBattleItemMap[mon.species];
	extraItems.typeItem1 = GameInfo::TypeBattleItemMap[GameInfo::Pokemons[mon.species].type1];
	extraItems.typeItem2 = GameInfo::TypeBattleItemMap[GameInfo::Pokemons[mon.species].type2];
	
	int includeItemsIgnore = 0; //bitfield, bit 1 ,2 ,3 for species, type1, type2
	if (extraItems.speciesItem == GameInfo::NO_ITEM) includeItemsIgnore |= 1;
	if (extraItems.typeItem1 == GameInfo::NO_ITEM) includeItemsIgnore |= 2;
	if (extraItems.typeItem2 == GameInfo::NO_ITEM) includeItemsIgnore |= 4;
	if (extraItems.typeItem1 == extraItems.typeItem2) includeItemsIgnore |= 4; //in case of mono type pokemon

	unsigned int legalItemsN = 0;
	static GameInfo::ItemId legalItems[256];
	const GameInfo::ItemId* itemList;
	unsigned int itemListN;

	FilterBufferGenerateOrDefault(&itemList, &itemListN,
						legalItems, legalItemsN,
						GameInfo::ExistingItemMap, _countof(GameInfo::ExistingItemMap),
						itemFilterBuffer, itemFilterBufferN,
						&[&](GameInfo::ItemId iId) {
		if (extraItems.speciesItem == iId) includeItemsIgnore |= 1;
		if (extraItems.typeItem1 == iId) includeItemsIgnore |= 2;
		if (extraItems.typeItem2 == iId) includeItemsIgnore |= 4;
		if (itemFilter) return itemFilter(iId, mon.species); 
		return false;
	});


	if (includeTypeSpeciesSpecific) {
		//try drawing type or species specific item
		unsigned int addN = 3 - 
			((includeItemsIgnore & 1) + (includeItemsIgnore & 2) >> 1 + (includeItemsIgnore & 4) >> 2);
		if (addN > 0) {
			std::uniform_int_distribution<int> dist(0, (int)itemListN - 1 + addN);
			int rand = dist(Random::Generator);
			if (rand > (int)itemListN - 1) {
				unsigned int useAddN = rand - (int)itemListN - 1;
				for (int i = 0; i < 3; i++) {
					if (((1 << i) & includeItemsIgnore) && useAddN-- == 0) {
						mon.item = extraItems.items[i];
						return;
					}
				}
			}
		}
		
	}
	
	std::uniform_int_distribution<int> dist(0, (int)itemListN - 1);
	mon.item = itemList[dist(Random::Generator)];
	return;
	

	/*
	if (itemFilter) {
		for (int i = 0; i <= _countof(GameInfo::ExistingItemMap); i++) {
			if (itemFilter(GameInfo::ExistingItemMap[i], mon.species)) {
				legalItems[legalItemsN++] = (GameInfo::ItemId)i;
			}
		}
		if (legalItemsN > 0) {
			std::uniform_int_distribution<int> dist(0, (int)legalItemsN-1);
			mon.item = (GameInfo::ItemId)dist(Random::Generator);
			return;
		}
		
	}
	if (itemFilterBuffer) {
		std::uniform_int_distribution<int> dist(0, (int)itemFilterBufferN-1);
		mon.item = itemFilterBuffer[dist(Random::Generator)];
		return;
	}
	{
		std::uniform_int_distribution<int> dist(0, (int)_countof(GameInfo::ExistingItemMap)-1);
		mon.item = GameInfo::ExistingItemMap[dist(Random::Generator)];
		return;
	}*/
}
