#pragma once

#include <map>

#include "MoveTable.h"

class MoveEffectValue
{
public:
	MoveEffectValue();
	MoveEffectValue(const char* filename);
	~MoveEffectValue();


	struct Value {
		double m;
		double c;
		double a;
		enum {
			NONE = 0,
			MULT_BP = 1,
			FLAT_BONUS = 2,
			USE_VALUE = 4
		};
		int affects = NONE;
	};
	
	std::map<GameInfo::MoveEffect, Value> data;
};

