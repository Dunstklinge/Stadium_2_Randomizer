#include "GlobalRandom.h"

namespace Random {
	std::default_random_engine Generator;

	void Init() {
		Generator.seed(1);
	}
};
