#include "DefTrainer.h"

#include "Tables.h"
#include "DefPokemon.h"

void DefTrainer::Print(DefText* texts, std::ostream & o)
{
	auto it = texts->begin();
	
	o << it[TableInfo::TEXT_TRAINER_NAMES][trainerId - 1] << "\n";
	o << "cat " << (int)trainerCat << ", " << (int)nPokes << " pokes:\n";
	for (int i = 0; i < nPokes; i++) {
		o << "Nickname: ";
		char* nick = it[TableInfo::TEXT_TRAINER_TEXT_FIRST + textId][TableInfo::TRAINERTEXT_NICKNAME1 + i];
		if (*nick) o << nick;
		else o << "<default>";
		o << "\n";
		pokemon[i].Print(o);
	}
}
