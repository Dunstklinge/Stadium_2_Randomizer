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

	DefPokemon Generate() const;
	DefPokemon Generate(GameInfo::PokemonId species) const; //generates mon of this species
	DefPokemon Generate(const DefPokemon& from) const; //may keep info from this mon

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
	static DiscreteDistribution::Scaling BstEvIvBias(DiscreteDistribution::Scaling startScaling, double bias) {
		return startScaling.MoveByPercent(bias * 3);
	}
	static DiscreteDistribution::Scaling MovePowerBias(DiscreteDistribution::Scaling startScaling, double bias) {
		return startScaling.MoveByPercent(bias * 3);
	}

	inline void ClearAllFilters() {
		speciesFilter = Filter<GameInfo::PokemonId>{};
		itemFilter = Filter<GameInfo::ItemId>{};
		//minOneMoveFilter = Filter<GameInfo::MoveId>{};
		generalMoveFilter = Filter<GameInfo::MoveId>{};
		cache = Cache{};
	}
private:
	void GenSpecies(DefPokemon& mon) const;
	void GenLevel(DefPokemon& mon) const;
	void GenEvsIvs(DefPokemon& mon) const;
	void GenHappiness(DefPokemon& mon) const;
	void GenMoves(DefPokemon& mon) const;
	void GenItem(DefPokemon& mon) const;
	void GenMovesBasedOnOldMovePower(DefPokemon& mon) const;

	struct Cache {
		std::vector<GameInfo::PokemonId> species;
		std::vector<GameInfo::ItemId> items;
		std::vector<GameInfo::MoveId> moves;
		std::vector<GameInfo::MoveId> min1Moves;
	};
	mutable Cache cache;

	GameContext context;

	template<typename T, unsigned size>
	void Refresh(GameInfo::PokemonId species, const Filter<T>& filter, std::vector<T>& cache, const T (&baseBuffer)[size]) const;
	template<typename T, typename It>
	void Refresh(GameInfo::PokemonId species, const Filter<T>& filter, std::vector<T>& cache, It iterator, unsigned size) const;
	void RefreshSpecies() const;
	void RefreshItems(GameInfo::PokemonId species) const;
	void RefreshMoves(GameInfo::PokemonId species) const;
	void RefreshMin1Moves(GameInfo::PokemonId species) const;
};

