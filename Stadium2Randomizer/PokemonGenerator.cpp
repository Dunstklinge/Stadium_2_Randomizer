#include "PokemonGenerator.h"

#include "Constants.h"
#include "Tables.h"
#include "GlobalRandom.h"
#include "GeneratorUtil.h"
#include "MoveEffectValue.h"

PokemonGenerator::PokemonGenerator() : PokemonGenerator(DefaultContext)
{
}

PokemonGenerator::PokemonGenerator(GameContext context) : context(context)
{
	changeSpecies = true;
	changeEvsIvs = true;
	changeLevel = true;
	changeMoves = true;
	changeItem = true;
	changeHappiness = true;

	minLevel = 100;
	maxLevel = 100;
	levelDist = DiscreteDistribution(0, 100);

	moveRandMove = MoveRandMode::EqualChance;

	randEvs = true;
	randIvs = true;
	bstEvIvs = true;
	statsDist = DiscreteDistribution(0, 100);
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
	GenHappiness(mon);
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
	GenHappiness(mon);
	GenMoves(mon);
	if (changeItem)
		GenItem(mon);
	else
		mon.item = GameInfo::NO_ITEM;

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
	if(changeHappiness)
		GenHappiness(mon);
	if(changeMoves)
		GenMoves(mon);
	if (changeItem)
		GenItem(mon);

	return mon;
}

template<typename T, typename It>
void PokemonGenerator::Refresh(GameInfo::PokemonId species, Filter<T>& filter, std::vector<T>& cache, It iterator, unsigned size) {
	if (filter.buffer && filter.bufferN > 0)
	{
		cache.resize(filter.bufferN);
		std::copy(filter.buffer, filter.buffer + filter.bufferN, cache.begin());
	}
	else {
		cache.resize(size);
		std::copy(iterator, iterator + size, cache.begin());
	}
	if (filter.func)
	{
		cache.erase(std::remove_if(cache.begin(), cache.end(),
			[&](T id) { return !filter.func(id, species); }
		));
	}
}

template<typename T, unsigned size>
void PokemonGenerator::Refresh(GameInfo::PokemonId species, Filter<T>& filter, std::vector<T>& cache, const T(&baseBuffer)[size]) {
	Refresh(species, filter, cache, baseBuffer, size);
}

//this one has to be copy pastad because the filter accepts one less parameter, as there is no species context when generating species
void PokemonGenerator::RefreshSpecies() {
	if (speciesFilter.buffer && speciesFilter.bufferN > 0)
	{
		cache.species.resize(speciesFilter.bufferN);
		std::copy(speciesFilter.buffer, speciesFilter.buffer + speciesFilter.bufferN, cache.species.begin());
	}
	else {
		cache.species.resize(_countof(GameInfo::PokemonIds));
		std::copy(GameInfo::PokemonIds, GameInfo::PokemonIds + _countof(GameInfo::PokemonIds), cache.species.begin());
	}
	if (speciesFilter.func)
	{
		cache.species.erase(std::remove_if(cache.species.begin(), cache.species.end(),
			[&](GameInfo::PokemonId id) { return !speciesFilter.func(id); }
		));
	}
}

void PokemonGenerator::RefreshItems(GameInfo::PokemonId species) {
	Refresh(species, itemFilter, cache.items, GameInfo::ExistingItemMap);
}

void PokemonGenerator::RefreshMoves(GameInfo::PokemonId species) {
	Refresh(species, generalMoveFilter, cache.moves, GameInfo::MoveIdList);
}

void PokemonGenerator::RefreshMin1Moves(GameInfo::PokemonId species) {
	Refresh(species, minOneMoveFilter, cache.min1Moves, cache.moves.data(), cache.moves.size());
}

void PokemonGenerator::GenSpecies(DefPokemon & mon)
{
	RefreshSpecies();
	if (cache.species.size() == 0)
		return;
	std::uniform_int_distribution<int> dist(0, cache.species.size()-1);
	mon.species = cache.species[dist(Random::Generator)];
	
	
}

void PokemonGenerator::GenLevel(DefPokemon& mon)
{
	if (minLevel == maxLevel) {
		mon.level = minLevel;
		return;
	}
	levelDist.SetMinMax(minLevel, maxLevel);
	mon.level = levelDist(Random::Generator);
}

