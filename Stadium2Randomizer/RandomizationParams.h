#pragma once

#include "DiscreteDistribution.h"

#include <string>

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
			bool bp;
			DiscreteDistribution bpDist;
			bool closeBp;
			bool pp;
			DiscreteDistribution ppDist;
			bool statusMoves;
			bool type;
			bool secEffect;
			bool secEffectChance;
			double secEffectAddPercent;
			double secEffectRemPercent;
			DiscreteDistribution secEffectChanceDist;
		};
		Moves moves;
		struct Species {
			bool randomizeBst;
			DiscreteDistribution bstDist;
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
		bool randEvIv;
		DiscreteDistribution randEvIvDist;
		bool randMoves;
		bool randMovesBalanced;
		DiscreteDistribution randMovesBalancedDist;
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
		DiscreteDistribution trainerRandIvEvDist;
		bool trainerRandPoke;
		int trainerRandMovesDetails;
		DiscreteDistribution trainerRandMovesDetailsDist;
	} trainerMons;

	int min1Buttons;

	wchar_t romPath[256];
	unsigned long seed;

	bool legalMovesOnly;
	bool patchCic;

	std::string Serialize();
	bool Deserialize(std::string_view obj);
};

