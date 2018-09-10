#include "GlobalConfigs.h"

namespace GlobalConfig {
	PokemonNicknames PokemonNicks;
	TrainerNames TrainerNicks;

	void Init() {
		try {
			PokemonNicks = PokemonNicknames("config\\monNames.txt");
			TrainerNicks = TrainerNames("config\\trainerNames.txt");
		}
		catch (std::invalid_argument& e) {
			std::string message = e.what();
			message = "Error In Pokemon or Trainer-Name config file:\r\n\r\n" + message;
			message += "\r\n\r\nThe Name-Tables are not correctly initialized. Please fix the files and then "
				"restart the Program.";
			MessageBoxA(NULL, message.c_str(), "Error", MB_ICONERROR);
		}
	}

}