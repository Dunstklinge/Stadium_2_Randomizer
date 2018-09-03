#pragma once

#include "Tables.h"



bool FilterPokemonByBST(GameInfo::PokemonId mon, unsigned int minBST, unsigned int maxBST);

bool FilterMoveByBP(GameInfo::MoveId, unsigned int minBP, unsigned int maxBP);
bool FilterMoveByStab(GameInfo::MoveId, GameInfo::PokemonId);
bool FilterOutLittlecupMoves(GameInfo::MoveId, GameInfo::PokemonId);

extern GameInfo::PokemonId LittlecupLegalMons[86];
extern GameInfo::PokemonId PokecupLegalMons[246];

extern GameInfo::PokemonId ChallengecupLegalMonsPokeball[57];
extern GameInfo::PokemonId ChallengecupLegalMonsGreatball[58];
extern GameInfo::PokemonId ChallengecupLegalMonsUltraball[58];
extern GameInfo::PokemonId ChallengecupLegalMonsMasterball[59];