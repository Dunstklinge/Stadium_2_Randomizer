#include "stdafx.h"
#include "Randomizer.h"

#include<iostream>
#include <sstream>

#include "CMainDialog.h"
#include "Crc.h"
#include "CustomRosterInfo.h"
#include "CustomCode.h"
#include "Filters.h"
#include "PokemonGenerator.h"
#include "TrainerGenerator.h"
#include "GlobalRandom.h"
#include "GlobalConfigs.h"
#include "DefFaces.h"


Randomizer::Randomizer(CMainDialog* settings)
{
	m_settings = settings;
	m_romRoster = nullptr;
	m_romText = nullptr;
}


Randomizer::~Randomizer()
{
	delete[](uint8_t*)m_romRoster, m_romRoster = nullptr;
	delete[](uint8_t*)m_romText, m_romText= nullptr;
	delete[] m_customItemRedirectCode, m_customItemRedirectCode = nullptr;
	delete[] m_customItemInjectCode, m_customItemInjectCode = nullptr;
	delete[] m_customRentalRedirectCode, m_customRentalRedirectCode = nullptr;
	delete[] m_customRentalInjectCode, m_customRentalInjectCode = nullptr;
}

void Randomizer::Randomize(const CString& path, CMainDialog * settings)
{
	Randomizer r(settings);
	r.RandomizeRom(path);
}




void Randomizer::RandomizeRom(const CString & path)
{
	//setup general data
	m_genLog.open("genLog.txt");						//logger

	CString strSeed;
	m_settings->edSeed.GetWindowText(strSeed);
	m_genLog << "Generating with seed " << strSeed << "\n";

	m_in.open(path, std::ifstream::binary);				//input file

	m_settings->progressBar.GetRange(m_progressBarMin, m_progressBarMax);

	AnalyseRom();										//rom type					

	m_cupRules = GenerateCupRules();					//cup rules

	char dwordContainingRegion[4];
	m_in.seekg(0x3C);
	m_in.read(dwordContainingRegion, 4);
	DoSwaps(dwordContainingRegion, 4);
	m_romRegion = (dwordContainingRegion)[2];			//region (Us, Europe)


	//setup parts to be written
	m_romRoster = DefRoster::FromFileStream(m_in);
	DoSwaps(m_romRoster, DefRoster::segSize);
	m_romRoster->Curate(true);
	m_romText = DefText::FromFileStream(m_in);
	DoSwaps(m_romText, DefText::segSize);
	m_romText->Curate(true);
	m_customRInfoTable.clear();
	m_customRentalTables.clear();
	m_customIInfoTable.clear();
	m_customItemTables.clear();
	m_customFaceTable.clear();
	m_customTrainers.clear();

	//
	// randomize data
	//

	m_progressPartMinPercent = 0.0;
	m_progressPartMaxPercent = 0.1;
	RandomizeRegularRentals();

	m_progressPartMinPercent = 0.1;
	m_progressPartMaxPercent = 0.5;
	RandomizeHackedRentals();

	m_progressPartMinPercent = 0.5;
	m_progressPartMaxPercent = 0.9;
	RandomizeTrainers();

	m_progressPartMinPercent = 0.9;
	m_progressPartMaxPercent = 0.9;
	RandomizeItems();

	m_progressPartMinPercent = 0.9;
	m_progressPartMaxPercent = 1.0;
	SortInjectedData();

	//
	// save generated data
	//
	

	const TCHAR filters[] = _T("n64 rom files|*.n64;*.z64;*.v64||");
	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filters, m_settings);
	int choice = dlg.DoModal();

	m_in.close();
	if (choice == IDOK) {
		BOOL suc = FALSE;
		if (path == dlg.GetPathName()) {
			//overwrite original
			suc = TRUE;
		}
		else {
			suc = CopyFileA(path, dlg.GetPathName(), FALSE);
		}

		if (suc) {
			std::ofstream out(dlg.GetPathName(),std::ofstream::in | std::ofstream::out | std::ofstream::binary);
			for (auto& rep : m_romReplacements) {
				out.seekp(rep.romOffset);
				out.write((char*)rep.buffer, rep.bufferSize);
			}
			out.close();

			if (m_needsNewCrc) {
				uint8_t* checksumPartBuffer = new uint8_t[CHECKSUM_END];
				m_in.open(dlg.GetPathName(), std::ifstream::binary);
				m_in.seekg(0);
				m_in.read((char*)checksumPartBuffer, CHECKSUM_END);
				DoSwaps(checksumPartBuffer, CHECKSUM_END);

				uint32_t newCrcs[2];
				bool crcSuc = CalculateCrcs(newCrcs, checksumPartBuffer);
				if (crcSuc) {
					SwitchEndianness(newCrcs[0]);
					SwitchEndianness(newCrcs[1]);
					//crcs are at 0x10 and 0x14
					*((uint32_t*)(checksumPartBuffer + 0x10)) = newCrcs[0];
					*((uint32_t*)(checksumPartBuffer + 0x14)) = newCrcs[1];
				}

				DoSwaps(checksumPartBuffer, CHECKSUM_END);

				m_in.close();
				out.open(dlg.GetPathName(), std::ofstream::in | std::ofstream::out | std::ofstream::binary);
				out.seekp(0);
				out.write((char*)checksumPartBuffer, CHECKSUM_END);
				out.close();

				delete[] checksumPartBuffer;
			}
		}
	}

	delete[]((uint8_t*)m_romText), m_romText = nullptr;
	delete[]((uint8_t*)m_romRoster), m_romRoster = nullptr;

	SetProgress(0);
}

void Randomizer::SetProgress(double percent)
{
	m_settings->progressBar.SetPos((int)((m_progressBarMax - m_progressBarMin) * percent + m_progressBarMin));
}

void Randomizer::SetPartialProgress(double percent)
{
	double partPercent = m_progressPartMinPercent + (m_progressPartMaxPercent - m_progressPartMinPercent) * percent;
	SetProgress(partPercent);
}

void Randomizer::DoSwaps(void * buffer, size_t size)
{
	uint8_t* bBuffer = (uint8_t*)buffer;
	if (m_normal) return;
	else if (m_byteswapped) {
		if (size % 2 != 0) throw std::domain_error("Tried to Byteswap buffer of unaligned size");
		for (size_t i = 0; i < size; i += 2) SwitchEndianness(*((uint16_t*)(bBuffer + i)));
	}
	else if (m_wordswapped) {
		if (size % 4 != 0) throw std::domain_error("Tried to Wordswap buffer of unaligned size");
		for (size_t i = 0; i < size; i += 4) SwitchEndianness(*((uint32_t*)(bBuffer + i)));
	}
}


