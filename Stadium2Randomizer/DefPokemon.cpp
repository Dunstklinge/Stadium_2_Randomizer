#include "DefPokemon.h"

void DefPokemon::Print(std::ostream & o)
{
	auto flags = o.flags();
	o << GameInfo::PokemonNames[species] << " level " << (int)level << "\n";
	o << GameInfo::ItemNames[item] << "\n";
	o << GameInfo::MoveNames[move1] << ", " << GameInfo::MoveNames[move2] << ", " <<
		GameInfo::MoveNames[move3] << ", " << GameInfo::MoveNames[move4] << "\n";
	o << std::hex << "IV: " << dvs << "\n";
	o << "EVs: " << evHp << " " << evAtk << " " << evDef << " " << evSpd << " " << evSpc << "\n";
	o.flags(flags);
}
