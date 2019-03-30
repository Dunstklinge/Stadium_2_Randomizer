#include "stdafx.h"
#include "CustomTrainerDefs.h"

#include <fstream>
#include <string>
#include <sstream>

#include "Constants.h"
#include "GlobalRandom.h"

CustomTrainerDefs::CustomTrainerDefs()
{
}

CustomTrainerDefs::CustomTrainerDefs(const char * trainerfolder)
{
	folder = trainerfolder;
	CFileFind finder;
	CString pattern = trainerfolder;
	pattern += TEXT("*.txt");
	BOOL suc = finder.FindFile(pattern);
	while (suc) {
		suc = finder.FindNextFile();
		if (finder.IsDirectory() || finder.IsDots()) continue;
		CString path = finder.GetFilePath();
		AddTrainerFile(path);
	}
}


CustomTrainerDefs::~CustomTrainerDefs()
{
}

void CustomTrainerDefs::AddTrainerFile(const char * filename)
{
	TrainerData t;

	std::ifstream file(filename);
	std::string line;

	std::string textLine;
	bool isName = false;
	unsigned long lineNumber = 0;
	unsigned long lineNumberMax = 0;
	unsigned long chance = 1;
	auto AddParsedData = [&]() {
		//insert string into string table
		t.stringTable.push_back({ std::move(textLine), -1, -1 });
		unsigned int stringId = t.stringTable.size() - 1;
		//insert lines, making them reference this string
		TrainerData::LineAlternatives::LineEntry entry;
		entry.chance = chance;
		entry.lineTableIndex = t.stringTable.size() - 1;
		if (isName) {
			t.names.chanceTotal += chance;
			t.names.lines.push_back(entry);
		}
		else for (int i = lineNumber; i <= lineNumberMax; i++) {
			t.lines[i - 1].chanceTotal += chance;
			t.lines[i - 1].lines.push_back(entry);
		}

		textLine.clear();
	};

	while (getline(file, line)) {
		const char* buff = line.c_str();
		const char* it = buff;

		//skip initial white spaces
		while (*it && isspace(*it)) it++;
		const char* preambleStart = it;

		if (!*preambleStart) continue; //blank line

		//find the :
		while (*it && *it != ':') it++;
		const char* preambleEnd = it;

		if (*preambleEnd) {
			//end old line
			if (textLine.size() > 0) {
				AddParsedData();
			}
			//new line

			if (strncmp(preambleStart, "NAME", sizeof("NAME") - 1) == 0) {
				isName = true;

				bool malformed = false;
				const char* it = preambleStart + 4;
				while (*it && isspace(*it)) it++;
				if (*it == ',') {
					chance = strtoul(++it, const_cast<char**>(&it), 10);
					while (*it && isspace(*it)) it++;
				}
				else chance = 1;
				if (*it != ':') malformed = true;

				if (malformed) {
					std::stringstream errorMsg;
					errorMsg << "Invalid Custom Trainer Text in file " << filename << "; line was\r\n\r\n" << line << "\r\n";
					throw std::invalid_argument(errorMsg.str());
				}
			}
			else {
				isName = false;
				const char* it;
				bool malformed = false;
				
				lineNumber = strtoul(preambleStart, const_cast<char**>(&it), 10);
				while (*it && isspace(*it)) it++;
				if (*it == '-') {
					lineNumberMax = strtoul(++it, const_cast<char**>(&it), 10);
					while (*it && isspace(*it)) it++;
				}
				else lineNumberMax = lineNumber;
				if (*it == ',') {
					chance = strtoul(++it, const_cast<char**>(&it), 10);
					while (*it && isspace(*it)) it++;
				}
				else chance = 1;
				if (*it != ':') malformed = true;
 

				if (lineNumber == 0 || lineNumber > 40 || lineNumberMax == 0 || lineNumberMax > 40 || chance == 0 || malformed) {
					std::stringstream errorMsg;
					errorMsg << "Invalid Custom Trainer Text in file " << filename << "; line was\r\n\r\n" << line << "\r\n";
					throw std::invalid_argument(errorMsg.str());
				}
			}

			it++; //skip the :
			while (*it && isspace(*it)) it++;
			const char* lineStart = it;

			textLine.append(lineStart);
		}
		else {
			//continued line
			textLine.append("\n");
			textLine.append(preambleStart);
		}
	}
	if (textLine.size() > 0) {
		AddParsedData();
	}

	//look for faces and add face data
	for (auto& line : t.stringTable) {
		if (strncmp(line.line.c_str(), "<FACE,", sizeof("<FACE,") - 1) == 0) {
			//get face parameters 
			const char* it = line.line.c_str();
			it += 6;
			const char* charNameBegin = it;
			while (*it && *it != ',') it++;
			const char* charNameEnd = it;

			if (*it) it++;
			const char* portraitNameBegin = it;
			while (*it && *it != '>') it++;
			const char* portraitNameEnd = it;

			std::string charName = std::string(charNameBegin, charNameEnd);
			std::string charExpr = std::string(portraitNameBegin, portraitNameEnd);
			std::string portraitPath = folder + "portraits\\" + charName + "_" + charExpr + ".png";

			//check if face image actually exists
			if (!std::ifstream(portraitPath).good()) {
				std::stringstream errorMsg;
				errorMsg << "Could not open portrait File " << portraitPath << "; requested from file " << filename << ", line " << line.line << "\r\n";
				throw std::invalid_argument(errorMsg.str());
			}

			//add face image to the list
			auto charExists = std::find_if(portraitFiles.begin(), portraitFiles.end(), [&](const Face& f) { return f.charName == charName; });
			if (charExists == portraitFiles.end()) {
				Face f;
				f.charName = std::move(charName);
				f.charExpressions.push_back(std::move(charExpr));
				portraitFiles.push_back(std::move(f));
				line.faceIndex = portraitFiles.size() - 1;
				line.exprIndex = 0;
			}
			else {
				line.faceIndex = charExists - portraitFiles.begin();
				auto& expressions = charExists->charExpressions;
				auto charExprExists = std::find_if(expressions.begin(), expressions.end(), [&](const std::string& expr) { return expr == charExpr; });
				if (charExprExists == expressions.end()) {
					expressions.push_back(std::move(charExpr));
					line.exprIndex = expressions.size()-1;
				}
				else {
					line.exprIndex = charExprExists - expressions.begin();
				}
			}
		}
		
	}

	customTrainers.push_back(t);
}

