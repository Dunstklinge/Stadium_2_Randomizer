#include "TrainerGenerator.h"

#include "GlobalRandom.h"
#include "GlobalConfigs.h"
#include "Filters.h"
#include "GeneratorUtil.h"

TrainerGenerator::TrainerGenerator()
{
	gen.changeSpecies = true;
	gen.changeEvsIvs = true;
	gen.changeLevel = false;
	gen.changeMoves = true;
	gen.changeItem = true;

	gen.minLevel = 100;
	gen.maxLevel = 100;
	gen.levelUniformDistribution = true;
	gen.randEvs = true;
	gen.randIvs = true;
	gen.bstEvIvs = false;
	gen.statsUniformDistribution = false;

	usefulItem = false;
	stayCloseToBST = true;
	stayCloseToBSTThreshold = 50;
	//minOneMove = NONE;
}


TrainerGenerator::~TrainerGenerator()
{
}

DefTrainer TrainerGenerator::Generate(const DefTrainer & from)
{
	DefTrainer ret = from;

	gen.minLevel = minLevel;
	gen.maxLevel = maxLevel;

	//set up pokemon gen
	if (usefulItem) {
		gen.itemFilter = nullptr;
		gen.itemFilterBuffer = GameInfo::GeneralBattleItemMap;
		gen.itemFilterBufferN = _countof(GameInfo::GeneralBattleItemMap);
		gen.includeTypeSpeciesSpecific = true;
	}
	else {
		gen.itemFilter = nullptr;
		gen.itemFilterBuffer = nullptr;
		gen.includeTypeSpeciesSpecific = false;
	}


	//give trainer a random name
	if (changeName) {
		auto tIt = GlobalConfig::TrainerNicks.trainerNicknameMap.find((GameInfo::TrainerNames)from.trainerId);
		if (tIt != GlobalConfig::TrainerNicks.trainerNicknameMap.end() && tIt->second.size() > 0) {
			std::uniform_int_distribution<unsigned> dist(0, tIt->second.size() - 1);
			unsigned int rand = dist(Random::Generator);
			std::string& name = tIt->second[rand];
			textChanges.AddChange(TableInfo::TEXT_TRAINER_NAMES, from.trainerId - 1, name);

		}
		else {
			auto cIt = GlobalConfig::TrainerNicks.catNicknameMap.find((GameInfo::TrainerCat)from.trainerCat);
			if (cIt != GlobalConfig::TrainerNicks.catNicknameMap.end() && cIt->second.size() > 0) {
				std::uniform_int_distribution<unsigned> dist(0, cIt->second.size() - 1);
				unsigned int rand = dist(Random::Generator);
				std::string& name = cIt->second[rand];
				textChanges.AddChange(TableInfo::TEXT_TRAINER_NAMES, from.trainerId - 1, name);
			}
		}
	}
	
	//generate pokemon.
	
	if (changePokes) {
		for (int i = 0; i < ret.nPokes; i++) {
			DefPokemon newPoke;

			//to prevent doubles, we generate a viable pokemon array manually from both their buffer and our filter
			//and ignore previous pokemon
			SatUAr oldBst = GameInfo::Pokemons[ret.pokemon[i].species].CalcBST();
			unsigned int minBst = oldBst - stayCloseToBSTThreshold;
			unsigned int maxBst = oldBst + stayCloseToBSTThreshold;
			GameInfo::PokemonId validSpecies[256];
			unsigned int validSpeciesN = 0;


			const GameInfo::PokemonId* filterList;
			unsigned int filterListN;
			if (gen.speciesFilterBuffer) {
				filterList = gen.speciesFilterBuffer;
				filterListN = gen.speciesFilterBufferN;
			}
			else {
				filterList = GameInfo::PokemonIds;
				filterListN = _countof(GameInfo::PokemonIds);
			}

			for (unsigned int j = 0; j < filterListN; j++) {
				if (!stayCloseToBST || FilterPokemonByBST(filterList[j], minBst, maxBst)) {
					bool used = false;
					for (int k = 0; k < i; k++) used |= ret.pokemon[k].species == filterList[j];
					if (!used) {
						validSpecies[validSpeciesN++] = filterList[j];
					}
				}
			}

			auto* oldBuffer = gen.speciesFilterBuffer;
			auto oldBufferN = gen.speciesFilterBufferN;

			if (validSpeciesN > 0) {
				gen.speciesFilterBuffer = validSpecies;
				gen.speciesFilterBufferN = validSpeciesN;
			}

			auto oldOneMoveFilter = gen.minOneMoveFilter;


			newPoke = gen.Generate(ret.pokemon[i]);
			ret.pokemon[i] = newPoke;

			gen.minOneMoveFilter = oldOneMoveFilter;

			gen.speciesFilterBuffer = oldBuffer;
			gen.speciesFilterBufferN = oldBufferN;



			//give pokemon a random name
			if (changePokemonNicknames) {
				auto pIt = GlobalConfig::PokemonNicks.nicknameMap.find(newPoke.species);
				if (pIt != GlobalConfig::PokemonNicks.nicknameMap.end() && pIt->second.size() > 0) {
					std::uniform_int_distribution<unsigned> dist(0, pIt->second.size() - 1);
					unsigned int rand = dist(Random::Generator);
					//have a chance to get the original name
					if (rand == pIt->second.size()) {
						textChanges.AddChange(TableInfo::TEXT_TRAINER_TEXT_FIRST + from.textId, TableInfo::TRAINERTEXT_NICKNAME1 + i, "");
					}
					else {
						std::string& name = pIt->second[rand];
						textChanges.AddChange(TableInfo::TEXT_TRAINER_TEXT_FIRST + from.textId, TableInfo::TRAINERTEXT_NICKNAME1 + i, name);
					}

				}
				else {
					textChanges.AddChange(TableInfo::TEXT_TRAINER_TEXT_FIRST + from.textId, TableInfo::TRAINERTEXT_NICKNAME1 + i, "");
				}
			}
			else if (gen.changeSpecies) {
				//set name to default because nicknames would not match new species
				textChanges.AddChange(TableInfo::TEXT_TRAINER_TEXT_FIRST + from.textId, TableInfo::TRAINERTEXT_NICKNAME1 + i, "");
			}

		}

		//adjust levels
		if (gen.changeLevel) {
			//blindly assuming to have >= 3 mons

			//first, sort pokemon by level
			struct tmp {
				int level, index;
				bool operator <(const tmp& rhs) { return level < rhs.level; }
			};
			tmp levels[6];
			for (int i = 0; i < ret.nPokes; i++) {
				levels[i].level = ret.pokemon[i].level;
				levels[i].index = i;
			}
			std::sort(levels, levels + ret.nPokes);

			//next, if the highest pokemon can not be used, adjust lowest pokemon to make it usable
			int test = levels[0].level + levels[1].level + levels[ret.nPokes - 1].level;
			if (test > levelSum) {
				//cant even use highest level pokemon; adjust lowest level mons to make it usable
				int allowedTotal = levelSum - levels[ret.nPokes - 1].level;
				int level1, level2;
				level1 = level2 = allowedTotal / 2;
				if (allowedTotal & 1) level2++;
				ret.pokemon[levels[0].index].level = level1;
				ret.pokemon[levels[1].index].level = level2;
			}
		}
	}
	

	return ret;
}
