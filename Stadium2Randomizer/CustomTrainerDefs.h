#pragma once

#include <vector>
#include <array>
#include <string>

class CustomTrainerDefs
{
public:
	CustomTrainerDefs();
	CustomTrainerDefs(const char* trainerfolder);
	~CustomTrainerDefs();

	void AddTrainerFile(const char* filename);

	struct Face {
		std::string charName;
		std::vector<std::string> charExpressions;
	};

	struct Trainer {
		std::string name;
		std::string textLines[40];
	};
	Trainer GenPermutation(int trainerId, std::vector < std::vector < std::string >> &usedPortraitList);
	constexpr static int faceIdOffset = 67;
	

	struct TrainerData {
		struct TextLine {
			std::string line;
			int faceIndex, exprIndex;
		};
		struct LineAlternatives {
			struct LineEntry {
				int lineTableIndex;
				int chance;
			};
			std::vector<LineEntry> lines;
			int chanceTotal = 0;
		};
		
		std::vector<TextLine> stringTable; //all used strings
		LineAlternatives names;
		LineAlternatives lines[40];
	};
	std::vector<TrainerData> customTrainers;
	std::vector<Face> portraitFiles;

private:
	std::string folder;
};