bool Randomizer::AnalyseRom()
{
	//there seem to be 3 formats: normal, byteswapped (read in as 16 bit little endian) and wordswapped
	//(confusingly just called little endian, which makes 0 sense because the rom has no idea about the semantics
	// of values)
	//the first 4 bytes of the header are 80 37 12 40. therefor:
	//if the first byte is 0x80, we are normal
	//if the first byte is 0x37, we are byteswapped
	//if the first byte is 0x40, its wordswapped
	uint8_t firstByte;
	m_in.read((char*)&firstByte, 1);

	m_normal = firstByte == 0x80, m_byteswapped = firstByte == 0x37, m_wordswapped = firstByte == 0x40;
	auto DoSwaps = [&](void* buffer, size_t size) {
		uint8_t* bBuffer = (uint8_t*)buffer;
		if (m_normal) return false;
		else if (m_byteswapped) for (size_t i = 0; i < size; i += 2) SwitchEndianness(*(uint16_t*)(bBuffer + i));
		else if (m_wordswapped) for (size_t i = 0; i < size; i += 4) SwitchEndianness(*(uint32_t*)(bBuffer + i));
	};
	return true;
}

Randomizer::CupRules Randomizer::GenerateCupRules()
{
	int primecupLevel = _tstoi(m_settings->strPrimeCupLevel);
	int glcLevel = m_settings->changeGlcRentalLevel ? _tstoi(m_settings->seperateGlcRentalsLevel) : 100;
	return CupRules {
		CupRule{50, 55, 155, nullptr, PokecupLegalMons, _countof(PokecupLegalMons)},
		CupRule{5, 5, 999, &FilterOutLittlecupMoves, LittlecupLegalMons, _countof(LittlecupLegalMons)},
		CupRule{m_settings->randPrimecupLevels ? primecupLevel : 100, 100, 999},  //poke, little, prime/gym

		CupRule{30,30,999, nullptr, ChallengecupLegalMonsPokeball, _countof(ChallengecupLegalMonsPokeball)}, //challenge cup 1,2,3,4
		CupRule{45,45,999, nullptr, ChallengecupLegalMonsGreatball, _countof(ChallengecupLegalMonsGreatball)},
		CupRule{60,60,999, nullptr, ChallengecupLegalMonsUltraball, _countof(ChallengecupLegalMonsUltraball)},
		CupRule{75,75,999, nullptr, ChallengecupLegalMonsMasterball, _countof(ChallengecupLegalMonsMasterball)},

		CupRule{glcLevel,glcLevel,999} //glc,
	};
}

void Randomizer::RandomizeRegularRentals()
{

	/*0: Little Cup rentals, offset 490 (#1168)
	1: Prime Cup/Gym leaders castle/Free battle rentals, offset CB0 (#3248)
	2: Prime Cup round 2 (same but with celeby), offset 23D0 (#9168)
	3: Poke Cup, offset 3B20 (#15136)
	4: Poke Cup again (same exact table, referenced twice - maybe this was supposed to be Poke Cup r2?)*/
	auto rentalIt = m_romRoster->rentalBegin();
	PokemonGenerator pokeGen;
	pokeGen.changeSpecies = false;
	pokeGen.changeEvsIvs = true;
	pokeGen.changeLevel = true;
	pokeGen.changeMoves = true;
	pokeGen.changeItem = false;

	pokeGen.randEvs = m_settings->randEvIv;
	pokeGen.randIvs = m_settings->randEvIv;
	pokeGen.bstEvIvs = m_settings->rentalSpeciesEvIv;
	pokeGen.statsUniformDistribution = m_settings->randEvIvUniformDist;
	pokeGen.levelUniformDistribution = m_settings->randLevelsUniformDist;

	{
		using namespace std::placeholders;
		if (m_settings->min1Buttons == 1) pokeGen.minOneMoveFilter = std::bind(&FilterMoveByBP, _1, 1, 999);
		else if (m_settings->min1Buttons == 2) pokeGen.minOneMoveFilter = std::bind(&FilterMoveByBP, _1, 75, 999);
		else if (m_settings->min1Buttons == 3) pokeGen.minOneMoveFilter = &FilterMoveByStab;
		else if (m_settings->min1Buttons == 4) pokeGen.minOneMoveFilter =
			std::bind(std::logical_and<bool>(), std::bind(&FilterMoveByBP, _1, 75, 999), std::bind(&FilterMoveByStab, _1, _2));
		else pokeGen.minOneMoveFilter = nullptr;
	}
	auto randomizeRentals = [&](int cupIndex) {
		for (int i = 0; i < rentalIt[cupIndex].nPokemon; i++) {
			DefPokemon newMon = pokeGen.Generate(rentalIt[cupIndex].pokemon[i]);
			if (m_settings->min1Buttons == 5) { newMon.move1 = newMon.move2 = newMon.move3 = newMon.move4 = GameInfo::METRONOME; }
			rentalIt[cupIndex].pokemon[i] = newMon;
			newMon.Print(m_genLog);
			m_genLog << "\n";
		}
	};
	m_genLog << "Randomizing rentals...\n";

	m_genLog << "Littlecup:\n";
	//little cup
	pokeGen.minLevel = 5;
	pokeGen.maxLevel = 5;
	pokeGen.generalMoveFilter = m_cupRules[LITTLECUP].legalMoveFilter;
	if (m_settings->legalMovesOnly) {
		using namespace std::placeholders;
		pokeGen.generalMoveFilter = std::bind(std::logical_and<bool>(),
			std::bind(pokeGen.generalMoveFilter, _1, _2),
			std::bind(FilterLegalMovesOnly, _1, _2));
	}
	pokeGen.speciesFilterBuffer = m_cupRules[LITTLECUP].legalMonList;
	pokeGen.speciesFilterBufferN = m_cupRules[LITTLECUP].legalMonListN;
	randomizeRentals(0);

	SetPartialProgress(0.2);

	m_genLog << "Prime Cup:\n";
	//prime cup
	int primecupLevel = _tstoi(m_settings->strPrimeCupLevel);
	pokeGen.ClearAllFilters();
	if (m_settings->legalMovesOnly)
		pokeGen.generalMoveFilter = FilterLegalMovesOnly;
	pokeGen.minLevel = m_settings->randPrimecupLevels ? primecupLevel : 100;
	pokeGen.maxLevel = 100;
	randomizeRentals(1);

	SetPartialProgress(0.5);

	//prime cup r2
	m_genLog << "Prime Cup R2:\n";
	randomizeRentals(2);

	SetPartialProgress(0.75);

	//poke cup
	m_genLog << "Poke Cup:\n";
	pokeGen.ClearAllFilters();
	if (m_settings->legalMovesOnly)
		pokeGen.generalMoveFilter = FilterLegalMovesOnly;
	pokeGen.speciesFilterBuffer = m_cupRules[POKECUP].legalMonList;
	pokeGen.speciesFilterBufferN = m_cupRules[POKECUP].legalMonListN;
	pokeGen.minLevel = 50;
	pokeGen.maxLevel = m_settings->randLevels ? 55 : 50;
	randomizeRentals(3);

	SetPartialProgress(1);

	m_genLog << "... generating rentals done!\n";
}

