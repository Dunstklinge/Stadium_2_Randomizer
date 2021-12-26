#pragma once

#include "Tables.h"
#include "GameContext.h"

bool FilterPokemonByBST(GameInfo::PokemonId mon, unsigned int minBST, unsigned int maxBST, const GameContext& context);

bool FilterMoveByBP(GameInfo::MoveId, unsigned int minBP, unsigned int maxBP, const GameContext& context);
bool FilterMoveByStab(GameInfo::MoveId, GameInfo::PokemonId, const GameContext& context);
bool FilterOutLittlecupMoves(GameInfo::MoveId, GameInfo::PokemonId, const GameContext& context);

bool FilterMetronomeOnly(GameInfo::MoveId);
bool FilterLegalMovesOnly(GameInfo::MoveId, GameInfo::PokemonId);

extern GameInfo::PokemonId LittlecupLegalMons[86];
extern GameInfo::PokemonId PokecupLegalMons[246];

extern GameInfo::PokemonId ChallengecupLegalMonsPokeball[57];
extern GameInfo::PokemonId ChallengecupLegalMonsGreatball[58];
extern GameInfo::PokemonId ChallengecupLegalMonsUltraball[58];
extern GameInfo::PokemonId ChallengecupLegalMonsMasterball[59];