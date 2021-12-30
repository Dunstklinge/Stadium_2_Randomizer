#include "stdafx.h"

#include "RandomizationParams.h"
#include "DiscreteDistribution.h"

#include "json.h"
using namespace nlohmann;

//fuck this language needs fucking reflection so badly
std::string RandomizationParams::Serialize() {
	json j;
	std::string retVal;

	try {
		j["uber"]["moves"]["randomize"] = uber.moves.randomize;
		j["uber"]["moves"]["balance"] = uber.moves.balance;
		j["uber"]["moves"]["acc"] = uber.moves.acc;
		j["uber"]["moves"]["accDist"] = uber.moves.accDist;
		j["uber"]["moves"]["bp"] = uber.moves.bp;
		j["uber"]["moves"]["bpDist"] = uber.moves.bpDist;
		j["uber"]["moves"]["closeBp"] = uber.moves.closeBp;
		j["uber"]["moves"]["pp"] = uber.moves.pp;
		j["uber"]["moves"]["ppDist"] = uber.moves.ppDist;
		j["uber"]["moves"]["statusMoves"] = uber.moves.statusMoves;
		j["uber"]["moves"]["type"] = uber.moves.type;
		j["uber"]["moves"]["secEffect"] = uber.moves.secEffect;
		j["uber"]["moves"]["secEffectChance"] = uber.moves.secEffectChance;
		j["uber"]["moves"]["secEffectAddPercent"] = uber.moves.secEffectAddPercent;
		j["uber"]["moves"]["secEffectRemPercent"] = uber.moves.secEffectRemPercent;
		j["uber"]["moves"]["secEffectChanceDist"] = uber.moves.secEffectChanceDist;

		j["uber"]["species"]["randomizeBst"] = uber.species.randomizeBst;
		j["uber"]["species"]["bstDist"] = uber.species.bstDist;
		j["uber"]["species"]["evoBst"] = uber.species.evoBst;
		j["uber"]["species"]["evoStats"] = uber.species.evoStats;
		j["uber"]["species"]["bstType"] = uber.species.bstType;
		j["uber"]["species"]["randomizeTypes"] = uber.species.randomizeTypes;
		j["uber"]["species"]["addTypePercent"] = uber.species.addTypePercent;
		j["uber"]["species"]["removeTypePercent"] = uber.species.removeTypePercent;

		j["rentals"]["randRentals"] = rentals.randRentals;
		j["rentals"]["randRentalHappiness"] = rentals.randRentalHappiness;
		j["rentals"]["randLevels"] = rentals.randLevels;
		j["rentals"]["randLevelsDist"] = rentals.randLevelsDist;
		j["rentals"]["randEvIv"] = rentals.randEvIv;
		j["rentals"]["randEvIvDist"] = rentals.randRelEvIvDist;
		j["rentals"]["randMoves"] = rentals.randMoves;
		j["rentals"]["randMovesBalanced"] = rentals.randMovesBalanced;
		j["rentals"]["randMovesBalancedDist"] = rentals.randRelMovesBalancedDist;
		j["rentals"]["randPrimecupLevels"] = rentals.randPrimecupLevels;
		j["rentals"]["randChooseItems"] = rentals.randChooseItems;
		j["rentals"]["changeItemN"] = rentals.changeItemN;
		j["rentals"]["changeItemNAmount"] = rentals.changeItemNAmount;
		j["rentals"]["itemsPerRentalSet"] = rentals.itemsPerRentalSet;
		j["rentals"]["strPrimeCupLevel"] = rentals.strPrimeCupLevel;
		j["rentals"]["randIncludeMonspecificItems"] = rentals.randIncludeMonspecificItems;
		j["rentals"]["changeGlcRentalLevel"] = rentals.changeGlcRentalLevel;
		j["rentals"]["glcTableCount"] = rentals.glcTableCount;
		j["rentals"]["seperateGlcRentalsLevel"] = rentals.seperateGlcRentalsLevel;
		j["rentals"]["rentalSpeciesEvIv"] = rentals.rentalSpeciesEvIv;
		j["rentals"]["moreRentalTables"] = rentals.moreRentalTables;
		j["rentals"]["multipleGlcRentals"] = rentals.multipleGlcRentals;
		j["rentals"]["multiplePokecupRentals"] = rentals.multiplePokecupRentals;
		j["rentals"]["multipleR2Rentals"] = rentals.multipleR2Rentals;

		j["trainers"]["trainerRand"] = trainers.trainerRand;
		j["trainers"]["randName"] = trainers.randName;
		j["trainers"]["shuffleBosses"] = trainers.shuffleBosses;
		j["trainers"]["shuffleCross"] = trainers.shuffleCross;
		j["trainers"]["shuffleRegulars"] = trainers.shuffleRegulars;
		j["trainers"]["mixCustomsInBosses"] = trainers.mixCustomsInBosses;
		j["trainers"]["mixCustomsInTrainers"] = trainers.mixCustomsInTrainers;
		j["trainers"]["strCustomTrainerN"] = trainers.strCustomTrainerN;

		j["trainerMons"]["battleItemsOnly"] = trainerMons.battleItemsOnly;
		j["trainerMons"]["randLevels"] = trainerMons.randLevels;
		j["trainerMons"]["randMonNames"] = trainerMons.randMonNames;
		j["trainerMons"]["randSpecies"] = trainerMons.randSpecies;
		j["trainerMons"]["trainerMin1Atk"] = trainerMons.trainerMin1Atk;
		j["trainerMons"]["speciesUsesBstDist"] = trainerMons.speciesUsesBstDist;
		j["trainerMons"]["speciesBstDist"] = trainerMons.speciesBstDist;
		j["trainerMons"]["stayCloseToBST"] = trainerMons.stayCloseToBST;
		j["trainerMons"]["seperateBossSpeciesDist"] = trainerMons.seperateBossSpeciesDist;
		j["trainerMons"]["bossSpeciesBstDist"] = trainerMons.bossSpeciesBstDist;
		j["trainerMons"]["bossStayCloseToBST"] = trainerMons.bossStayCloseToBST;
		j["trainerMons"]["trainerRandMoves"] = trainerMons.trainerRandMoves;
		j["trainerMons"]["trainerRandEvIv"] = trainerMons.trainerRandEvIv;
		j["trainerMons"]["trainerRandHappiness"] = trainerMons.trainerRandHappiness;
		j["trainerMons"]["trainerRandItems"] = trainerMons.trainerRandItems;
		j["trainerMons"]["trainerRandIvEvDist"] = trainerMons.trainerRandRelIvEvDist;
		j["trainerMons"]["trainerRandPoke"] = trainerMons.trainerRandPoke;
		j["trainerMons"]["trainerRandMovesDetails"] = trainerMons.trainerRandMovesDetails;
		j["trainerMons"]["trainerRandMovesDetailsDist"] = trainerMons.trainerRandRelMovesDetailsDist;

		j["min1Buttons"] = min1Buttons;
		j["legalMovesOnly"] = legalMovesOnly;

		retVal = j.dump();
	}
	catch (json::exception& what) {
		retVal.clear();
	}
	return retVal;
}