void Randomizer::RandomizeHackedRentals()
{
	if (!m_settings->moreRentalTables) return;

	PokemonGenerator pokeGen;
	pokeGen.changeSpecies = false;
	pokeGen.changeEvsIvs = true;
	pokeGen.changeLevel = true;
	pokeGen.changeMoves = true;
	pokeGen.changeItem = false;

	pokeGen.randEvs = m_settings->randEvIv;
	pokeGen.randIvs = m_settings->randEvIv;
	pokeGen.bstEvIvs = m_settings->rentalSpeciesEvIv;
	pokeGen.statsUniformDistribution = m_settings->randEvIvUniformDist;
	pokeGen.levelUniformDistribution = m_settings->randLevelsUniformDist;

	{
		using namespace std::placeholders;
		if (m_settings->min1Buttons == 1) pokeGen.minOneMoveFilter = std::bind(&FilterMoveByBP, _1, 1, 999);
		else if (m_settings->min1Buttons == 2) pokeGen.minOneMoveFilter = std::bind(&FilterMoveByBP, _1, 75, 999);
		else if (m_settings->min1Buttons == 3) pokeGen.minOneMoveFilter = &FilterMoveByStab;
		else if (m_settings->min1Buttons == 4) pokeGen.minOneMoveFilter =
			std::bind(std::logical_and<bool>(), std::bind(&FilterMoveByBP, _1, 75, 999), std::bind(&FilterMoveByStab, _1, _2));
		else pokeGen.minOneMoveFilter = nullptr;
	}

	if (m_settings->legalMovesOnly) 
		pokeGen.generalMoveFilter = FilterLegalMovesOnly;	

	const auto tid = [](int id) {return id + 5; };
	const auto AddRentalSet = [&](int tidBorder, int rentalIndex) {
		//replace the existing one if this exact one was allready added
		if (m_customRInfoTable.size() > 0 && m_customRInfoTable.back().fightMin == tidBorder) {
			m_customRInfoTable.back() = CustomRosterInfo{ (uint16_t)tidBorder,rentalIndex,0 };
		}
		else {
			m_customRInfoTable.emplace_back(CustomRosterInfo{ (uint16_t)tidBorder,rentalIndex,0 });
		}
		
		return m_customRInfoTable.size() - 1;
	};
	const auto GenRentalSetAndAdd = [&](CupRulesId cup) {
		auto& table = m_customRentalTables.emplace_back();

		pokeGen.minLevel = m_cupRules[cup].minLevel;
		pokeGen.maxLevel = m_cupRules[cup].maxLevel;

		int nMons = m_cupRules[cup].legalMonListN;
		if (nMons == 0) nMons = 251;
		
		table.resize(nMons * sizeof(DefPokemon) + 4);
		if (nMons > 255) throw std::invalid_argument("Tried to generate a rental table with more than 255 pokemon, which is maximum allowed size");
		memset(table.data(), 0, table.size());
		table[0] = nMons;
		table[1] = 0;
		table[2] = 0;
		table[3] = 0;

		m_genLog << "generating rental table with " << nMons << " pokemon...\n";
		
		for (unsigned int i = 0; i < nMons; i++) {
			DefPokemon newMon;
			newMon.species = m_cupRules[cup].legalMonListN == 0 ? (GameInfo::PokemonId)(i + 1) : m_cupRules[cup].legalMonList[i];
			newMon.item = GameInfo::NO_ITEM;
			newMon = pokeGen.Generate(newMon);
			if (m_settings->min1Buttons == 5) { newMon.move1 = newMon.move2 = newMon.move3 = newMon.move4 = GameInfo::METRONOME; }
			*(DefPokemon*)&(table[i * sizeof(DefPokemon) + 4]) = newMon;
			newMon.Print(m_genLog);
			m_genLog << "\n";
		}
		return m_customRentalTables.size() - 1;
	};

	m_genLog << "generating additional rental sets...\n";
	//0 and 1 are unknown and never covered anyway
	AddRentalSet(0,-1);

	if (m_settings->multiplePokecupRentals) {
		//2 allready has its default rental table, so we make 3,4,5
		AddRentalSet(tid(3), GenRentalSetAndAdd(POKECUP));
		AddRentalSet(tid(4), GenRentalSetAndAdd(POKECUP));
		AddRentalSet(tid(5), GenRentalSetAndAdd(POKECUP));
		AddRentalSet(tid(6),-1); //end it
	}
	SetPartialProgress(0.2);
	if (m_settings->multipleGlcRentals) {
		AddRentalSet(tid(8), GenRentalSetAndAdd(GLC)); //start at falkner
		AddRentalSet(tid(13), GenRentalSetAndAdd(GLC));//new rentals from jasmine
		AddRentalSet(tid(19), GenRentalSetAndAdd(GLC));//new rentals after elite 4
		AddRentalSet(tid(29),-1); //end it after rival
	}
	SetPartialProgress(0.4);
	if (m_settings->multipleR2Rentals) {
		if (m_settings->multiplePokecupRentals) {
			//pokecups, this time all 4
			AddRentalSet(tid(29), GenRentalSetAndAdd(POKECUP));
			AddRentalSet(tid(30), GenRentalSetAndAdd(POKECUP));
			AddRentalSet(tid(31), GenRentalSetAndAdd(POKECUP));
			AddRentalSet(tid(32), GenRentalSetAndAdd(POKECUP));
			//littlecup
			AddRentalSet(tid(33), GenRentalSetAndAdd(LITTLECUP));
			//primecup r2 has its own allready, so no need for it
			AddRentalSet(tid(34),-1); //end it
		}

		SetPartialProgress(0.7);

		if (m_settings->multipleGlcRentals) {
			AddRentalSet(tid(35), GenRentalSetAndAdd(GLC)); //start at falkner
			AddRentalSet(tid(40), GenRentalSetAndAdd(GLC));//new rentals from jasmine
			AddRentalSet(tid(46), GenRentalSetAndAdd(GLC));//new rentals after elite 4
			AddRentalSet(tid(56),-1); //end it after rival
		}
	}
	SetPartialProgress(0.9);

	std::sort(m_customRInfoTable.begin(), m_customRInfoTable.end(), [](CustomRosterInfo& lhs, CustomRosterInfo& rhs) -> bool { return lhs.fightMin > rhs.fightMin; });
	
	SetPartialProgress(1.0);

	m_genLog << "generating additional rental sets done!\n";
}

