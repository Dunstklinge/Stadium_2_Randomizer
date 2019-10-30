#pragma once

#include "GeneratorUtil.h"

class DiscreteDistribution
{
public:
	struct normalParams {
		double nStandardDerivations;
		double xTranslation;
	};
	struct triangleParams {
		double xPeak;
		double xTranslation;
	};

	enum Type {
		UNIFORM,
		NORMAL,
		TRIANGLE,
	};
	inline void MakeUniform() { type = UNIFORM; }
	inline void MakeNormal(normalParams np) { type = NORMAL; this->np = np; }
	inline void MakeTriangle(triangleParams tp) { type = TRIANGLE; this->tp = tp; }
public:

	DiscreteDistribution();

	DiscreteDistribution(int min, int max) 
		: left(min), right(max), type(UNIFORM) {};
	DiscreteDistribution(int min, int max, normalParams np) 
		: left(min), right(max), type(NORMAL), np(np) {}
	DiscreteDistribution(int min, int max, triangleParams tp)
		: left(min), right(max), type(TRIANGLE), tp(tp) {}

	inline int& Min() { return left; }
	inline int& Max() { return right; }
	inline Type GetType() { return type; }
	inline void SetMinMax(int min, int max) { this->left = min; this->right = max; }
	template<typename T>
	int operator()(T& gen);

	template<typename T>
	int GenValueClamp(T& gen);
	template<typename T>
	int GenValueClamp(T& gen, int min, int max);
	template<typename T>
	int GenValueRetry(T& gen, int nTries = 10);
	template<typename T>
	int GenValueRetry(T& gen, int nTries, int min, int max);


	int left;
	int right;
	Type type;
	union {
		normalParams np;
		triangleParams tp;
	};

private:
	template<typename T>
	int GenValue(T& gen);
};

template<typename T>
int DiscreteDistribution::GenValue(T& gen) {
	if (right <= left) return left;
	switch (type) {
	case NORMAL: {
		std::normal_distribution<double> dist((left + right) / 2.0, (right - left) / (2 * np.nStandardDerivations));
		return dist(gen) + np.xTranslation; }
	case TRIANGLE: {
		double center = (right + left) / 2;
		double a = left;
		double b = right;
		double c = center + tp.xPeak;
		double F = (c - a) / (b - a);
		std::uniform_real_distribution<double> udist(0, 1);
		double U = udist(gen);
		double g;
		if (U < F) {
			g = a + std::sqrt(U * (b - a) * (c - a));
		}
		else {
			g = b - std::sqrt((1 - U) * (b - a) * (b - c));
		}
		return g + tp.xTranslation; }
	case UNIFORM:
	default: {
		std::uniform_int_distribution<int> dist(left, right);
		return dist(gen); }
	}
}

template<typename T>
int DiscreteDistribution::GenValueClamp(T& gen) { return GenValueClamp(gen, left, right); }
template<typename T>
int DiscreteDistribution::GenValueClamp(T& gen, int min, int max) { 
	if (min >= max) return max;
	return std::clamp(GenValue(gen), min, max); 
}
template<typename T>
int DiscreteDistribution::GenValueRetry(T& gen, int nTries) { return GenValueRetry(gen, nTries, left, right); }
template<typename T>
int DiscreteDistribution::GenValueRetry(T& gen, int nTries, int min, int max) {
	if (min >= max) return max;
	for (int i = 0; i < nTries; i++) {
		int v = GenValue(gen);
		if (min <= v && v <= max) 
			return v;
	}
	return GenValueClamp(gen, min, max);
}

template<typename T>
int DiscreteDistribution::operator()(T& gen)
{
	return GenValueClamp(gen);
}
