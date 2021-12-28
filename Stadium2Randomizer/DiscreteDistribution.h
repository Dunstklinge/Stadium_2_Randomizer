#pragma once

#include "GeneratorUtil.h"
#include "Util.h"
#include "GlobalRandom.h"
#include "json_fwd.h"

/// <summary>
/// Represents a Distribution that produces descrete integer values.
/// Each Distribution is defined in a relative way; that is, it can generate
/// values in different ranges, and will produce the same shape in those intervals.
/// </summary>
class DiscreteDistribution
{
public:
	struct Scaling {
		Scaling(int left, int right) : left(left), right(right + 1) {}
		Scaling MoveByPercent(double amount) { 
			Scaling ret = *this;
			ret.left = left + (right - left) * amount;
			ret.right = right + (right - left) * amount;
			return ret;
		}
		Scaling CenterAround(double newCenter) {
			Scaling ret = *this;
			int currCenter = right - left;
			double move = newCenter - currCenter;
			ret.left = left + move;
			ret.right = right + move;
			return ret;
		}
	private:
		friend class DiscreteDistribution;
		double left;
		double right;
	};
	struct Borders {
		int min;
		int max;
	};
	struct normalParams {
		double nStandardDerivations;
		double xTranslation;
		double mean(Scaling scaling) const {
			return (scaling.left + scaling.right) / 2.0 + (scaling.right - scaling.left) / 2.0 * xTranslation;
		}
		double stddev(Scaling scaling) const {
			return (scaling.right - scaling.left) / (2 * nStandardDerivations);
		}
	};
	struct triangleParams {
		double leftHeight;
		double centerHeight;
		double centerX;
		double rightHeight;

		Point a(Scaling scaling) const {
			return { double(scaling.left), leftHeight };
		}
		Point b(Scaling scaling) const {
			return { double(scaling.right), rightHeight };
		}
		Point c(Scaling scaling) const {
			const Point pA = a(scaling);
			const Point pB = b(scaling);
			Point pC = { (scaling.left + scaling.right) / 2.0 + (scaling.right - scaling.left) / 2.0 * centerX, centerHeight };
			if (pC.x < pA.x) pC.x = pA.x;
			if (pC.x > pB.x) pC.x = pB.x;
			if (pA.y == 0 && pC.y == 0 && pB.y == 0) pC.y = 1;
			return pC;
		}
	};

	enum Type {
		UNIFORM,
		NORMAL,
		TRIANGLE,
	};
	enum EdgeType {
		CLAMP,
		REROLL
	};
	inline void MakeUniform() { type = UNIFORM; }
	inline void MakeNormal(normalParams np) { type = NORMAL; this->np = np; }
	inline void MakeTriangle(triangleParams tp) { type = TRIANGLE; this->tp = tp; }

	inline Type GetType() const { return type; }
	inline EdgeType GetEdgeType() const { return edgeType; }
	inline void SetEdgeType(EdgeType t) { edgeType = t; }


	static void toJson(nlohmann::json& j, const DiscreteDistribution& d);
	static void fromJson(const nlohmann::json& j, DiscreteDistribution& d);

	union {
		normalParams np;
		triangleParams tp;
	};
private:
	Type type;
	EdgeType edgeType;
	Scaling defScaling;
	Borders defBorders;
	int defMin;
	int defMax;

public:
	DiscreteDistribution() : type(UNIFORM), edgeType(CLAMP), defScaling(0,0) { SetMinMax(0, 100); }
	DiscreteDistribution(int min, int max) : type(UNIFORM), edgeType(CLAMP), defScaling(0, 0) { SetMinMax(min, max); };
	DiscreteDistribution(int min, int max, normalParams np) : type(NORMAL), edgeType(CLAMP), np(np), defScaling(0, 0) { SetMinMax(min, max); }
	DiscreteDistribution(int min, int max, triangleParams tp) : type(TRIANGLE), edgeType(CLAMP), tp(tp), defScaling(0, 0) { SetMinMax(min, max);}

	void SetMinMax(int min, int max) { 
		defBorders.min = min;
		defBorders.max = max;
		defScaling.left = min;
		defScaling.right = max + 1;//since we generate doubles and cast to int, we need to make sure
									 //to generate between e.g [0, 5[ to get [0, 4]
									 //as doubles in [0, 4[ would make it impossible to generate the int 4
	}
	Scaling GetScaling() const { return defScaling; }
	Borders GetBorders() const { return defBorders; }
	double ChanceBetween(double lhs, double rhs, Scaling scaling, Borders borders) const;
	template<typename T>
	int operator()(T& gen) const;
	template<typename T>
	int operator()(T& gen, Borders borders) const;
	template<typename T>
	int operator()(T& gen, Scaling scaled) const;
	template<typename T>
	int operator()(T& gen, Scaling scaled, Borders borders) const;