//todo: split this up into multiple functions, its unreadably long
void Randomizer::RandomizeTrainers()
{
	m_genLog << "Randomizing Trainers...:\n";

	//rule map; see FormatNotes
	struct {
		const CupRule* rule;
		int r; //0 for round 1, 1 for round 2
		const CupRule& operator*() const { return *rule; }
		const CupRule* operator->() const { return rule; }

	} ruleMap[62];
	for (int i = 2; i <= 5; i++) ruleMap[i - 2] = { &m_cupRules[POKECUP], 0 };
	for (int i = 6; i <= 6; i++) ruleMap[i - 2] = { &m_cupRules[LITTLECUP], 0 };
	for (int i = 7; i <= 7; i++) ruleMap[i - 2] = { &m_cupRules[PRIMECUP], 0 };
	for (int i = 8; i <= 28; i++) ruleMap[i - 2] = { &m_cupRules[GLC], 0 };

	for (int i = 29; i <= 32; i++) ruleMap[i - 2] = { &m_cupRules[POKECUP], 1 };
	for (int i = 33; i <= 33; i++) ruleMap[i - 2] = { &m_cupRules[LITTLECUP], 1 };
	for (int i = 34; i <= 34; i++) ruleMap[i - 2] = { &m_cupRules[PRIMECUP], 1 };
	for (int i = 35; i <= 55; i++) ruleMap[i - 2] = { &m_cupRules[GLC], 1 };

	for (int i = 56; i <= 56; i++) ruleMap[i - 2] = { &m_cupRules[CHALLENGECUP_1], 0 };
	for (int i = 57; i <= 57; i++) ruleMap[i - 2] = { &m_cupRules[CHALLENGECUP_2], 0 };
	for (int i = 58; i <= 58; i++) ruleMap[i - 2] = { &m_cupRules[CHALLENGECUP_3], 0 };
	for (int i = 59; i <= 59; i++) ruleMap[i - 2] = { &m_cupRules[CHALLENGECUP_4], 0 };

	for (int i = 60; i <= 60; i++) ruleMap[i - 2] = { &m_cupRules[CHALLENGECUP_1], 1 };
	for (int i = 61; i <= 61; i++) ruleMap[i - 2] = { &m_cupRules[CHALLENGECUP_2], 1 };
	for (int i = 62; i <= 62; i++) ruleMap[i - 2] = { &m_cupRules[CHALLENGECUP_3], 1 };
	for (int i = 63; i <= 63; i++) ruleMap[i - 2] = { &m_cupRules[CHALLENGECUP_4], 1 };

	//
	//collect shuffle candidates
	//

	//data structures to collect them into
	struct ShuffleData {
		struct tmp {
			uint32_t id;
			uint32_t cat;
			uint32_t text;

			tmp(const DefTrainer& t) : id(t.trainerId), cat(t.trainerCat), text(t.textId) {  }
			bool operator<(const tmp& rhs) const { return id < rhs.id || id == rhs.id && (cat < rhs.cat || cat == rhs.cat && (text < rhs.text)); }
			bool operator==(const tmp& rhs) const { return id == rhs.id && cat == rhs.cat && text == rhs.text; }
		};
		std::vector<tmp> trainers;
		unsigned int trainersNBenched = 0;	//the last N elements will still be considered to see if this trainer can be randomized, but is not a candidate anymore
		std::vector<tmp> bosses;
		unsigned int bossesNBenched = 0;
		
	};
	struct CustomShuffleInfo {
		uint32_t trainerId;
		std::vector<DefTrainer*> trainersStructs;
	};
	std::vector<CustomShuffleInfo> shuffledIds;
	std::vector<int> customTrainersUsed;

	ShuffleData shuffle[2]; //for round 1 and round 2

	std::vector<std::vector<std::string>> customFaceList;
	TextMods customTrainerTextMods;

	
	//actual collection
	if(m_settings->shuffleRegulars || m_settings->shuffleBosses) {
		auto it = m_romRoster->trainerBegin() + 2;
		for (int i = 2; i <= 63; i++, ++it) {
			for (int j = 0; j < it->nTrainers; j++) {
				auto& trainer = it->trainers[j];
				bool isBoss = false;
				//for the purposes of shuffling, only gym leaders and rivals count as bosses
				switch (trainer.trainerCat) {
				case GameInfo::GYM_LEADER:
				case GameInfo::ELITE_FOUR:
				case GameInfo::CHAMPION:
				case GameInfo::RIVAL:
				case GameInfo::RIVAL17:
					isBoss = true;
					break;
				case GameInfo::POKEMON_TRAINER:
					isBoss = trainer.trainerId - 1 == TableInfo::RED;
					break;
				default:
					break;
				}
				if (!isBoss && m_settings->shuffleRegulars || isBoss && m_settings->shuffleBosses && m_settings->shuffleCross) {
					shuffle[ruleMap[i - 2].r].trainers.push_back(ShuffleData::tmp(trainer));
				}
				else if (isBoss && m_settings->shuffleBosses) {
					shuffle[ruleMap[i - 2].r].bosses.push_back(ShuffleData::tmp(trainer));
				}
			}
		}
		

		std::sort(shuffle[0].trainers.begin(), shuffle[0].trainers.end());
		std::sort(shuffle[0].bosses.begin(), shuffle[0].bosses.end());
		std::sort(shuffle[1].trainers.begin(), shuffle[1].trainers.end());
		std::sort(shuffle[1].bosses.begin(), shuffle[1].bosses.end());
	}

	//
	// set up trainer generator
	//
	TrainerGenerator tgen;
	{
		using namespace std::placeholders;
		if (m_settings->min1Buttons == 1) tgen.gen.minOneMoveFilter = std::bind(&FilterMoveByBP, _1, 1, 999);
		else if (m_settings->min1Buttons == 2) tgen.gen.minOneMoveFilter = std::bind(&FilterMoveByBP, _1, 75, 999);
		else if (m_settings->min1Buttons == 3) tgen.gen.minOneMoveFilter = &FilterMoveByStab;
		else if (m_settings->min1Buttons == 4) tgen.gen.minOneMoveFilter =
			std::bind(std::logical_and<bool>(), std::bind(&FilterMoveByBP, _1, 75, 999), std::bind(&FilterMoveByStab, _1, _2));
		else if (m_settings->trainerMin1Atk) tgen.gen.minOneMoveFilter = std::bind(&FilterMoveByBP, _1, 1, 999);
		else tgen.gen.minOneMoveFilter = nullptr;
	}
	tgen.usefulItem = m_settings->battleItems;
	tgen.gen.changeLevel = m_settings->trainerRandLevels;
	tgen.gen.changeSpecies = m_settings->trainerRandSpecies;
	tgen.changeName = m_settings->trainerRandName;
	tgen.changePokemonNicknames = m_settings->trainerRandMonNames;
	tgen.gen.randEvs = m_settings->randEvIv;
	tgen.gen.randIvs = m_settings->randEvIv;
	tgen.gen.statsUniformDistribution = m_settings->randEvIvUniformDist;
	tgen.gen.levelUniformDistribution = m_settings->randLevelsUniformDist;
	tgen.stayCloseToBST = m_settings->stayCloseToBST;
	tgen.stayCloseToBSTThreshold = 30;


	SetPartialProgress(0.1);

	//
	// randomize trainers with generator
	//
	auto trainerIt = m_romRoster->trainerBegin() + 2;
	for (int i = 2; i <= 63; i++, ++trainerIt) {
		//set trainer generator rules to cup rules
		tgen.minLevel = ruleMap[i - 2]->minLevel;
		tgen.maxLevel = ruleMap[i - 2]->maxLevel;
		tgen.levelSum = ruleMap[i - 2]->levelSum;
		if (m_settings->legalMovesOnly) {
			if (ruleMap[i - 2]->legalMoveFilter) {
				using namespace std::placeholders;
				tgen.gen.generalMoveFilter = std::bind(std::logical_and<bool>(),
					std::bind(FilterLegalMovesOnly, _1, _2),
					std::bind(ruleMap[i - 2]->legalMoveFilter, _1, _2));
			}
			else {
				tgen.gen.generalMoveFilter = FilterLegalMovesOnly;
			}
		}
		else 
			tgen.gen.generalMoveFilter = ruleMap[i - 2]->legalMoveFilter;
		tgen.gen.speciesFilterBuffer = ruleMap[i - 2]->legalMonList;
		tgen.gen.speciesFilterBufferN = ruleMap[i - 2]->legalMonListN;

		//iterate through trainers of this cup
		auto oldOneMoveFilter = tgen.gen.minOneMoveFilter;
		int nTrainers = trainerIt->nTrainers;
		for (int j = 0; j < nTrainers; j++) {
			bool isBoss = j == nTrainers - 1;
			if (!isBoss) switch (trainerIt->trainers[j].trainerCat) {
			case GameInfo::GYM_LEADER:
			case GameInfo::ELITE_FOUR:
			case GameInfo::CHAMPION:
			case GameInfo::RIVAL:
			case GameInfo::RIVAL17:
				isBoss = true;
				break;
			case GameInfo::POKEMON_TRAINER:
				isBoss = trainerIt->trainers[j].trainerId - 1 == TableInfo::RED;
				break;
			default:
				break;
			}

			//set trainer specific options
			if (isBoss) {
				tgen.stayCloseToBST = m_settings->bossStayCloseToBST;
				using namespace std::placeholders;
				if (tgen.gen.minOneMoveFilter == nullptr) {
					tgen.gen.minOneMoveFilter = std::bind(FilterMoveByBP, _1, 60, 999);
				}
				else {
					tgen.gen.minOneMoveFilter = std::bind(std::logical_and<bool>(),
						std::bind(tgen.gen.minOneMoveFilter, _1, _2),
						std::bind(&FilterMoveByBP, _1, 60, 999));
				}
			}
			//shuffle trainer if shuffle requested
			if (m_settings->shuffleRegulars || m_settings->shuffleBosses) {
				auto shuffleIfFound = [&](std::vector<ShuffleData::tmp>& vc, unsigned int& benched) -> bool {
					ShuffleData::tmp s(trainerIt->trainers[j]);
					int lastRemTrainerN = vc.size() - 1 - benched;
					for (int i = 0; i < vc.size(); i++) {
						if (vc[i] == s) {
							//we are part of this vector, so replace us with a random element
							//replace with other trainer from this vector
							int rand = Random::GetInt(0, lastRemTrainerN);
							trainerIt->trainers[j].trainerId = vc[rand].id;
							trainerIt->trainers[j].trainerCat = vc[rand].cat;
							trainerIt->trainers[j].textId = vc[rand].text;

							m_genLog << "shuffling trainer " << (int)trainerIt->trainers[j].trainerId << " (" << (int)trainerIt->trainers[j].textId
								<< ") and " << vc[rand].id << "( " << (int)vc[rand].text << ")\r\n";
							std::swap(vc[rand], vc[lastRemTrainerN]);
							benched++;

							return true;
						}
					}
					return false;
				};
				bool suc = false;
				if (!suc) suc = shuffleIfFound(shuffle[ruleMap[i - 2].r].bosses, shuffle[ruleMap[i - 2].r].bossesNBenched);
				if (!suc) suc = shuffleIfFound(shuffle[ruleMap[i - 2].r].trainers, shuffle[ruleMap[i - 2].r].trainersNBenched);
			}

			if (trainerIt->trainers[j].trainerId != (uint32_t)GameInfo::TrainerNames::GRUNT) {
				if (m_settings->mixCustomsInBosses && isBoss || m_settings->mixCustomsInTrainers && !isBoss) {
					uint32_t id = trainerIt->trainers[j].trainerId;
					auto it = std::find_if(shuffledIds.begin(), shuffledIds.end(), [&](const auto& lhs) {return lhs.trainerId == id; });
					if (it == shuffledIds.end()) {
						shuffledIds.push_back({ id , {&trainerIt->trainers[j]} });
					}
					else it->trainersStructs.push_back(&trainerIt->trainers[j]);
				}
			}

			DefTrainer newDef = tgen.Generate(trainerIt->trainers[j]);

			//restore trainer specific options
			if (m_settings->min1Buttons == 5) 
				for(int i = 0; i < newDef.nPokes; i++)
					{ newDef.pokemon[i].move1 = newDef.pokemon[i].move2 = newDef.pokemon[i].move3 = newDef.pokemon[i].move4 = GameInfo::METRONOME; }
			trainerIt->trainers[j] = newDef;
			if (isBoss) {
				tgen.stayCloseToBST = m_settings->stayCloseToBST;
				tgen.gen.minOneMoveFilter = oldOneMoveFilter;
			}
			if (m_settings->mixCustomsInBosses) {
				tgen.changeName = m_settings->trainerRandName;
			}
			
			newDef.Print(m_romText, m_genLog);
			m_genLog << "\n";
			//SetProgress(0.08 + (i - 2) * 0.0132 + 0.0132 * (j+1)/(float)nTrainers);
		}

		SetPartialProgress(0.1 + (i - 1) * (0.9 / 62));
	}

	//
	// shuffle in custom trainers if reqeusted
	//
	for (int i = 0; i < shuffledIds.size(); i++) {
		if (customTrainersUsed.size() < GlobalConfig::CustomTrainers.customTrainers.size()
			&& (Random::GetInt(0, shuffledIds.size()-1-i + GlobalConfig::CustomTrainers.customTrainers.size()) > shuffledIds.size()-1-i))
		{
			//replace with custom trainer
			int customN = GlobalConfig::CustomTrainers.customTrainers.size();
			int usedN = customTrainersUsed.size();
			//get unused trainer id (adjust random number)
			int randomIndex = Random::GetInt(0, customN - usedN - 1);
			for (int i = 0; i < usedN; i++) {
				if (customTrainersUsed[i] >= randomIndex) randomIndex++;
			}
			customTrainersUsed.push_back(randomIndex);
			std::sort(customTrainersUsed.begin(), customTrainersUsed.end());

			m_genLog << "shuffling trainer " << (int)shuffledIds[i].trainerId << " with custom trainer " << randomIndex << "\r\n";
			for (auto trainer : shuffledIds[i].trainersStructs) {
				auto customTrainer = GlobalConfig::CustomTrainers.GenPermutation(randomIndex, customFaceList);
				customTrainerTextMods.AddChange(TableInfo::TEXT_TRAINER_NAMES, trainer->trainerId - 1, customTrainer.name);
				for (int i = 0; i < 40; i++) {
					if (customTrainer.textLines[i].size() > 0) {
						customTrainerTextMods.AddChange(TableInfo::TEXT_TRAINER_TEXT_FIRST + trainer->textId,
							i, customTrainer.textLines[i]);
					}
				}
				m_genLog << "... replaced text id " << (int)trainer->textId
					<< ") with custom " << customTrainer.name << "\r\n";
			}
		}
	}
	

	//
	// build portrait list of used custom trainers
	//
	m_customFInfoTable.resize(customFaceList.size());
	m_customFaceTable.resize(customFaceList.size());

	for (int i = 0; i < customFaceList.size(); i++) {
		//build Faces struct in vector (which is uint8 buffer)
		auto& face = customFaceList[i];
		m_customFaceTable[i].reserve((4 + face.size() * (4 + 8 + 64 * 64 * 2)) + 0xF & ~0xF); //estimate size
		uint32_t totalSize = 4 + face.size() * 4; //nFaces and offsets
		m_customFaceTable[i].resize(totalSize);
		DefFaces::Faces* faces = (DefFaces::Faces*)m_customFaceTable[i].data();
		faces->nFaces = face.size();
		for (int j = 0; j < face.size(); j++) {
			CImage img;
			HRESULT res = img.Load(face[j].c_str());
			if (res != S_OK) {
				int error = GetLastError();
				std::stringstream errorMsg;
				errorMsg << "Could not load image file " << face[j] << "; error " << error;
				throw std::invalid_argument(errorMsg.str());
			}
			else {
				uint32_t oldSize = totalSize;
				totalSize += 8 + img.GetWidth()*img.GetHeight() * 2;
				m_customFaceTable[i].resize(totalSize);
				DefFaces::Faces::Face* face = (DefFaces::Faces::Face*)(m_customFaceTable[i].data() + oldSize);
				faces = (DefFaces::Faces*)m_customFaceTable[i].data();
				faces->offsets[j] = oldSize + 8; //this offset points to pixels for some reason
				face->width = img.GetWidth();
				face->height = img.GetHeight();
				face->unknown = 0x20000;
				face->SetPixelsFromImage(img);
			}
		}
		//align to 0x10 border
		m_customFaceTable[i].resize(totalSize + 0xF & ~0xF);
	}

	m_genLog << "... generating trainers done!\n";

	
	//
	//apply text changes
	//
	DefText* newText = (DefText*)new uint8_t[DefText::segSize];
	if (customTrainerTextMods.changes.size() > 0) {
		customTrainerTextMods.Add(tgen.textChanges);
		tgen.textChanges = customTrainerTextMods;
	}
	tgen.textChanges.Apply(m_romText, newText);
	memcpy(m_romText, newText, DefText::segSize);
	delete[]((uint8_t*)newText);

	SetPartialProgress(1.0);
}

