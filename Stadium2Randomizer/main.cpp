#include <iostream>
#include <fstream>
#include <Windows.h>

#include "DefRoster.h"
#include "Tables.h"
#include "GlobalRandom.h"
#include "TextMods.h"
#include "PokemonGenerator.h"
#include "TrainerGenerator.h"
#include "GlobalConfigs.h"
#include "MoveTable.h"

int main(int argc, char* argv[]) {
	GameInfo::InitTables();
	Random::Init();
	GlobalConfig::Init();

	const char* param;
	if (argc == 1) {
		param = "Pokemon Stadium 2 (U) [!].z64";
	}
	else {
		param = argv[1];
	}

	std::ifstream in(param, std::ifstream::binary);

	DefRoster* roster = DefRoster::FromFileStream(in);
	roster->Curate(true);

	//lets try randomizing little cup for now
	PokemonGenerator pokeGen;
	pokeGen.changeSpecies = false;
	pokeGen.changeEvsIvs = true;
	pokeGen.changeLevel = false;
	pokeGen.changeMoves = true;

	pokeGen.minLevel = 5;
	pokeGen.maxLevel = 5;
	pokeGen.randEvs = true;
	pokeGen.randIvs = true;

	auto rentalIt = roster->rentalBegin();
	auto& littlecupRentals = rentalIt[0];
	for (int i = 0; i < littlecupRentals.nPokemon; i++) {
		DefPokemon newMon = pokeGen.Generate(littlecupRentals.pokemon[i]);
		littlecupRentals.pokemon[i] = newMon;
	}

	//do a trainer
	TrainerGenerator tgen;
	//tgen.minOneMove = tgen.MIN_ONE_STRONG_STAB;
	tgen.usefulItem = true;
	auto trainerIt = roster->trainerBegin();
	DefTrainer newDef = tgen.Generate(trainerIt->trainers[0]);


	//try changing a name

	//get old text
	DefText* oldText = DefText::FromFileStream(in);
	oldText->Curate(true);

	//auto trainerIt = roster->trainerBegin();
	uint8_t trainerId, textId;
	trainerId = trainerIt[TableInfo::FALKNER_GYM_R1].trainers[0].trainerId;
	textId = trainerIt[TableInfo::FALKNER_GYM_R1].trainers[0].textId;
	const char* testname = "testname";

	TextMods mods;
	mods.AddChange(TableInfo::TEXT_TRAINER_TEXT_FIRST + textId, TableInfo::TRAINERTEXT_NICKNAME1, testname);
	DefText* newText = (DefText*)new uint8_t[DefText::segSize];
	mods.Apply(oldText, newText);

	newText->Curate(false);

	roster->Curate(false);

	BOOL suc = CopyFileA(param, "Pokemon Stadium 2 modified (U) [!].z64", FALSE);
	if (suc) {
		std::ofstream out("Pokemon Stadium 2 modified (U) [!].z64", std::ofstream::in | std::ofstream::out | std::ofstream::binary);
		out.seekp(DefRoster::offStart);
		out.write((char*)roster, DefRoster::offEnd - DefRoster::offStart);
		
		out.seekp(DefText::offStart);
		out.write((char*)newText, DefText::segSize);
	}



	delete[]((uint8_t*)roster);

	return 0;
}

int main2(int argc, char* argv[]) {
	GameInfo::InitTables();
	Random::Init();

	const char* param;
	if (argc == 1) {
		param = "Pokemon Stadium 2 (U) [!].z64";
	}
	else {
		param = argv[1];
	}

	DefRoster* roster = DefRoster::FromFile(param);
	roster->Curate(true);
	DefText* oldText = DefText::FromFile(param);
	oldText->Curate(true);

	//PrintAllRosterTables(roster);
	PrintAllNicknames(roster, oldText);
	PrintAllTrainerNames(roster, oldText);

	roster->Curate(false);

	delete[]((uint8_t*)roster);
	delete[]((uint8_t*)oldText);
	
	return 0;
}

void Test_PrintAllLines(CString rom) {
	std::vector<std::vector<CString>> lineStrings;
	lineStrings.resize(40);
	DefText* t = DefText::FromFile(rom);
	t->Curate(true);
	for (auto it = t->trainerTextBegin(); it != t->trainerTextEnd(); ++it) {
		for (int i = 0; i < 40 && i < it->nStrings; i++) {
			lineStrings[i].push_back((*it)[i]);
		}
	}

	std::ofstream otest("outlines.txt");
	for (int i = 0; i < 40; i++) {
		otest << "===========================\n===========================\n===========================\n" << i+1
			<< "\n===========================\n===========================\n===========================\n\n\n";
		for (auto& str : lineStrings[i])
		{
			otest << str << "\n";
		}
	}
	delete (uint8_t*)t;
}

#include "DefFaces.h"

