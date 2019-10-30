#include "stdafx.h"
#include "DiscreteDistribution.h"
#include "GlobalRandom.h"
#include "GeneratorUtil.h"


DiscreteDistribution::DiscreteDistribution()
	: left(0), right(0), type(Type::UNIFORM)
{
}