void Randomizer::RandomizeItems()
{
	uint8_t* checksumPartBuffer = nullptr;

	if (!m_settings->randChooseItems) return;

	const auto tid = [](int id) {return id + 5; };

	m_genLog << "generating random item sets...\n";

	//make an item array with valid items to avoid repitition
	GameInfo::ItemId battleItemMapCopy[_countof(GameInfo::BattleItemMap)];
	unsigned int battleItemMapCopySize;
	if (m_settings->randIncludeMonspecificItems) {
		battleItemMapCopySize = _countof(GameInfo::BattleItemMap);
		memcpy(battleItemMapCopy, GameInfo::BattleItemMap, sizeof GameInfo::BattleItemMap);
	}
	else {
		battleItemMapCopySize = _countof(GameInfo::GeneralBattleItemMap);
		memcpy(battleItemMapCopy, GameInfo::GeneralBattleItemMap, sizeof GameInfo::GeneralBattleItemMap);
	}
	const auto AddItemSet = [&](int tidBorder, int itemIndex) {
		//replace the existing one if this exact one was allready added
		if (m_customIInfoTable.size() > 0 && m_customIInfoTable.back().fightMin == tidBorder) {
			m_customIInfoTable.back() = CustomItemInfo{ (uint16_t)tidBorder,(uint32_t)itemIndex };
		}
		else {
			m_customIInfoTable.emplace_back(CustomItemInfo{ (uint16_t)tidBorder,(uint32_t)itemIndex });
		}

		return m_customIInfoTable.size() - 1;
	};
	const auto GenItemsAndAdd = [&](int nItems) {
		auto& outList = m_customItemTables.emplace_back();
		outList.resize(nItems + 1);	//terminated with NO_ITEM

		for (int i = 0; i < nItems; i++) {
			std::uniform_int_distribution<unsigned int> dist(0, battleItemMapCopySize - 1 - i);
			unsigned int index = dist(Random::Generator);
			GameInfo::ItemId item = battleItemMapCopy[index];
			outList[i] = item;
			m_genLog << GameInfo::ItemNames[item] << "\n";

			std::swap(battleItemMapCopy[index], battleItemMapCopy[battleItemMapCopySize - 1 - i]);
		}
		outList.back() = GameInfo::NO_ITEM;

		return m_customItemTables.size() - 1;
	};

	int nItems = 6;
	if (m_settings->changeItemN) nItems = _tstoi(m_settings->changeItemNText);

	int defaultRentalItems = GenItemsAndAdd(nItems);	 //default item list
	AddItemSet(tid(0), defaultRentalItems);

	if (m_settings->itemsPerRentalSet) {
		unsigned int pokeCup = AddItemSet(tid(2), GenItemsAndAdd(nItems));
		if (m_settings->multiplePokecupRentals) {
			AddItemSet(tid(3), GenItemsAndAdd(nItems));
			AddItemSet(tid(4), GenItemsAndAdd(nItems));
			AddItemSet(tid(5), GenItemsAndAdd(nItems));
		}
		unsigned int littleCup = AddItemSet(tid(6), GenItemsAndAdd(nItems));
		unsigned int primeCup = AddItemSet(tid(7), GenItemsAndAdd(nItems));
		unsigned int glc = AddItemSet(tid(8), GenItemsAndAdd(nItems));
		if (m_settings->multipleGlcRentals) {
			AddItemSet(tid(13), GenItemsAndAdd(nItems));//new rentals from jasmine
			AddItemSet(tid(19), GenItemsAndAdd(nItems));//new rentals after elite 4
		}
		AddItemSet(tid(29), pokeCup);
		if (m_settings->multipleR2Rentals) {
			if (m_settings->multiplePokecupRentals) {
				//pokecups
				AddItemSet(tid(29), GenItemsAndAdd(nItems));
				AddItemSet(tid(30), GenItemsAndAdd(nItems));
				AddItemSet(tid(31), GenItemsAndAdd(nItems));
				AddItemSet(tid(32), GenItemsAndAdd(nItems));
			}
			AddItemSet(tid(33), littleCup);
			if (m_settings->multiplePokecupRentals) {
				//littlecup
				AddItemSet(tid(33), GenItemsAndAdd(nItems));
			}
			AddItemSet(tid(34), primeCup);
			if (m_settings->multiplePokecupRentals) {
				AddItemSet(tid(34), GenItemsAndAdd(nItems));
			}
			AddItemSet(tid(35), glc);
			if (m_settings->multipleGlcRentals) {
				AddItemSet(tid(35), GenItemsAndAdd(nItems)); //start at falkner
				AddItemSet(tid(40), GenItemsAndAdd(nItems));//new rentals from jasmine
				AddItemSet(tid(46), GenItemsAndAdd(nItems));//new rentals after elite 4
				AddItemSet(tid(56), defaultRentalItems); //end it after rival
			}
		}
	}

	std::sort(m_customIInfoTable.begin(), m_customIInfoTable.end(), [](CustomItemInfo& lhs, CustomItemInfo& rhs) -> bool { return lhs.fightMin > rhs.fightMin; });

	m_genLog << "generating random item sets done!\n";

	SetPartialProgress(1.0);
}

