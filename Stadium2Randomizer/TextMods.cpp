#include "TextMods.h"

#include <algorithm>

TextMods::TextMods()
{
}


TextMods::~TextMods()
{
}

void TextMods::Apply(DefText * buffer, DefText* newBuffer)
{
	uint8_t* byteBuffer = (uint8_t*)buffer, *newByteBuffer = (uint8_t*)newBuffer;
	//sort changes first
	std::sort(changes.begin(), changes.end(), [](Change lhs, Change rhs) {
		return lhs.textTable < rhs.textTable || lhs.textTable == rhs.textTable && lhs.stringNumber < rhs.stringNumber;
	});

	auto it = buffer->begin();
	auto cIt = changes.begin();

	//copy table infos. note that the table offsets have to be corrected afterwards
	uint32_t dataOffset = 0x10 + buffer->tables.nTables * sizeof buffer->tables.subtableInfos[0];
	memcpy(newBuffer, buffer, dataOffset);

	uint32_t currNewBufferOffset = dataOffset;

	uint32_t postLastModTable = 0;
	int32_t totalOffset = 0;

	while (true) {
		//find next table to modify. if no more mods are there, we musnt break, but still copy the rest, so we pretend theres a mod past the last table
		uint32_t nextMod = cIt == changes.end() ? buffer->tables.nTables : cIt->textTable;

		//copy all tables inbetween. Note that these tables are untouched and thus adhere to alignment rules allready.
		//copy from start of first table that wasnt looked at yet to start of mod table, or end of segment if no more edits are present
		uint32_t copyStart = postLastModTable >= buffer->tables.nTables ? 
			buffer->tables.subtableInfos[buffer->tables.nTables - 1].tableOffset + buffer->tables.subtableInfos[buffer->tables.nTables - 1].tableSize :
			buffer->tables.subtableInfos[postLastModTable].tableOffset; 
		uint32_t copyEnd = nextMod >= buffer->tables.nTables ? 
			buffer->tables.subtableInfos[buffer->tables.nTables - 1].tableOffset  + buffer->tables.subtableInfos[buffer->tables.nTables-1].tableSize :
			buffer->tables.subtableInfos[nextMod].tableOffset;
		uint32_t copySize = copyEnd - copyStart;
		//make sure we dont copy to outside of the buffer
		if (currNewBufferOffset + copySize > DefText::segSize) {
			throw std::length_error(std::string("Modifcations exceeded length of text segment. total:") + std::to_string(currNewBufferOffset + copySize));
		}
		memcpy(newByteBuffer + currNewBufferOffset, byteBuffer + copyStart, copySize);
		currNewBufferOffset += copySize;

		//correct the offsets in offset table if previous changes made a difference here
		if (totalOffset != 0) for (int i = postLastModTable; i != nextMod; i++) {
			newBuffer->tables.subtableInfos[i].tableOffset += totalOffset;
		}

		//if mod was actually past table, we abort here
		if (cIt == changes.end()) break;

		//apply changes to this table: basically do the same again, but for string list
		DefText::StringList* strTable = &buffer->begin()[nextMod];
		DefText::StringList* newStrTable = ((DefText::StringList*)(newByteBuffer + currNewBufferOffset));
		uint32_t dataOffset = 4 + strTable->nStrings * sizeof strTable->stringOffsets[0];
		memcpy(newStrTable, strTable, dataOffset);
		uint32_t currNewStringOffset = dataOffset;
		uint32_t postLastModString = 0;
		int32_t totalStringOffset = 0;
		while (true) {
			//this is a copy pasta of code above, adjusted to the scale of the string table, so thats its mostly agnostic about the table its in.
			//should probably parameterize this so i dont have to copy pasta
			uint32_t nextStringMod = cIt == changes.end() || cIt->textTable != nextMod ? strTable->nStrings : cIt->stringNumber;
			uint32_t copyStart = postLastModString >= strTable->nStrings ? 
											strTable->stringOffsets[strTable->nStrings - 1] + strlen((*strTable)[strTable->nStrings - 1]) + 1 :
											strTable->stringOffsets[postLastModString];
			uint32_t copyEnd = nextStringMod >= strTable->nStrings ? 
											strTable->stringOffsets[strTable->nStrings-1] + strlen((*strTable)[strTable->nStrings-1]) + 1 : 
											strTable->stringOffsets[nextStringMod];
			uint32_t copySize = copyEnd - copyStart;
			if (currNewBufferOffset + currNewStringOffset + copySize > DefText::segSize) {
				throw std::length_error(std::string("Modifcations exceeded length of text segment. total:") + std::to_string(currNewBufferOffset + copySize));
			}
			memcpy(((uint8_t*)newStrTable) + currNewStringOffset, ((uint8_t*)strTable) + copyStart, copySize);
			currNewStringOffset += copySize;
			if (totalStringOffset != 0) for (int i = postLastModString; i != nextStringMod; i++) {
				newStrTable->stringOffsets[i] += totalStringOffset;
			}
			if (cIt == changes.end() || cIt->textTable != nextMod) break;
			//we are now at nextStringMod, so that currNewStringOffset points to where that string should be.
			//we just copy the change buffer instead. make sure to keep track of size differences.
			int32_t oldLength = strlen((*strTable)[nextStringMod]) + 1;
			int32_t newLength = cIt->replaceWith.size() + 1;
			memcpy(((uint8_t*)newStrTable) + currNewStringOffset, cIt->replaceWith.c_str(), newLength);
			newStrTable->stringOffsets[nextStringMod] = currNewStringOffset;
			currNewStringOffset += newLength;

			totalStringOffset += (newLength - oldLength);
			postLastModString = nextStringMod + 1;
			//skip duplicated entries 
			auto origIt = cIt;
			do {
				++cIt;
			} while (cIt != changes.end() && cIt->stringNumber == origIt->stringNumber && cIt->textTable == origIt->textTable);
		}

		//now that the table has been written, align it to next boundary; alignment is so that the lowest nibble of the address must be 0
		uint32_t currTableLength = currNewStringOffset;
		uint32_t alignedTableLength = (currTableLength + 0xF) & ~0xF;
		memset(newByteBuffer + currNewBufferOffset + currTableLength, 0xFF, alignedTableLength - currTableLength);

		//set table offset, table length, then adjust totalOffset
		newBuffer->tables.subtableInfos[nextMod].tableOffset = currNewBufferOffset;
		newBuffer->tables.subtableInfos[nextMod].tableSize = alignedTableLength;
		totalOffset += (alignedTableLength - buffer->tables.subtableInfos[nextMod].tableSize); //if we are bigger, others have to be moved lower, thus offset > 0

		currNewBufferOffset += alignedTableLength;
		postLastModTable = nextMod + 1;
	}
	

}

void TextMods::AddChange(uint32_t textTable, uint32_t stringNumber, std::string newString)
{
	Change c;
	c.textTable = textTable;
	c.stringNumber = stringNumber;
	c.replaceWith = newString;
	changes.push_back(c);
}
