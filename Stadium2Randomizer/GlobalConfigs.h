#pragma once

#include "PokemonNicknames.h"
#include "TrainerNames.h"
#include "CustomTrainerDefs.h"
#include "MoveEffectValue.h"

namespace GlobalConfig {

	void Init();

	extern PokemonNicknames PokemonNicks;
	extern TrainerNames TrainerNicks;
	extern CustomTrainerDefs CustomTrainers;
	extern MoveEffectValue MoveEffectValues;
}