void Randomizer::SortInjectedData()
{
	//
	//gameplan: first add all curated replacement buffers to m_romReplacements, then swap all
	//
	constexpr uint32_t alignment = 0x3;
	auto Align = [=](uint32_t& o) {o = (o + alignment) & (~alignment); };

	//these are direct copies of rom parts, so these can just be copied
	m_romRoster->Curate(false);
	m_romReplacements.emplace_back( RomReplacements{ (uint8_t*)m_romRoster, DefRoster::segSize, DefRoster::offStart });
	m_romText->Curate(false);;
	m_romReplacements.emplace_back( RomReplacements{ (uint8_t*)m_romText, DefText::segSize, DefText::offStart });

	//new data is more complicated: code, custom info tables, item lists, rental mons
	bool insertItemData = m_customIInfoTable.size() > 1;
	bool insertRentalData = m_customRInfoTable.size() > 1;
	bool insertFaceData = m_customFaceTable.size() > 0;

	const uint32_t borders[][2] = { { emptySpace1Start, emptySpace1End }, { emptySpace2Start, emptySpace2End } };
	uint32_t offsetIts[_countof(borders)] = { borders[0][0], borders[1][0] };
	auto AddData = [&](uint8_t* buffer, unsigned int size) {
		//find a free space where it still fits
		for (int i = 0; i < _countof(offsetIts); i++) {
			Align(offsetIts[i]);
			uint32_t remainingSpace = borders[i][1] - offsetIts[i];
			if (remainingSpace < size) continue;
			//fits
			uint32_t offset = offsetIts[i];
			m_romReplacements.emplace_back(RomReplacements{ buffer, size, offset });
			offsetIts[i] += size;
			return offset;
		}
		throw std::bad_alloc();
	};
	auto AddDataVec = [&](auto& vector) {
		return AddData((uint8_t*)vector.data(), vector.size() * sizeof(vector[0]));
	};

	//insert code
	if (insertItemData) {
		unsigned int size;
		
		//our custom code
		m_customItemInjectCode = InjectedItem::CreateInjection(&size);
		uint32_t itemCodeAddr = AddData(m_customItemInjectCode, size);

		//the redirect in their code
		m_customItemRedirectCode = InjectedItem::CreateRedirect(&size);
		m_romReplacements.emplace_back(RomReplacements{ m_customItemRedirectCode, size, RentalItemFuncAddr() });
		InjectedItem::SetRedirectTarget(m_customItemRedirectCode, itemCodeAddr + 0xB0000000);
	}
	SetPartialProgress(0.1);

	if (insertRentalData) {
		unsigned int size;

		//our custom code
		m_customRentalInjectCode = InjectedRental::CreateInjection(&size);
		uint32_t rentalCodeAddr = AddData(m_customRentalInjectCode, size);

		//the redirect in their code
		m_customRentalRedirectCode = InjectedRental::CreateRedirect(&size);
		m_romReplacements.emplace_back(RomReplacements{ m_customRentalRedirectCode, size, RentalRosterFuncAddr() });
		InjectedRental::SetRedirectTarget(m_customRentalRedirectCode, rentalCodeAddr + 0xB0000000);
	}
	SetPartialProgress(0.2);

	if (insertFaceData) {
		unsigned int size;

		//our custom code
		m_customFaceInjectCode = InjectedFace2::CreateInjection(&size);
		uint32_t faceCodeAddr = AddData(m_customFaceInjectCode, size);

		//the redirect in their code
		m_customFaceRedirectCode = InjectedFace2::CreateRedirect(&size);
		m_romReplacements.emplace_back(RomReplacements{ m_customFaceRedirectCode, size, Face2RedirectAdr() });
		InjectedFace2::SetRedirectTarget(m_customFaceRedirectCode, faceCodeAddr + 0xB0000000);
	}

	SetPartialProgress(0.3);

	uint32_t codeOffset = offsetIts[0];
	uint32_t codeSize = 0x1000;
	offsetIts[0] += codeSize;

	//insert header tables, adjust code
	if (insertItemData) {
		m_genItemTableOffset = AddDataVec(m_customIInfoTable);
		m_genLog << "placed custom item table header at " << m_genItemTableOffset << "\n";
		InjectedItem::SetInjectionTableAddress(m_customItemInjectCode, m_genItemTableOffset + 0xB0000000);
		m_needsNewCrc = true;
	}
	if (insertRentalData) {
		m_genRosterTableOffset = AddDataVec(m_customRInfoTable);
		m_genLog << "placed custom rental table header at " << m_genRosterTableOffset << "\n";
		InjectedRental::SetInjectionTableAddress(m_customRentalInjectCode, m_genRosterTableOffset + 0xB0000000);
		m_needsNewCrc = true;
	}
	if (insertFaceData) {
		m_genFaceTableOffset = AddDataVec(m_customFInfoTable);
		m_genLog << "placed custom face table header at " << m_genFaceTableOffset << "\n";
		InjectedFace2::SetInjectionTableAddress(m_customFaceInjectCode, m_genFaceTableOffset + 0xB0000000);
		m_needsNewCrc = true;
	}
	SetPartialProgress(0.4);

	//insert data, adjust header tables
	if (insertItemData) {
		for (unsigned int i = 0; i < m_customItemTables.size(); i++) {
			//if these are byteswapped / wordswapped, we have to align them so that each can be swapped individually
			int alignment = m_byteswapped ? 2 : m_wordswapped ? 4 : 1;
			int alignmentMiss = m_customItemTables[i].size() % alignment;
			if (alignmentMiss != 0) {
				int oldSize = m_customItemTables[i].size();
				m_customItemTables[i].resize(oldSize + alignment - alignmentMiss);
				for (int j = oldSize; j < m_customItemTables[j].size(); i++) m_customItemTables[i][j] = GameInfo::NO_ITEM;
			}
			uint32_t romOffset = AddDataVec(m_customItemTables[i]);
			//adjust offset in info struct
			for (auto& info : m_customIInfoTable) {
				if (info.itemPtr == i) {
					info.itemPtr = romOffset + 0xB0000000; //this is where ROM is in virtual memory
				}
			}
			m_genLog << "placed custom item list nr. " << i << " at " << romOffset << " - " << romOffset + m_customItemTables[i].size() << "\n";
		}
	}
	if (insertRentalData) {
		for (unsigned int i = 0; i < m_customRentalTables.size(); i++) {
			uint32_t romOffset = AddDataVec(m_customRentalTables[i]);
			//adjust length and offset based on original table position
			for (auto& info : m_customRInfoTable) {
				if (info.rentalOffset == i) {
					info.rentalOffset = romOffset - DefRoster::offStart;
					info.rentalLength = m_customRentalTables[i].size();
				}
			}

			m_genLog << "placed custom rental table nr. " << i << " at " << romOffset << " - " << romOffset + m_customRentalTables[i].size() << "\n";
		}
	}
	if (insertFaceData) {
		for (int i = 0; i < m_customFaceTable.size(); i++) {
			uint32_t romOffset = AddDataVec(m_customFaceTable[i]);
			//adjust length and offset bad on original table position
			m_customFInfoTable[i].faceLength = m_customFaceTable[i].size();
			m_customFInfoTable[i].faceOffset = romOffset - DefFaces::offStart;
		}
	}
	m_genLog << "done placing custom data; used " << offsetIts[0] - borders[0][0] << "/" << borders[0][1] - borders[0][0] << " in space 1, "
		<< offsetIts[1] - borders[1][0] << "/" << borders[1][1] - borders[1][0] << " in space 2\n";
	SetPartialProgress(0.6);

	//now curate it all
	for (auto& it : m_customIInfoTable) it.Curate(false);
	for (auto& it : m_customRInfoTable) it.Curate(false);
	for (auto& it : m_customFInfoTable) it.Curate(false);
	//for (auto& it : m_customItemTables) item table is list of bytes and does not need to be curated
	for (auto& table : m_customRentalTables)
		for (DefPokemon* it = (DefPokemon*)(table.data() + 4); it < (DefPokemon*)(table.data() + table.size()); it++) it->Curate(false);
	for (auto& table : m_customFaceTable)
		((DefFaces::Faces*)table.data())->Curate(false);
	SetPartialProgress(0.85);

	//all gathered, now do swaps
	for (auto& rep : m_romReplacements) {
		DoSwaps(rep.buffer, rep.bufferSize);
	}
	SetPartialProgress(1.0);
}
