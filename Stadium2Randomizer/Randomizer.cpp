#include "stdafx.h"
#include "Randomizer.h"

#include "CMainDialog.h"
#include "Crc.h"
#include "CustomRosterInfo.h"
#include "CustomCode.h"
#include "Filters.h"
#include "PokemonGenerator.h"
#include "TrainerGenerator.h"
#include "GlobalRandom.h"


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

	if (choice == IDOK) {
		BOOL suc = FALSE;
		if (path == dlg.GetPathName()) {
			//overwrite original
			m_in.close();
			suc = TRUE;
		}
		else {
			suc = CopyFileA(path, dlg.GetPathName(), FALSE);
		}

		if (suc) {
			std::ofstream out(dlg.GetPathName(), std::ofstream::in | std::ofstream::out | std::ofstream::binary);
			for (auto& rep : m_romReplacements) {
				out.seekp(rep.romOffset);
				out.write((char*)rep.buffer, rep.bufferSize);
			}


			if (m_needsNewCrc) {
				uint8_t* checksumPartBuffer = new uint8_t[CHECKSUM_END];
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

		pokeGen.ClearAllFilters();
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

void Randomizer::RandomizeTrainers()
{
	m_genLog << "Randomizing Trainers...:\n";
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


	//rule map; see look FormatNotes
	const CupRule* ruleMap[62];
	for (int i = 2; i <= 5; i++) ruleMap[i - 2] = &m_cupRules[0];
	for (int i = 6; i <= 6; i++) ruleMap[i - 2] = &m_cupRules[1];
	for (int i = 7; i <= 28; i++) ruleMap[i - 2] = &m_cupRules[2];

	for (int i = 29; i <= 32; i++) ruleMap[i - 2] = &m_cupRules[0];
	for (int i = 33; i <= 33; i++) ruleMap[i - 2] = &m_cupRules[1];
	for (int i = 34; i <= 55; i++) ruleMap[i - 2] = &m_cupRules[2];

	for (int i = 34; i <= 55; i++) ruleMap[i - 2] = &m_cupRules[2];
	for (int i = 34; i <= 55; i++) ruleMap[i - 2] = &m_cupRules[2];
	for (int i = 34; i <= 55; i++) ruleMap[i - 2] = &m_cupRules[2];

	for (int i = 56; i <= 56; i++) ruleMap[i - 2] = &m_cupRules[3];
	for (int i = 57; i <= 57; i++) ruleMap[i - 2] = &m_cupRules[4];
	for (int i = 58; i <= 58; i++) ruleMap[i - 2] = &m_cupRules[5];
	for (int i = 59; i <= 59; i++) ruleMap[i - 2] = &m_cupRules[6];

	for (int i = 60; i <= 60; i++) ruleMap[i - 2] = &m_cupRules[3];
	for (int i = 61; i <= 61; i++) ruleMap[i - 2] = &m_cupRules[4];
	for (int i = 62; i <= 62; i++) ruleMap[i - 2] = &m_cupRules[5];
	for (int i = 63; i <= 63; i++) ruleMap[i - 2] = &m_cupRules[6];

	SetPartialProgress(0.1);



	auto trainerIt = m_romRoster->trainerBegin() + 2;
	for (int i = 2; i <= 63; i++, ++trainerIt) {
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
		auto oldOneMoveFilter = tgen.gen.minOneMoveFilter;
		int nTrainers = trainerIt->nTrainers;
		for (int j = 0; j < nTrainers; j++) {
			bool isBoss = j == nTrainers - 1;
			if (!isBoss) switch (trainerIt->trainers[j].trainerCat) {
			case GameInfo::GYM_LEADER:
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
			DefTrainer newDef = tgen.Generate(trainerIt->trainers[j]);
			trainerIt->trainers[j] = newDef;
			if (isBoss) {
				tgen.stayCloseToBST = m_settings->stayCloseToBST;
				tgen.gen.minOneMoveFilter = oldOneMoveFilter;
			}
			newDef.Print(m_romText, m_genLog);
			m_genLog << "\n";
			//SetProgress(0.08 + (i - 2) * 0.0132 + 0.0132 * (j+1)/(float)nTrainers);
		}

		SetPartialProgress(0.1 + (i - 1) * (0.9 / 62));
	}

	m_genLog << "... generating trainers done!\n";

	//apply text changes too
	DefText* newText = (DefText*)new uint8_t[DefText::segSize];
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
	SetPartialProgress(0.3);

	//insert data, adjust header tables
	if (insertItemData) {
		for (unsigned int i = 0; i < m_customItemTables.size(); i++) {
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
	m_genLog << "done placing custom data; used " << offsetIts[0] - borders[0][0] << "/" << borders[0][1] - borders[0][0] << " in space 1, "
		<< offsetIts[1] - borders[1][0] << "/" << borders[1][1] - borders[1][0] << " in space 2\n";
	SetPartialProgress(0.5);

	//now curate it all
	for (auto& it : m_customIInfoTable) it.Curate(false);
	for (auto& it : m_customRInfoTable) it.Curate(false);
	//for (auto& it : m_customItemTables) item table is list of bytes and does not need to be curated
	for (auto& table : m_customRentalTables)
		for (DefPokemon* it = (DefPokemon*)(table.data() + 4); it < (DefPokemon*)(table.data() + table.size()); it++) it->Curate(false);
	SetPartialProgress(0.75);

	//all gathered, now do swaps
	for (auto& rep : m_romReplacements) {
		DoSwaps(rep.buffer, rep.bufferSize);
	}
	SetPartialProgress(1.0);
}
