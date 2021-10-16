#include "GlobalConfigs.h"

#include <stdexcept>

namespace GlobalConfig {
	PokemonNicknames PokemonNicks;
	TrainerNames TrainerNicks;
	CustomTrainerDefs CustomTrainers;
	MoveEffectValue MoveEffectValues;

	void Init() {
		try {
			PokemonNicks = PokemonNicknames("config\\monNames.txt");
			TrainerNicks = TrainerNames("config\\trainerNames.txt");
			CustomTrainers = CustomTrainerDefs("config\\trainers\\");
			MoveEffectValues = MoveEffectValue("config\\moveEffectValue.txt");
		}
		catch (std::invalid_argument& e) {
			std::string message = e.what();
			message = "Error In config file:\r\n\r\n" + message;
			message += "\r\n\r\nThe Name-Tables are not correctly initialized. Please fix the files and then "
				"restart the Program.";
			MessageBoxA(NULL, message.c_str(), "Error", MB_ICONERROR);
		}
	}

}