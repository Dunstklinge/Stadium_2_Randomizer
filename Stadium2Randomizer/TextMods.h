#pragma once

#include "DefText.h"

#include <string>
#include <stdint.h>
#include <vector>

/*
 * records text modifications that can be applied to DefText in one swoop
 */
class TextMods
{
public:
	TextMods();
	~TextMods();

	inline void Add(const TextMods& other) {
		changes.insert(changes.end(), other.changes.begin(), other.changes.end());
	}

	void Apply(DefText* buffer, DefText* newBuffer); //newBuffer is assumed to be the size of the text segment including padding FFs at the end.
	void AddChange(uint32_t textTable, uint32_t stringNumber, std::string newString);

	struct Change {
		friend class TextMods;

		uint32_t textTable;
		uint32_t stringNumber;
		std::string replaceWith;
	};

	std::vector<Change> changes;
};