	inline double TheoreticalOdds(int num) const {
		return ChanceBetween(num, num + 1, defScaling, defBorders);
	}
	inline double TheoreticalOdds(int num, Scaling scaling) const {
		return ChanceBetween(num, num + 1, scaling, defBorders);
	}
	inline double TheoreticalOdds(double min, double max, Scaling scaling) {
		return ChanceBetween(min, max, scaling, defBorders);
	}
private:
	template<typename T>
	int GenValueClamp(T& gen, Scaling scaling, Borders borders) const;
	template<typename T>
	int GenValueRetry(T& gen, int nTries, Scaling scaling, Borders borders) const;

	template<typename T>
	int TryGenValue(T& gen, Scaling scaling, Borders borders) const;

	static double NormalCdf(double x, double mean, double stddev);
	static double NormalIcdf(double x, double mean, double stddev);
};

void to_json(nlohmann::json& j, const DiscreteDistribution& d);
void from_json(const nlohmann::json& j, DiscreteDistribution& d);

template<typename T>
int DiscreteDistribution::TryGenValue(T& gen, Scaling scaling, Borders borders) const {
	if (scaling.right <= scaling.left) return scaling.left;
	switch (type) {
	case NORMAL: {
		double num;
		if (edgeType == REROLL) {
			//if we want a truncated value, we can generate them directly with a better algorithm
			//than a redraw-until-succeed approach:
			//inverse cdf(y) = mean + sqrt(2)*sigma *InverseErf[-1 + 2*y]
			//nehme uniform distributed werte zwischen cdf(xMin), cdf(xMax)
			//schmeiße die in inverse cdf
			//ergebnis sind truncated random numbers
			double cdfMin = NormalCdf(borders.min, np.mean(scaling), np.stddev(scaling));
			double cdfMax = NormalCdf(borders.max, np.mean(scaling), np.stddev(scaling));
			std::uniform_real_distribution<double> udist(cdfMin, cdfMax);
			double y = udist(gen);
			//note that icdf(1) is infinity and thus would break here
			if (y >= 1) {
				num = borders.min;
			}
			else {
				num = NormalIcdf(y, np.mean(scaling), np.stddev(scaling));
			}
		}
		else {
			std::normal_distribution<double> dist(np.mean(scaling), np.stddev(scaling));
			num = dist(gen);
		}
		
		return int(num); }
	case TRIANGLE: {
		Point lhs = tp.a(scaling);
		Point c = tp.c(scaling);
		Point rhs = tp.b(scaling);

		double x[] = { lhs.x, c.x, rhs.x };
		double y[] = { lhs.y, c.y, rhs.y };
		std::piecewise_linear_distribution<double> dist(std::begin(x), std::end(x), std::begin(y));
		return int(dist(gen)); }
	case UNIFORM:
	default: {
		std::uniform_int_distribution<int> dist(borders.min, borders.max);
		return dist(gen); }
	}
}

template<typename T>
int DiscreteDistribution::GenValueClamp(T& gen, Scaling scaling, Borders borders) const {
	if (borders.min >= borders.max) return borders.max;
	return std::clamp(TryGenValue(gen, scaling, borders), borders.min, borders.max);
}
template<typename T>
int DiscreteDistribution::GenValueRetry(T& gen, int nTries, Scaling scaling, Borders borders) const {
	if (borders.min >= borders.max) return borders.max;
	if (type == NORMAL) {
		
	}
	for (int i = 0; i < nTries; i++) {
		int v = TryGenValue(gen, scaling, borders);
		if (borders.min <= v && v <= borders.max)
			return v;
	}
	return GenValueClamp(gen, scaling, borders);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen) const
{
	if (edgeType == CLAMP)
		return GenValueClamp(gen, defScaling, defBorders);
	else
		return GenValueRetry(gen, 10, defScaling, defBorders);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen, Borders borders) const
{
	if (edgeType == CLAMP)
		return GenValueClamp(gen, defScaling, borders);
	else
		return GenValueRetry(gen, 10, defScaling, borders);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen, Scaling scaling) const {
	if (edgeType == CLAMP)
		return GenValueClamp(gen, scaling, defBorders);
	else
		return GenValueRetry(gen, 10, scaling, defBorders);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen, Scaling scaling, Borders borders) const {
	if (edgeType == CLAMP)
		return GenValueClamp(gen, scaling, borders);
	else
		return GenValueRetry(gen, 10, scaling, borders);
}