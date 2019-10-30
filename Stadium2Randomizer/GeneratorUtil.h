#pragma once

#include <random>

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