CustomTrainerDefs::Trainer CustomTrainerDefs::GenPermutation(int trainerId, std::vector<std::vector<std::string>>& usedPortraitList)
{
	Trainer retVal;
	TrainerData& data = customTrainers[trainerId];

	TrainerData::LineAlternatives nameAltTempCopy = data.names;
	TrainerData::LineAlternatives lineAltTempCopy[40];
	for (int i = 0; i < 40; i++) lineAltTempCopy[i] = data.lines[i];

	auto SelectRandomLine = [&](TrainerData::LineAlternatives& alts) -> TrainerData::TextLine* {
		if (alts.lines.size() == 0) return nullptr;
		int randomInt = Random::GetInt(1, alts.chanceTotal);
		for (auto& line : alts.lines) {
			if (randomInt <= line.chance) {
				TrainerData::TextLine* retVal = &data.stringTable[line.lineTableIndex];
				//remove used line from list
				alts.chanceTotal -= line.chance;
				std::swap(line, alts.lines.back());
				alts.lines.resize(alts.lines.size() - 1);
				return retVal;
			}
			else randomInt -= line.chance;
		}
		return nullptr;
	};

	//assign random name
	if (data.names.lines.size() > 0) retVal.name = SelectRandomLine(nameAltTempCopy)->line;
	//assign random lines
	for (int i = 0; i < 40; i++) {
		TrainerData::TextLine* selectedLine = SelectRandomLine(lineAltTempCopy[i]);
		if (selectedLine) {
			retVal.textLines[i] = selectedLine->line;

			//add face portrait used by this line to usedPortraitList (if it isnt allready used)
			const std::string& usedChar = portraitFiles[selectedLine->faceIndex].charName;
			const std::string& usedExpr = portraitFiles[selectedLine->faceIndex].charExpressions[selectedLine->exprIndex];
			std::string usedPortraitPath = folder + "portraits\\" + usedChar + "_" + usedExpr + ".png";
			unsigned int usedPortraitCharPartEnd = usedPortraitPath.size() - usedExpr.size() - 4 - 1; //index of the _
			int charId;
			int exprId;
			auto charIt = std::find_if(usedPortraitList.begin(), usedPortraitList.end(),
				[&](const std::vector<std::string>& vec) { return strncmp(vec[0].c_str(), usedPortraitPath.c_str(), usedPortraitCharPartEnd) == 0 && 
																	*(vec[0].c_str() + usedPortraitCharPartEnd) == '_'; }
			);
			if (charIt == usedPortraitList.end()) {
				std::vector<std::string> exprList;
				exprList.push_back(usedPortraitPath);
				usedPortraitList.push_back(std::move(exprList));
				charId = usedPortraitList.size() - 1;
				exprId = 0;
			}
			else {
				charId = charIt - usedPortraitList.begin();
				auto exprIt = std::find_if(charIt->begin(), charIt->end(), [&](const std::string& path) { return path == usedPortraitPath; });
				if (exprIt == charIt->end()) {
					charIt->push_back(usedPortraitPath);
					exprId = charIt->size() - 1;
				}
				else {
					exprId = exprIt - charIt->begin();
				}
			}
			//modify line to reference these ids
			std::string& sline = retVal.textLines[i];
			if (strncmp(sline.c_str(), "<FACE,", sizeof("<FACE,") - 1) == 0) {
				int faceEnd = 6;
				while (faceEnd < sline.size() && sline[faceEnd] != '>') faceEnd++;
				faceEnd++;
				if (faceEnd <= sline.size()) {
					std::string newFace = "<FACE," + std::to_string(charId + faceIdOffset) + "," + std::to_string(exprId) + ">";
					if (faceEnd < sline.size()) {
						sline.replace(sline.begin(), sline.begin() + faceEnd, newFace);
					}
					else {
						sline = std::move(newFace);
					}
					
				}
			}
			
		}
	}

	return retVal;
}
