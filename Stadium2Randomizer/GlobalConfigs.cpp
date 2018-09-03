#include "GlobalConfigs.h"

namespace GlobalConfig {
	PokemonNicknames PokemonNicks;
	TrainerNames TrainerNicks;

	void Init() {
		PokemonNicks = PokemonNicknames("config\\monNames.txt");
		TrainerNicks = TrainerNames("config\\trainerNames.txt");
	}

}