bool RandomizationParams::Deserialize(std::string_view obj) {
	try {
		json j = json::parse(obj);

		j.at("uber").at("moves").at("randomize").get_to(uber.moves.randomize);
		j.at("uber").at("moves").at("balance").get_to(uber.moves.balance);
		j.at("uber").at("moves").at("acc").get_to(uber.moves.acc);
		j.at("uber").at("moves").at("accDist").get_to(uber.moves.accDist);
		j.at("uber").at("moves").at("bp").get_to(uber.moves.bp);
		j.at("uber").at("moves").at("bpDist").get_to(uber.moves.bpDist);
		j.at("uber").at("moves").at("closeBp").get_to(uber.moves.closeBp);
		j.at("uber").at("moves").at("pp").get_to(uber.moves.pp);
		j.at("uber").at("moves").at("ppDist").get_to(uber.moves.ppDist);
		j.at("uber").at("moves").at("statusMoves").get_to(uber.moves.statusMoves);
		j.at("uber").at("moves").at("type").get_to(uber.moves.type);
		j.at("uber").at("moves").at("secEffect").get_to(uber.moves.secEffect);
		j.at("uber").at("moves").at("secEffectChance").get_to(uber.moves.secEffectChance);
		j.at("uber").at("moves").at("secEffectAddPercent").get_to(uber.moves.secEffectAddPercent);
		j.at("uber").at("moves").at("secEffectRemPercent").get_to(uber.moves.secEffectRemPercent);
		j.at("uber").at("moves").at("secEffectChanceDist").get_to(uber.moves.secEffectChanceDist);

		j.at("uber").at("species").at("randomizeBst").get_to(uber.species.randomizeBst);
		j.at("uber").at("species").at("bstDist").get_to(uber.species.bstDist);
		j.at("uber").at("species").at("evoBst").get_to(uber.species.evoBst);
		j.at("uber").at("species").at("evoStats").get_to(uber.species.evoStats);
		j.at("uber").at("species").at("bstType").get_to(uber.species.bstType);
		j.at("uber").at("species").at("randomizeTypes").get_to(uber.species.randomizeTypes);
		j.at("uber").at("species").at("addTypePercent").get_to(uber.species.addTypePercent);
		j.at("uber").at("species").at("removeTypePercent").get_to(uber.species.removeTypePercent);

		j.at("rentals").at("randRentals").get_to(rentals.randRentals);
		j.at("rentals").at("randRentalHappiness").get_to(rentals.randRentalHappiness);
		j.at("rentals").at("randLevels").get_to(rentals.randLevels);
		j.at("rentals").at("randLevelsDist").get_to(rentals.randLevelsDist);
		j.at("rentals").at("randEvIv").get_to(rentals.randEvIv);
		j.at("rentals").at("randEvIvDist").get_to(rentals.randRelEvIvDist);
		j.at("rentals").at("randMoves").get_to(rentals.randMoves);
		j.at("rentals").at("randMovesBalanced").get_to(rentals.randMovesBalanced);
		j.at("rentals").at("randMovesBalancedDist").get_to(rentals.randRelMovesBalancedDist);
		j.at("rentals").at("randPrimecupLevels").get_to(rentals.randPrimecupLevels);
		j.at("rentals").at("randChooseItems").get_to(rentals.randChooseItems);
		j.at("rentals").at("changeItemN").get_to(rentals.changeItemN);
		j.at("rentals").at("changeItemNAmount").get_to(rentals.changeItemNAmount);
		j.at("rentals").at("itemsPerRentalSet").get_to(rentals.itemsPerRentalSet);
		j.at("rentals").at("strPrimeCupLevel").get_to(rentals.strPrimeCupLevel);
		j.at("rentals").at("randIncludeMonspecificItems").get_to(rentals.randIncludeMonspecificItems);
		j.at("rentals").at("changeGlcRentalLevel").get_to(rentals.changeGlcRentalLevel);
		j.at("rentals").at("glcTableCount").get_to(rentals.glcTableCount);
		j.at("rentals").at("seperateGlcRentalsLevel").get_to(rentals.seperateGlcRentalsLevel);
		j.at("rentals").at("rentalSpeciesEvIv").get_to(rentals.rentalSpeciesEvIv);
		j.at("rentals").at("moreRentalTables").get_to(rentals.moreRentalTables);
		j.at("rentals").at("multipleGlcRentals").get_to(rentals.multipleGlcRentals);
		j.at("rentals").at("multiplePokecupRentals").get_to(rentals.multiplePokecupRentals);
		j.at("rentals").at("multipleR2Rentals").get_to(rentals.multipleR2Rentals);

		j.at("trainers").at("trainerRand").get_to(trainers.trainerRand);
		j.at("trainers").at("randName").get_to(trainers.randName);
		j.at("trainers").at("shuffleBosses").get_to(trainers.shuffleBosses);
		j.at("trainers").at("shuffleCross").get_to(trainers.shuffleCross);
		j.at("trainers").at("shuffleRegulars").get_to(trainers.shuffleRegulars);
		j.at("trainers").at("mixCustomsInBosses").get_to(trainers.mixCustomsInBosses);
		j.at("trainers").at("mixCustomsInTrainers").get_to(trainers.mixCustomsInTrainers);
		j.at("trainers").at("strCustomTrainerN").get_to(trainers.strCustomTrainerN);

		j.at("trainerMons").at("trainerRandPoke").get_to(trainerMons.trainerRandPoke);
		j.at("trainerMons").at("battleItemsOnly").get_to(trainerMons.battleItemsOnly);
		j.at("trainerMons").at("randLevels").get_to(trainerMons.randLevels);
		j.at("trainerMons").at("randMonNames").get_to(trainerMons.randMonNames);
		j.at("trainerMons").at("randSpecies").get_to(trainerMons.randSpecies);
		j.at("trainerMons").at("trainerMin1Atk").get_to(trainerMons.trainerMin1Atk);
		j.at("trainerMons").at("speciesUsesBstDist").get_to(trainerMons.speciesUsesBstDist);
		j.at("trainerMons").at("speciesBstDist").get_to(trainerMons.speciesBstDist);
		j.at("trainerMons").at("stayCloseToBST").get_to(trainerMons.stayCloseToBST);
		j.at("trainerMons").at("bossStayCloseToBST").get_to(trainerMons.bossStayCloseToBST);
		j.at("trainerMons").at("seperateBossSpeciesDist").get_to(trainerMons.seperateBossSpeciesDist);
		j.at("trainerMons").at("bossSpeciesBstDist").get_to(trainerMons.bossSpeciesBstDist);
		j.at("trainerMons").at("trainerRandMoves").get_to(trainerMons.trainerRandMoves);
		j.at("trainerMons").at("trainerRandEvIv").get_to(trainerMons.trainerRandEvIv);
		j.at("trainerMons").at("trainerRandHappiness").get_to(trainerMons.trainerRandHappiness);
		j.at("trainerMons").at("trainerRandItems").get_to(trainerMons.trainerRandItems);
		j.at("trainerMons").at("trainerRandIvEvDist").get_to(trainerMons.trainerRandRelIvEvDist);
		j.at("trainerMons").at("trainerRandMovesDetails").get_to(trainerMons.trainerRandMovesDetails);
		j.at("trainerMons").at("trainerRandMovesDetailsDist").get_to(trainerMons.trainerRandRelMovesDetailsDist);

		j.at("min1Buttons").get_to(min1Buttons);
		j.at("legalMovesOnly").get_to(legalMovesOnly);
	}
	catch (json::exception& what) {
		return false;
	}
	return true;
}