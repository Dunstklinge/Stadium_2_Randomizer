#include <iostream>
#include <fstream>
#include <Windows.h>

#include "DefRoster.h"
#include "Tables.h"
#include "GlobalRandom.h"
#include "TextMods.h"
#include "PokemonGenerator.h"
#include "TrainerGenerator.h"
#include "GlobalConfigs.h"

int main(int argc, char* argv[]) {
	GameInfo::InitTables();
	Random::Init();
	GlobalConfig::Init();

	const char* param;
	if (argc == 1) {
		param = "Pokemon Stadium 2 (U) [!].z64";
	}
	else {
		param = argv[1];
	}

	std::ifstream in(param, std::ifstream::binary);

	DefRoster* roster = DefRoster::FromFileStream(in);
	roster->Curate(true);

	//lets try randomizing little cup for now
	PokemonGenerator pokeGen;
	pokeGen.changeSpecies = false;
	pokeGen.changeEvsIvs = true;
	pokeGen.changeLevel = false;
	pokeGen.changeMoves = true;

	pokeGen.minLevel = 5;
	pokeGen.maxLevel = 5;
	pokeGen.randEvs = true;
	pokeGen.randIvs = true;

	auto rentalIt = roster->rentalBegin();
	auto& littlecupRentals = rentalIt[0];
	for (int i = 0; i < littlecupRentals.nPokemon; i++) {
		DefPokemon newMon = pokeGen.Generate(littlecupRentals.pokemon[i]);
		littlecupRentals.pokemon[i] = newMon;
	}

	//do a trainer
	TrainerGenerator tgen;
	//tgen.minOneMove = tgen.MIN_ONE_STRONG_STAB;
	tgen.usefulItem = true;
	auto trainerIt = roster->trainerBegin();
	DefTrainer newDef = tgen.Generate(trainerIt->trainers[0]);


	//try changing a name

	//get old text
	DefText* oldText = DefText::FromFileStream(in);
	oldText->Curate(true);

	//auto trainerIt = roster->trainerBegin();
	uint8_t trainerId, textId;
	trainerId = trainerIt[TableInfo::FALKNER_GYM_R1].trainers[0].trainerId;
	textId = trainerIt[TableInfo::FALKNER_GYM_R1].trainers[0].textId;
	const char* testname = "testname";

	TextMods mods;
	mods.AddChange(TableInfo::TEXT_TRAINER_TEXT_FIRST + textId, TableInfo::TRAINERTEXT_NICKNAME1, testname);
	DefText* newText = (DefText*)new uint8_t[DefText::segSize];
	mods.Apply(oldText, newText);

	newText->Curate(false);

	roster->Curate(false);

	BOOL suc = CopyFileA(param, "Pokemon Stadium 2 modified (U) [!].z64", FALSE);
	if (suc) {
		std::ofstream out("Pokemon Stadium 2 modified (U) [!].z64", std::ofstream::in | std::ofstream::out | std::ofstream::binary);
		out.seekp(DefRoster::offStart);
		out.write((char*)roster, DefRoster::offEnd - DefRoster::offStart);
		
		out.seekp(DefText::offStart);
		out.write((char*)newText, DefText::segSize);
	}



	delete[]((uint8_t*)roster);

	return 0;
}

int main2(int argc, char* argv[]) {
	GameInfo::InitTables();
	Random::Init();

	const char* param;
	if (argc == 1) {
		param = "Pokemon Stadium 2 (U) [!].z64";
	}
	else {
		param = argv[1];
	}

	DefRoster* roster = DefRoster::FromFile(param);
	roster->Curate(true);
	DefText* oldText = DefText::FromFile(param);
	oldText->Curate(true);

	//PrintAllRosterTables(roster);
	PrintAllNicknames(roster, oldText);
	PrintAllTrainerNames(roster, oldText);

	roster->Curate(false);

	delete[]((uint8_t*)roster);
	delete[]((uint8_t*)oldText);
	
	return 0;
}