void Test_FaceChange(CString rom) {
	DefFaces* fa = DefFaces::FromFile(rom);
	DefRoster* ro = DefRoster::FromFile(rom);
	DefText* tx = DefText::FromFile(rom);
	fa->Curate(true);
	ro->Curate(true);
	tx->Curate(true);

	struct {
		char* name;
		unsigned long faceNr;
	} gymleaders[16];
	int n = 0;
	for (auto it = ro->trainerBegin(); it != ro->trainerEnd(); ++it) {
		for (int i = 0; i < it->nTrainers; i++) {
			DefTrainer& trainer = it->trainers[i];
			if (trainer.trainerCat == GameInfo::GYM_LEADER) {
				//found gym leader
				char* name = tx->begin()[TableInfo::TEXT_TRAINER_NAMES][trainer.trainerId - 1];
				char* firstLine = tx->begin()[TableInfo::TEXT_TRAINER_TEXT_FIRST + trainer.textId][0];
				//first line starts as <FACE,XXXXXXX,N>, we want the xs
				unsigned long faceNr = 0;
				if (strncmp(firstLine, "<FACE,", sizeof("<FACE") - 1) == 0) {
					faceNr = strtoul(firstLine + 6, NULL, 10);
				}
				gymleaders[n].name = name;
				gymleaders[n].faceNr = faceNr;
				n++;
				if (n == 16) break;
			}
		}
		if (n == 16) break;
	}
	for (int i = 0; i < n; i++) {
		auto& faces = fa->begin()[gymleaders[i].faceNr];
		for (int j = 0; j < faces.nFaces; j++) {
			auto* face = faces.face(j);
			for (int k = 0; k < face->width*face->height; k++) {
				face->pixels[k] = 1 << i;
			}
		}
	}

	const TCHAR filters[] = _T("n64 rom files|*.n64;*.z64;*.v64||");
	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filters, nullptr);
	int choice = dlg.DoModal();

	fa->Curate(false);
	ro->Curate(false);
	tx->Curate(false);

	if (choice == IDOK) {
		BOOL suc = FALSE;
		if (rom == dlg.GetPathName()) {
		}
		else {
			CopyFileA(rom, dlg.GetPathName(), FALSE);
			std::ofstream out(dlg.GetPathName(), std::ofstream::in | std::ofstream::out | std::ofstream::binary);
			out.seekp(DefFaces::offStart);
			out.write((char*)fa, DefFaces::segSize);
			
		}
	}

	delete[](uint8_t*)fa;
	delete[](uint8_t*)ro;
	delete[](uint8_t*)tx;
}

void Test_ExtractAllFaces(CString rom) {
	DefFaces* fa = DefFaces::FromFile(rom);
	fa->Curate(true);

	int totalWidth = 0, totalHeight = 0;

	for (auto& v : *fa) {
		int width = 0, height = 0;
		for (int i = 0; i < v.nFaces; i++) {
			if (v.face(i)->height > height) height = v.face(i)->height;
			width += v.face(i)->width;
		}

		totalHeight += height;
		if (totalWidth < width) totalWidth = width;
	}

	struct {
		uint32_t* pixels;
		int height, width;
		uint32_t* operator[](int i) { return &pixels[i*width]; }
	} pixels;
	pixels.height = totalHeight, pixels.width = totalWidth;
	pixels.pixels = new uint32_t[totalHeight * totalWidth];
	memset(pixels.pixels, 0, totalHeight * totalWidth * sizeof(uint32_t));
	
	int yOff = 0;
	for (auto& v : *fa) {
		int totalHeight = 0;
		int xOff = 0;
		for (int i = 0; i < v.nFaces; i++) {
			auto* face = v.face(i);
			if (face->height > totalHeight) totalHeight = face->height;
			for (int y = 0; y < face->height; y++) {
				for (int x = 0; x < face->width; x++) {
					uint8_t r, g, b, a = 0;
					int xShuffle = y % 2 == 0 ? 0 : (x % 4 < 2 ? 2 : -2);
					uint16_t currPixel = face->pixels[y*face->width + x + xShuffle];
					/*
					000000	0
					000008	1
					000010	2
					000021	3
					000042	4
					000084	5

					000800	6
					001000	7
					002100	8
					004200	9
					008400	10

					080000	11
					100000	12
					210000	13
					420000	14
					840000	15
					*/
					r = currPixel >> 11 & 0x1F;
					g = currPixel >> 6 & 0x1F;
					b = currPixel >> 1 & 0x1F;

					r <<= 3, g <<= 3, b <<= 3;
					pixels[y + yOff][x + xOff] = b | g<<8 | r<<16;
				}
			}
			xOff += face->width;
		}
		yOff += totalHeight;
	}

	CBitmap bmp;
	bmp.CreateBitmap(totalWidth, totalHeight, 1, 32, pixels.pixels);
	CImage img;
	img.Attach(bmp);
	img.Save(TEXT("allfaces.bmp"), Gdiplus::ImageFormatBMP);

	delete[] pixels.pixels;
	delete[] (uint8_t*)fa;
}


void Test_ExtractTrainers(CString rom) {
	DefRoster* r = DefRoster::FromFile(rom);
	r->Curate(true);
	std::ofstream out("genTrainerList.txt");

	int cnt = 0;
	for (auto it = r->trainerBegin(); it != r->trainerEnd(); ++it, cnt++) {
		for (int i = 0; i < it->nTrainers; i++) {
			out << "(" << cnt << ", " << i << "): " << (int)it->trainers[i].trainerId
				<< ", " << (int)it->trainers[i].trainerCat << ", " << (int)it->trainers[i].textId << "\r\n";
		}
	}

	delete[](uint8_t*)r;
}


void Test_ExtractMoveEffects(CString rom) {
	DefText* text = DefText::FromFile(rom);
	text->Curate(true);
	std::ofstream out("genMoveEffectList.txt");

	auto& textIt = text->begin()[TableInfo::MOVE_NAMES];
	for (int i = 0; i < 250; i++){
		auto& move = GameInfo::Moves[i];
		out << textIt[i] << ":\t" << std::string(5 - (strlen(textIt[i])+1) / 4, '\t') 
			<< (int)move.basePower << "bp\t" << (move.basePower <= 9 ? "\t" : "") << (int)move.pp << "/" << (int)move.pp << "pp\t" 
			<< (int)move.type << "t\t"
			<< (int)move.effectId << "\t(" << (int)move.effectChance << ")\n";
	}
	delete[](uint8_t*)text;
}