void PokemonGenerator::GenEvsIvs(DefPokemon & mon)
{
	unsigned int minEv = 0;
	unsigned int maxEv = 0xFFFF;
	unsigned int minIv = 0;
	unsigned int maxIv = 15;

	if (bstEvIvs) {
		constexpr double maxEvRange = 0.6; //in percent from ffff range
		constexpr double maxIvRange = 0.5;

		SatUAr bst = context.pokeList[mon.species - 1].CalcBST();
		SatUAr shiftedBst = std::clamp(bst.t, 200u, 680u) - 200; //from 0 to 480
		//200 bst should always be FFFF, 680bst should always be 0.
		if constexpr (false) {
			//round: 200 and 680 should have 0 derivation, while towards the middle (440) deviation should be high
			//EVs
			{
				SatUAr<unsigned int> max = (480u - shiftedBst) * 136.53125; //note: 136.53125 * 480 = 0xFFFF
				unsigned int deviation = (240 - std::abs(240 - (int)shiftedBst)) * (maxEvRange * 0xFFFF / 240.0);
				maxEv = max;
				minEv = max - deviation;
			}
			//IVs
			{
				SatUAr<unsigned int> max = (480u - shiftedBst) * 0.03125;
				unsigned int deviation = (240 - std::abs(240 - (int)(bst - 200u))) * (maxIvRange * 15 / 240.0);
				maxIv = max;
				minIv = max - deviation;
			}
		}
		else {
			//EVs
			{
				//create a parallelogram-shape on the bst/ev graph:
				//area size always N, but 0=[0xFFFF,0xFFFF+N] und 480=[0-N,0] clamped to [0,FFFF]
				constexpr double N = maxEvRange * 0xFFFF;
				//max line: b=0xFFFF+N, m = -(N/480) -(4369/32)
				constexpr double fMaxB = 0xFFFF + N;
				constexpr double fMaxM = -(N / 480) - (4369.0 / 32.0);
				maxEv = std::clamp<int>(fMaxM * shiftedBst + fMaxB, 0, 0xFFFF);
				minEv = std::clamp<int>(fMaxM * shiftedBst + fMaxB - N, 0, 0xFFFF);
			}
			//same for IVs, but 0xFFFF is just 0xF
			{
				constexpr double N = maxEvRange * 0xF;
				constexpr double fMaxB = 0xF + N;
				constexpr double fMaxM = -(N / 480) - (1.0 / 32.0);
				maxIv = std::clamp<int>(fMaxM * shiftedBst + fMaxB, 0, 0xF);
				minIv = std::clamp<int>(fMaxM * shiftedBst + fMaxB - N, 0, 0xF);
			}
		}

	}

	statsDist.SetMinMax(minEv, maxEv);
	mon.evHp = statsDist(Random::Generator);
	mon.evAtk = statsDist(Random::Generator);
	mon.evDef = statsDist(Random::Generator);
	mon.evSpd = statsDist(Random::Generator);
	mon.evSpc = statsDist(Random::Generator);

		
	
	//double nStandardDists = 2;
	statsDist.SetMinMax(minIv, maxIv);
	int dvs[4];
	for (int i = 0; i < 4; i++) dvs[i] = statsDist(Random::Generator);
	mon.dvs = dvs[0] | (dvs[1] << 4) | (dvs[2] << 8) | (dvs[3] << 12);
	
	
	
}

void PokemonGenerator::GenHappiness(DefPokemon& mon) 
{
	std::uniform_int_distribution<int> dist(0, 0xFF);
	mon.happiness = dist(Random::Generator);
}

void PokemonGenerator::GenMoves(DefPokemon & mon)
{
	RefreshMoves(mon.species);
	
	mon.move1 = mon.move2 = mon.move3 = mon.move4 = (GameInfo::MoveId)0;

	std::uniform_int_distribution<int> dist(0, cache.moves.size()-1);
	//generate first move specially if requested
	if (minOneMoveFilter) {
		RefreshMin1Moves(mon.species);
		if (cache.min1Moves.size() > 0) {
			std::uniform_int_distribution<int> dist(0, (int)cache.min1Moves.size() - 1);
			mon.move1 = cache.min1Moves[dist(Random::Generator)];
		}
	}
	if (mon.move1 == 0) {
		mon.move1 = cache.moves[dist(Random::Generator)];
	}

	//prevent having the same move twice, the lazy way.
	//todo: implement a proper shifting approach (which will require to know
	//the index of move1 in cache.moves)
	if (cache.moves.size() < 2) return;
	do {
		mon.move2 = cache.moves[dist(Random::Generator)];
	} while (mon.move2 == mon.move1);
	if (cache.moves.size() < 3) return;
	do {
		mon.move3 = cache.moves[dist(Random::Generator)];
	} while (mon.move3 == mon.move1 || mon.move3 == mon.move2);
	if (cache.moves.size() < 4) return;
	do {
		mon.move4 = cache.moves[dist(Random::Generator)];
	} while (mon.move4 == mon.move1 || mon.move4 == mon.move2 || mon.move4 == mon.move3);


}

void PokemonGenerator::GenMovesBasedOnOldMovePower(DefPokemon& mon) {
	double oldRating = RateMove(GameInfo::Moves[mon.move1]);
	RefreshMoves(mon.species);
	std::sort(cache.moves.begin(), cache.moves.end(), [&](GameInfo::MoveId lhs, GameInfo::MoveId rhs) {
		return RateMove(context.moveList[lhs]) < RateMove(context.moveList[rhs]); 
	});
	if (minOneMoveFilter) {
		RefreshMin1Moves(mon.species);
		if (cache.min1Moves.size() > 0) {
			std::uniform_int_distribution<int> dist(0, (int)cache.min1Moves.size() - 1);
			mon.move1 = cache.min1Moves[dist(Random::Generator)];
		}
	}
}

void PokemonGenerator::GenItem(DefPokemon & mon)
{
	RefreshItems(mon.species);

	std::uniform_int_distribution<int> dist(0, (int)cache.items.size() - 1);
	mon.item = cache.items[dist(Random::Generator)];
}
