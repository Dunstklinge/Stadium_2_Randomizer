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
	movePowerDist = DiscreteDistribution(20, 200);

	randEvs = true;
	randIvs = true;
	bstEvIvs = true;
	statsDist = DiscreteDistribution();
}


PokemonGenerator::~PokemonGenerator()
{
}

DefPokemon PokemonGenerator::Generate() const
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

DefPokemon PokemonGenerator::Generate(GameInfo::PokemonId species) const
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

DefPokemon PokemonGenerator::Generate(const DefPokemon & from) const
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
void PokemonGenerator::Refresh(GameInfo::PokemonId species, const Filter<T>& filter, std::vector<T>& cache, It iterator, unsigned size) const {
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
void PokemonGenerator::Refresh(GameInfo::PokemonId species, const Filter<T>& filter, std::vector<T>& cache, const T(&baseBuffer)[size]) const {
	Refresh(species, filter, cache, baseBuffer, size);
}

//this one has to be copy pastad because the filter accepts one less parameter, as there is no species context when generating species
void PokemonGenerator::RefreshSpecies() const {
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

void PokemonGenerator::RefreshItems(GameInfo::PokemonId species) const {
	Refresh(species, itemFilter, cache.items, GameInfo::ExistingItemMap);
}

void PokemonGenerator::RefreshMoves(GameInfo::PokemonId species) const {
	Refresh(species, generalMoveFilter, cache.moves, GameInfo::MoveIdList);
}

void PokemonGenerator::RefreshMin1Moves(GameInfo::PokemonId species) const {
	Refresh(species, minOneMoveFilter, cache.min1Moves, cache.moves.data(), cache.moves.size());
}

void PokemonGenerator::GenSpecies(DefPokemon & mon) const
{
	RefreshSpecies();
	if (cache.species.size() == 0)
		return;
	std::uniform_int_distribution<int> dist(0, cache.species.size()-1);
	mon.species = cache.species[dist(Random::Generator)];
	
	
}

void PokemonGenerator::GenLevel(DefPokemon& mon) const
{
	if (minLevel == maxLevel) {
		mon.level = minLevel;
		return;
	}

	mon.level = levelDist(Random::Generator, DiscreteDistribution::Borders { minLevel, maxLevel });
}

void PokemonGenerator::GenEvsIvs(DefPokemon & mon) const
{
	unsigned int minEv = 0;
	unsigned int maxEv = 0xFFFF;
	unsigned int minIv = 0;
	unsigned int maxIv = 15;

	if (bstEvIvs) {
		unsigned bst = context.pokeList[mon.species].CalcBST();
		SatUAr shiftedBst = std::clamp(bst, 200u, 680u) - 200; //from 0 to 480
		double bstPercent = shiftedBst / 480.0;
		double shiftPercent = ((0.5 - bstPercent) * 2); //where -1 is strongest leftshift and +1 is strongest rightshift
		

	}
	else {
		DiscreteDistribution::Scaling evScaling { int(minEv), int(maxEv) };
		mon.evHp = statsDist(Random::Generator, evScaling);
		mon.evAtk = statsDist(Random::Generator, evScaling);
		mon.evDef = statsDist(Random::Generator, evScaling);
		mon.evSpd = statsDist(Random::Generator, evScaling);
		mon.evSpc = statsDist(Random::Generator, evScaling);

		DiscreteDistribution::Scaling dvScaling{ int(minIv), int(maxIv) };
		int dvs[4];
		for (int i = 0; i < 4; i++) dvs[i] = statsDist(Random::Generator, evScaling);
		mon.dvs = dvs[0] | (dvs[1] << 4) | (dvs[2] << 8) | (dvs[3] << 12);
	}

}

void PokemonGenerator::GenHappiness(DefPokemon& mon) const
{
	std::uniform_int_distribution<int> dist(0, 0xFF);
	mon.happiness = dist(Random::Generator);
}

void PokemonGenerator::GenMoves(DefPokemon & mon) const
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

void PokemonGenerator::GenMovesBasedOnOldMovePower(DefPokemon& mon) const {
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

void PokemonGenerator::GenItem(DefPokemon & mon) const
{
	RefreshItems(mon.species);

	std::uniform_int_distribution<int> dist(0, (int)cache.items.size() - 1);
	mon.item = cache.items[dist(Random::Generator)];
}
