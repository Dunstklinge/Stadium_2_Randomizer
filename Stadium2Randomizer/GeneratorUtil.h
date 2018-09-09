#pragma once

/*
 * makes a buffer of possible values out of a given filter buffer as well as a filter function.
 */
template<typename T, typename Filter>
void FilterBufferGenerate(T* outList, unsigned int& outN, 
						 const T* origBuffer, unsigned int origBufferSize, 
						 const T* buffer, unsigned int bufferN, 
					  	 Filter filter)
{
	outN = 0;

	const T* usedList;
	unsigned int usedListN;
	if (buffer) {
		usedList = buffer;
		usedListN = bufferN;
	}
	else {
		usedList = origBuffer;
		usedListN = origBufferSize;
	}

	if(filter) for (unsigned int i = 0; i < usedListN; i++) {
		if (filter(usedList[i])) {
			outList[outN++] = usedList[i];
		}
	}
}

template<typename T, typename Filter>
void FilterBufferGenerateOrDefault(
	const T** outListPtr, unsigned int* outListPtrN,
	T* outList, unsigned int& outN,
	const T* origBuffer, unsigned int origBufferSize,
	const T* filterBuffer, unsigned int filterBufferN,
	Filter* filter)
{
	outN = 0;

	const T* usedList;
	unsigned int usedListN;
	if (filterBuffer) {
		usedList = filterBuffer;
		usedListN = filterBufferN;
	}
	else {
		usedList = origBuffer;
		usedListN = origBufferSize;
	}

	if(filter) for (unsigned int i = 0; i < usedListN; i++) {
		if ((*filter)(usedList[i])) {
			outList[outN++] = usedList[i];
		}
	}

	if (outN == 0 && !filterBuffer) {
		*outListPtr = origBuffer;
		*outListPtrN = origBufferSize;
	}
	else if (outN == 0 && filterBuffer) {
		*outListPtr = filterBuffer;
		*outListPtrN = filterBufferN;
	}
	else {
		*outListPtr = outList;
		*outListPtrN = outN;
	}
}


class discrete_normal_distribution : public std::normal_distribution<double> {
	int min;
	int max;
	double nStandardDerivations;
public:
	discrete_normal_distribution(int min, int max, double nStandardDerivations)
		: normal_distribution((min + max) / 2.0, (max-min) / (2*nStandardDerivations))
	{
		this->min = min;
		this->max = max;
		this->nStandardDerivations = nStandardDerivations;
	}

	template <typename Generator>
	int operator()(Generator& gen) {
		return std::clamp((int)std::round(normal_distribution<double>::operator()(gen)), min, max);
	}
};