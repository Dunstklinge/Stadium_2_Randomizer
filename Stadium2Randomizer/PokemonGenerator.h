#pragma once

#include <random>
#include <functional>
#include <vector>

#include "GameContext.h"
#include "DefPokemon.h"
#include "Tables.h"
#include "Constants.h"
#include "DiscreteDistribution.h"

class PokemonGenerator
{
public:
	PokemonGenerator();
	PokemonGenerator(GameContext context);
	~PokemonGenerator();

	DefPokemon Generate();
	DefPokemon Generate(GameInfo::PokemonId species); //generates mon of this species
	DefPokemon Generate(const DefPokemon& from); //may keep info from this mon

	/* for Generate from an existing pokemon */
	bool changeSpecies;
	bool changeEvsIvs;
	bool changeLevel;
	bool changeMoves;
	bool changeItem;
	bool changeHappiness;

	/* new generation info */ 

	template<typename Id>
	struct Filter {
		std::function<bool(Id, GameInfo::PokemonId)> func = nullptr;
		const Id* buffer = nullptr;
		unsigned bufferN = 0;

		explicit operator bool() const {
			return func == nullptr && (buffer == nullptr || bufferN == 0);
		}
	};
	template<>
	struct Filter<GameInfo::PokemonId> {
		std::function<bool(GameInfo::PokemonId)> func = nullptr;
		const GameInfo::PokemonId* buffer = nullptr;
		unsigned bufferN = 0;

		explicit operator bool() const {
			return func == nullptr && (buffer == nullptr || bufferN == 0);
		}
	};
	
	//for species only, BOTH filter and buffer are used
	Filter<GameInfo::PokemonId> speciesFilter;

	int minLevel;
	int maxLevel;
	DiscreteDistribution levelDist;

	Filter<GameInfo::ItemId> itemFilter;
	
	Filter<GameInfo::MoveId> minOneMoveFilter;
	enum MoveRandMode {
		EqualChance, BasedOnOldMovePower, BasedOnSpeciesBst
	} moveRandMove;
	DiscreteDistribution movePowerDist;

	Filter<GameInfo::MoveId> generalMoveFilter;
	
	bool randEvs;
	bool randIvs;
	bool bstEvIvs;
	DiscreteDistribution statsDist;
	//bool statsUniformDistribution;

	inline void ClearAllFilters() {
		speciesFilter = Filter<GameInfo::PokemonId>{};
		itemFilter = Filter<GameInfo::ItemId>{};
		//minOneMoveFilter = Filter<GameInfo::MoveId>{};
		generalMoveFilter = Filter<GameInfo::MoveId>{};
		cache = Cache{};
	}
private:
	void GenSpecies(DefPokemon& mon);
	void GenLevel(DefPokemon& mon);
	void GenEvsIvs(DefPokemon& mon);
	void GenHappiness(DefPokemon& mon);
	void GenMoves(DefPokemon& mon);
	void GenItem(DefPokemon& mon);
	void GenMovesBasedOnOldMovePower(DefPokemon& mon);

	struct Cache {
		std::vector<GameInfo::PokemonId> species;
		std::vector<GameInfo::ItemId> items;
		std::vector<GameInfo::MoveId> moves;
		std::vector<GameInfo::MoveId> min1Moves;
	};
	Cache cache;

	GameContext context;

	template<typename T, unsigned size>
	void Refresh(GameInfo::PokemonId species, Filter<T>& filter, std::vector<T>& cache, const T (&baseBuffer)[size]);
	template<typename T, typename It>
	void Refresh(GameInfo::PokemonId species, Filter<T>& filter, std::vector<T>& cache, It iterator, unsigned size);
	void RefreshSpecies();
	void RefreshItems(GameInfo::PokemonId species);
	void RefreshMoves(GameInfo::PokemonId species);
	void RefreshMin1Moves(GameInfo::PokemonId species);
};

