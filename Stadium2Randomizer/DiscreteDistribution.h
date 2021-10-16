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
		Scaling MoveByPercent(double amount) { return Scaling(left + (right-left)*amount, right + (right - left)*amount); }
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
	int GenValueClamp(const T& func) const;
	template<typename T>
	int GenValueTruncClamp(const T& func, Borders borders) const;
	template<typename T>
	int GenValueRetry(const T& func, int nTries = 10) const;
	template<typename T>
	int GenValueTruncRetry(const T& func, int nTries, Borders borders) const;

	template<typename T>
	int GenValue(T& gen) const;
	template<typename T>
	int GenValueScaled(T& gen, Scaling scaling) const;
};

void to_json(nlohmann::json& j, const DiscreteDistribution& d);
void from_json(const nlohmann::json& j, DiscreteDistribution& d);

template<typename T>
int DiscreteDistribution::GenValueScaled(T& gen, Scaling scaling) const {
	if (scaling.right <= scaling.left) return scaling.left;
	switch (type) {
	case NORMAL: {
		std::normal_distribution<double> dist(np.mean(scaling), np.stddev(scaling));
		double num = dist(gen);
		return int(num); }
	case TRIANGLE: {
		Point lhs = tp.a(scaling);
		Point c = tp.c(scaling);
		Point rhs = tp.b(scaling);

		double x[] = { lhs.x, c.x, rhs.x };
		double y[] = { lhs.y, c.y, rhs.y };
		std::piecewise_linear_distribution<double> dist(std::begin(x), std::end(x), std::begin(y));
		return int(dist(Random::Generator)); }
	case UNIFORM:
	default: {
		std::uniform_int_distribution<int> dist(defBorders.min, defBorders.max);
		return dist(gen); }
	}
}

template<typename T>
int DiscreteDistribution::GenValue(T& gen) const {
	return GenValueScaled(gen, defScaling);
}

template<typename T>
int DiscreteDistribution::GenValueClamp(const T& func) const { return GenValueTruncClamp(func, defBorders); }
template<typename T>
int DiscreteDistribution::GenValueTruncClamp(const T& func, Borders borders) const {
	if (borders.min >= borders.max) return borders.max;
	return std::clamp(func(), borders.min, borders.max);
}
template<typename T>
int DiscreteDistribution::GenValueRetry(const T& func, int nTries) const { return GenValueTruncRetry(func, nTries, defBorders); }
template<typename T>
int DiscreteDistribution::GenValueTruncRetry(const T& func, int nTries, Borders borders) const {
	if (borders.min >= borders.max) return borders.max;
	for (int i = 0; i < nTries; i++) {
		int v = func();
		if (borders.min <= v && v <= borders.max)
			return v;
	}
	return GenValueTruncClamp(func, borders);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen) const
{
	if (edgeType == CLAMP)
		return GenValueClamp([&]() {return GenValue(gen); });
	else
		return GenValueRetry([&]() {return GenValue(gen); }, 10);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen, Borders borders) const
{
	if (edgeType == CLAMP)
		return GenValueTruncClamp([&]() {return GenValue(gen); }, defBorders);
	else
		return GenValueTruncRetry([&]() {return GenValue(gen); }, 10, defBorders);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen, Scaling scaling) const {
	if (edgeType == CLAMP)
		return GenValueClamp([&]() {return GenValueScaled(gen, scaling); });
	else
		return GenValueRetry([&]() {return GenValueScaled(gen, scaling); }, 10);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen, Scaling scaling, Borders borders) const {
	if (edgeType == CLAMP)
		return GenValueTruncClamp([&]() {return GenValueScaled(gen, scaling); }, borders);
	else
		return GenValueTruncRetry([&]() {return GenValueScaled(gen, scaling); }, 10, borders);
}