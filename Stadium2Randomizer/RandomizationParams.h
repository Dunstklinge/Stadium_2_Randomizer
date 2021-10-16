#pragma once

#include "DiscreteDistribution.h"

#include <string>
#include <algorithm>

typedef std::pair<int, int> DistBounds;

/*
* Pure data object to contain all the possible choices for randomization.
*/

struct RandomizationParams
{
	struct Uber {
		struct Moves {
			bool randomize;
			bool balance;
			bool acc;
			DiscreteDistribution accDist;
			static constexpr DistBounds accDistBounds = { 1, 255 }; //internal representation of percent
			bool bp;
			DiscreteDistribution bpDist;
			static constexpr DistBounds bpDistBounds = { 1, 250 }; //for the freedom to make bonkers op moves
			DiscreteDistribution bpRelDist;
			bool closeBp;
			bool pp;
			DiscreteDistribution ppDist;
			static constexpr DistBounds ppDistBounds = { 1, 63 }; //due to highest 2 bits tracking pp ups
			bool statusMoves;
			bool type;
			bool secEffect;
			bool secEffectChance;
			double secEffectAddPercent;
			double secEffectRemPercent;
			DiscreteDistribution secEffectChanceDist;
			static constexpr DistBounds secEffectChanceDistBounds = { 1, 255 }; //internal representation of percent
		};
		Moves moves;
		struct Species {
			bool randomizeBst;
			DiscreteDistribution bstDist;
			static constexpr DistBounds bstDistBounds = { 50, 1000 }; //for the freedom to make bonkers op mons
			bool evoBst;
			bool evoStats;
			enum {
				KeepBst,
				StayCloseToBst,
				RandomBst
			} bstType;
			bool randomizeTypes;
			double addTypePercent;
			double removeTypePercent;
		};
		Species species;
	};
	Uber uber;

	struct Rentals {
		bool randRentals;
		bool randRentalHappiness;
		bool randLevels;
		DiscreteDistribution randLevelsDist;
		static constexpr DistBounds randLevelDistBounds = { 50, 55 };
		bool randEvIv;
		DiscreteDistribution randRelEvIvDist;
		bool randMoves;
		bool randMovesBalanced;
		DiscreteDistribution  randRelMovesBalancedDist;
		bool randPrimecupLevels;
		bool randChooseItems;
		bool changeItemN;
		int changeItemNAmount;
		bool itemsPerRentalSet;
		int strPrimeCupLevel;
		bool randIncludeMonspecificItems;
		bool changeGlcRentalLevel;
		int seperateGlcRentalsLevel;
		bool rentalSpeciesEvIv;
		bool moreRentalTables;
		bool multipleGlcRentals;
		bool multiplePokecupRentals;
		bool multipleR2Rentals;
	};
	Rentals rentals;

	struct Trainers {
		bool randName;
		bool shuffleBosses;
		bool shuffleCross;
		bool shuffleRegulars;
		bool mixCustomsInBosses;
		bool mixCustomsInTrainers;
		int strCustomTrainerN;
	} trainers;

	struct TrainerMons {
		bool trainerRand;
		bool battleItemsOnly;
		bool randLevels;
		bool randMonNames;
		bool randSpecies;
		bool trainerMin1Atk;
		bool stayCloseToBST;
		bool bossStayCloseToBST;
		bool trainerRandMoves;
		bool trainerRandEvIv;
		bool trainerRandHappiness;
		bool trainerRandItems;
		DiscreteDistribution  trainerRandRelIvEvDist;
		bool trainerRandPoke;
		int trainerRandMovesDetails;
		DiscreteDistribution  trainerRandRelMovesDetailsDist;
	} trainerMons;

	int min1Buttons;

	wchar_t romPath[256];
	unsigned long seed;

	bool legalMovesOnly;
	bool patchCic;

	std::string Serialize();
	bool Deserialize(std::string_view obj);
};

