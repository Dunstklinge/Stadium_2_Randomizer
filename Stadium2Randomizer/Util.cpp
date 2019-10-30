#include "Util.h"

#include <fstream>
#include <map>
#include <set>

#include "Constants.h"
#include "DefText.h"
#include "DefRoster.h"
#include "Tables.h"

static void PrintPokemon(DefPokemon& poke, std::ofstream& out) {
	int species = poke.species;
	out << GameInfo::PokemonNames[species] << ": " << std::to_string(poke.level) << "\n";
	out << GameInfo::ItemNames[poke.item] << "\n";
	out << GameInfo::MoveNames[poke.move1] << ", " << GameInfo::MoveNames[poke.move2] << ", " <<
		GameInfo::MoveNames[poke.move3] << ", " << GameInfo::MoveNames[poke.move4] << "\n\n";
}

static void PrintTrainer(DefTrainer& trainer, std::ofstream& out) {
	out << "Id " << std::to_string(trainer.trainerId) << ", cat " << std::to_string(trainer.trainerCat) << "; text "
		<< std::to_string(trainer.textId) << "\n" << std::to_string(trainer.nPokes) << " pokemon:\n";
	for (int i = 0; i < trainer.nPokes; i++) {
		PrintPokemon(trainer.pokemon[i], out);
	}
}

void PrintAllRosterTables(DefRoster* roster) 
{
	std::ofstream out("rentalTables.txt");

	for (auto it = roster->rentalBegin(); it != roster->rentalEnd(); ++it) {
		out << "///////////////////////////////////////\n";
		out << "///////////////////////////////////////\n";
		out << "///////////////////////////////////////\n";
		out << "offset: " + std::to_string(it.tables->subtableInfos[it.n].tableOffset) << "\n\n";
		for (int i = 0; i < it->nPokemon; i++) {
			PrintPokemon(it->pokemon[i], out);
		}
	}

	out.close();

	out.open("trainerTables.txt");

	for (auto it = roster->trainerBegin(); it != roster->trainerEnd(); ++it) {
		out << "///////////////////////////////////////\n";
		out << "///////////////////////////////////////\n";
		out << "///////////////////////////////////////\n";
		uint32_t tableOffset = it.tables->subtableInfos[5 + it.n].tableOffset;
		char buffer[256];
		_itoa_s(tableOffset, buffer, 16);
		out << "offset: " << buffer << "(#" + std::to_string(tableOffset) + "), " << std::to_string(it->nTrainers) << " trainers\n\n";
		for (int i = 0; i < it->nTrainers; i++) {
			PrintTrainer(it->trainers[i], out);
		}
	}
}

void PrintAllNicknames(DefRoster* roster, DefText* text)
{
	std::map<GameInfo::PokemonId, std::set<std::string>> monMap;
	auto textIt = text->trainerTextBegin();

	for (auto it = roster->trainerBegin(); it != roster->trainerEnd(); ++it) {
		for (int i = 0; i < it->nTrainers; i++) {
			int textId = it->trainers[i].textId;
			for (int j = 0; j < it->trainers[i].nPokes; j++) {
				GameInfo::PokemonId species = it->trainers[i].pokemon[j].species;
				char* nickname = textIt[textId][TableInfo::TRAINERTEXT_NICKNAME1 + j];
				if(*nickname)
					monMap[species].insert(std::string(nickname));
			}
		}
	}

	std::ofstream out("nicknames.txt");

	for (auto& p : monMap) {
		const GameInfo::Pokemon& species = GameInfo::Pokemons[p.first - 1];
		out << p.first << "::" << GameInfo::PokemonNames[p.first] << "\n";
		for (auto str : p.second) {
			out << str << "\n";
		}
		out << "\n";
	}

	out.close();
}


void PrintAllTrainerNames(DefRoster* roster, DefText* text)
{
	std::map<GameInfo::TrainerCat, std::set<std::string>> monMap;

	auto textIt = text->begin();

	for (auto it = roster->trainerBegin(); it != roster->trainerEnd(); ++it) {
		for (int i = 0; i < it->nTrainers; i++) {
			DefTrainer& trainer = it->trainers[i];
			GameInfo::TrainerCat trainerCat = (GameInfo::TrainerCat)trainer.trainerCat;
			int trainerName = trainer.trainerId - 1;
			char* name = textIt[TableInfo::TEXT_TRAINER_NAMES][trainerName];
			if (*name)
				monMap[trainerCat].insert(std::string(name));
		}
	}

	std::ofstream out("trainerNames.txt");

	for (auto& p : monMap) {
		const GameInfo::Pokemon& species = GameInfo::Pokemons[p.first - 1];
		out << p.first << "::" << GameInfo::TrainerCatNames[p.first] << "\n";
		for (auto str : p.second) {
			out << str << "\n";
		}
		out << "\n";
	}

	out.close();
}

