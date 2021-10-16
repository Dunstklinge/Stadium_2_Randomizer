#include "stdafx.h"
#include "DiscreteDistribution.h"
#include "GlobalRandom.h"
#include "GeneratorUtil.h"
#include "json.h"
using namespace nlohmann;

#define _USE_MATH_DEFINES
#include <math.h>

double DiscreteDistribution::ChanceBetween(double lhs, double rhs, Scaling scaling, Borders borders) const {
	switch (GetType()) {
	case UNIFORM: {
		double odds = (rhs - lhs) / (scaling.right - scaling.left);
		return std::clamp(odds, 0.0, 1.0);
	}
	case NORMAL: {
		auto cdf = [](double x, double mean, double stddev) {
			return 0.5 * erfc((mean - x) * M_SQRT1_2 * (1 / stddev));
		};
		double mean = np.mean(scaling);
		double stddev = np.stddev(scaling);
		double lhsCdf = cdf(lhs, mean, stddev);
		double rhsCdf = cdf(rhs, mean, stddev);
		double chance = rhsCdf - lhsCdf;
		double subMinChance = cdf(borders.min, mean, stddev);
		double overMaxChance = 1 - cdf(borders.max, mean, stddev);
		if (GetEdgeType() == CLAMP) {
			if (lhs <= borders.min) {
				chance += subMinChance;
			}
			if (rhs >= borders.max+1) {
				chance += overMaxChance;
			}
		}
		else {
			double unreachablePart = subMinChance + overMaxChance;
			chance *= (1 + unreachablePart);
		}
		return chance; }
	case TRIANGLE: {
		lhs = std::clamp(lhs, scaling.left, scaling.right);
		rhs = std::clamp(rhs, scaling.left, scaling.right);
		Point a = tp.a(scaling);
		Point c = tp.c(scaling);
		Point b = tp.b(scaling);

		auto areaOf = [&](Point start, Point end) {
			//if we rotate the graph in our head it looks like a trapezoid with just one slope
			//so we can just calculate the area geometrically
			double a = start.y;
			double c = end.y;
			double h = (end.x - start.x);
			return (a + c) / 2 * h;
		};
		auto yAt = [&](double x) {
			if (x < c.x) {
				double percentAlong = (x - a.x) / (c.x - a.x);
				return a.y + (c.y - a.y) * percentAlong;
			}
			else if (x > c.x) {
				double percentAlong = (x - c.x) / (b.x - c.x);
				return c.y + (b.y - c.y) * percentAlong;
			}
			return c.y;
		};
		double fullArea = areaOf(a, c) + areaOf(c, b);
		double coveredArea = 0;
		if (lhs < c.x) {
			Point lhsP { lhs, yAt(lhs) };
			Point endP = rhs >= c.x ? c : Point{ rhs, yAt(rhs) };
			coveredArea += areaOf(lhsP, endP);
		}
		if (rhs > c.x) {
			Point startP = lhs < c.x ? c : Point{ lhs, yAt(lhs) };
			Point rhsP{ rhs, yAt(rhs) };
			coveredArea += areaOf(startP, rhsP);
		}
		if (GetEdgeType() == CLAMP) {
			if (lhs <= borders.min && lhs > scaling.left) {
				Point lhsP{ lhs, yAt(lhs) };
				coveredArea += areaOf(a, lhsP);
			}
			if (rhs >= borders.max+1 && rhs < scaling.right) {
				Point rhsP{ rhs, yAt(rhs) };
				coveredArea += areaOf(rhsP, b);
			}
		}
		return coveredArea; }
	default:
		return 0;
	}
}

void to_json(json& j, const DiscreteDistribution& d) {
	DiscreteDistribution::toJson(j, d);
}

void from_json(const json& j, DiscreteDistribution& d) {
	DiscreteDistribution::fromJson(j, d);
}

void DiscreteDistribution::toJson(json& j, const DiscreteDistribution& d) {
	j["min"] = d.GetBorders().min;
	j["max"] = d.GetBorders().max;
	j["scalingLeft"] = d.GetScaling().left;
	j["scalingRight"] = d.GetScaling().right;
	j["type"] = d.GetType();
	j["edgeType"] = d.GetEdgeType();
	if (d.GetType() == DiscreteDistribution::NORMAL) {
		j["unionParams"]["nStandardDerivations"] = d.np.nStandardDerivations;
		j["unionParams"]["xTranslation"] = d.np.xTranslation;
	}
	else if (d.GetType() == DiscreteDistribution::TRIANGLE) {
		j["unionParams"]["leftHeight"] = d.tp.leftHeight;
		j["unionParams"]["centerHeight"] = d.tp.centerHeight;
		j["unionParams"]["centerX"] = d.tp.centerX;
		j["unionParams"]["rightHeight"] = d.tp.rightHeight;
	}
}
void DiscreteDistribution::fromJson(const json& j, DiscreteDistribution& d) {
	j.at("min").get_to(d.defBorders.min);
	j.at("max").get_to(d.defBorders.max);
	j.at("scalingLeft").get_to(d.defScaling.left);
	j.at("scalingRight").get_to(d.defScaling.right);
	j.at("type").get_to(d.type);
	j.at("edgeType").get_to(d.edgeType);
	if (d.type == DiscreteDistribution::NORMAL) {
		j.at("unionParams").at("nStandardDerivations").get_to(d.np.nStandardDerivations);
		j.at("unionParams").at("xTranslation").get_to(d.np.xTranslation);
	}
	else if (d.type == DiscreteDistribution::TRIANGLE) {
		j.at("unionParams").at("leftHeight").get_to(d.tp.leftHeight);
		j.at("unionParams").at("centerHeight").get_to(d.tp.centerHeight);
		j.at("unionParams").at("centerX").get_to(d.tp.centerX);
		j.at("unionParams").at("rightHeight").get_to(d.tp.rightHeight);
	}
}