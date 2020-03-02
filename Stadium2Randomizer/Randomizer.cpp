#include "stdafx.h"
#include "Randomizer.h"

#include <iostream>
#include <sstream>

#include "CMainDialog.h"
#include "Crc.h"
#include "CustomRosterInfo.h"
#include "CustomCode.h"
#include "Filters.h"
#include "PokemonGenerator.h"
#include "TrainerGenerator.h"
#include "SpeciesGenerator.h"
#include "MoveGenerator.h"
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
	delete[] m_customMoves, m_customMoves = nullptr;
	delete[] m_customPokemon, m_customPokemon = nullptr;
	delete[](uint8_t*)m_romRoster, m_romRoster = nullptr;
	delete[](uint8_t*)m_romText, m_romText= nullptr;
	delete[] m_customItemRedirectCode, m_customItemRedirectCode = nullptr;
	delete[] m_customItemInjectCode, m_customItemInjectCode = nullptr;
	delete[] m_customRentalRedirectCode, m_customRentalRedirectCode = nullptr;
	delete[] m_customRentalInjectCode, m_customRentalInjectCode = nullptr;
	delete[] m_customFaceInjectCode, m_customFaceInjectCode = nullptr;
	delete[] m_customFaceRedirectCode, m_customFaceRedirectCode = nullptr;
	delete[] m_customStringsHelperInjectCode, m_customStringsHelperInjectCode = nullptr;
	delete[] m_customStringsGetRedirectCode, m_customStringsGetRedirectCode = nullptr;
	delete[] m_customStringsGetInjectCode, m_customStringsGetInjectCode = nullptr;
	delete[] m_customStringsInitRedirectCode, m_customStringsInitRedirectCode = nullptr;
	delete[] m_customStringsInitInjectCode, m_customStringsInitInjectCode = nullptr;
}

void Randomizer::Randomize(const CString& path, CMainDialog * settings)
{
	Randomizer r(settings);
	r.RandomizeRom(path);
}




void Randomizer::RandomizeRom(const CString & path)
{
	//setup general data
	m_genLog.open("gen\\genLog.txt");						//logger

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

	if (m_settings->uberSpeciesBst || m_settings->uberSpeciesTypes) {
		RandomizeSpecies();
	}
	if (m_settings->uberMoves) {
		RandomizeMoves();
	}

	if (m_settings->randRentals) {
		m_progressPartMinPercent = 0.0;
		m_progressPartMaxPercent = 0.1;
		RandomizeRegularRentals();

	
		m_progressPartMinPercent = 0.1;
		m_progressPartMaxPercent = 0.5;
		RandomizeHackedRentals();
	}

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

			bool touchedCrcArea = false;
			for (auto& rep : m_romReplacements) {
				if (rep.romOffset < CHECKSUM_END) {
					touchedCrcArea = true;
					break;
				}
			}
			if (touchedCrcArea) {
				uint8_t* checksumPartBuffer = new uint8_t[CHECKSUM_END];
				m_in.open(dlg.GetPathName(), std::ifstream::binary);
				m_in.seekg(0);
				m_in.read((char*)checksumPartBuffer, CHECKSUM_END);
				DoSwaps(checksumPartBuffer, CHECKSUM_END);

				if (m_settings->patchCic) {
					//instead of recalculation CRCs, patch the bootcode
					//stadium 2 has CIC version 6103 - we nop at address 0x63C and 0x648
					*((uint32_t*)(checksumPartBuffer + 0x63c)) = 0;
					*((uint32_t*)(checksumPartBuffer + 0x648)) = 0;
				}
				else {
					//recalculate CRCs and replace them
					uint32_t newCrcs[2];
					bool crcSuc = CalculateCrcs(newCrcs, checksumPartBuffer);
					if (crcSuc) {
						SwitchEndianness(newCrcs[0]);
						SwitchEndianness(newCrcs[1]);
						//crcs are at 0x10 and 0x14
						*((uint32_t*)(checksumPartBuffer + 0x10)) = newCrcs[0];
						*((uint32_t*)(checksumPartBuffer + 0x14)) = newCrcs[1];
					}
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


int Randomizer::CustomStringData::AddStringOverride(uint8_t tableId, uint8_t strIdStart, const std::string& str)
{
	int ti;
	for (ti = 0; ti < tables.size(); ti++)
		if (tables[ti].tableId == tableId)
			break;
	if (ti == tables.size()) {
		//need to add a new table
		tables.push_back(CustomStringTableInfo{ (uint16_t)tableId, 0 });
		sinfos.push_back(CustomStringInfo{strIdStart,strIdStart});
		strings.push_back(std::vector<std::string>{});
	}
	if (sinfos[ti].start <= strIdStart && strIdStart < sinfos[ti].end) {
		strings[ti][strIdStart - sinfos[ti].start] = str;
	}
	else if (sinfos[ti].end == strIdStart) {
		sinfos[ti].end++;
		strings[ti].push_back(str);
	}
	return ti;
}

void Randomizer::CustomStringData::Finalize()
{
	finalizedSinfoTables.clear();
	finalizedSinfoTables.resize(sinfos.size());
	for (int i = 0; i < finalizedSinfoTables.size(); i++) {
		int totalstrlen = 0;
		for (std::string& str : strings[i]) {
			totalstrlen += str.size() + 1; //+1 for \0
		}
		//buffer as true CustomStringInfo
		int size = sizeof(CustomStringInfo) + 4 * strings[i].size() + totalstrlen;
		//round up to 64 bit because of DMA
		size = (size + 7) / 8 * 8;
		tables[i].tableSize = size;
		finalizedSinfoTables[i].resize(size);
		uint8_t* fullBuffer = finalizedSinfoTables[i].data();
		CustomStringInfo*	info = (CustomStringInfo*)(fullBuffer);
		uint32_t*			arr  = (uint32_t*)		 (fullBuffer + sizeof(CustomStringInfo));
		uint8_t*			strs = (uint8_t*)		 (fullBuffer + sizeof(CustomStringInfo) + 4* strings[i].size());
		uint8_t*			end  = (uint8_t*)		 (fullBuffer + size);
		*info = sinfos[i];
		for (std::string& str : strings[i]) {
			uint32_t offset = strs - fullBuffer;
			*arr++ = offset;
			int len = str.size() + 1;
			memcpy_s(strs, strs-end, str.c_str(), len);
			strs += len;
		}
	}
	//add end marker for table table
	tables.push_back({ 0xFF,0xFF });
	sinfos.clear();
	strings.clear();
}

void Randomizer::CustomStringData::Curate(bool forth)
{
	for (auto& table : tables) table.Curate(forth);
	for (auto& stringtable : finalizedSinfoTables) {
		CustomStringInfo& sinfo = *((CustomStringInfo*)stringtable.data());
		sinfo.Curate(forth);
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
	pokeGen.changeEvsIvs = m_settings->randEvIv;
	pokeGen.changeLevel = m_settings->randLevels;
	pokeGen.changeMoves = m_settings->randMoves;
	pokeGen.changeItem = false;

	pokeGen.randEvs = m_settings->randEvIv;
	pokeGen.randIvs = m_settings->randEvIv;
	pokeGen.bstEvIvs = m_settings->rentalSpeciesEvIv;
	pokeGen.statsDist = m_settings->randEvIvDist;
	pokeGen.levelDist = m_settings->randLevelsDist;
	pokeGen.changeHappiness = m_settings->randRentalHappiness;

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

void Randomizer::RandomizeSpecies()
{
	m_customPokemon = new GameInfo::Pokemon[_countof(GameInfo::Pokemons)];
	memcpy_s(m_customPokemon, sizeof(GameInfo::Pokemons), GameInfo::Pokemons, sizeof(GameInfo::Pokemons));
	std::vector<bool> monDone;
	monDone.resize(_countof(GameInfo::Pokemons));

	SpeciesGenerator speciesGen;
	speciesGen.randomType = m_settings->uberSpeciesTypes;
	speciesGen.gainSecondTypeChance = _ttoi(m_settings->uberSpeciesAddTypePercent);
	speciesGen.looseSecondTypeChance = _ttoi(m_settings->uberSpeciesRemTypePercent);

	speciesGen.randomStats = m_settings->uberSpeciesBst;
	speciesGen.bstDist = m_settings->uberSpeciesBstDist;
	speciesGen.statDist = m_settings->uberSpeciesBstDist;
	speciesGen.dontDecreaseEvolutionBST = m_settings->uberSpeciesEvoBst;
	speciesGen.dontDecreaseEvolutionStats = m_settings->uberSpeciesEvoStats;
	speciesGen.keepBST = m_settings->uberSpeciesBstButtons == 0;
	speciesGen.stayCloseBST = m_settings->uberSpeciesBstButtons == 1;
	speciesGen.randomBST = m_settings->uberSpeciesBstButtons == 2;
	speciesGen.context = m_customPokemon;

	//c++ lambdas cant do recursive, but i cant be arsed to make a new method right now
	const auto gen = [&](int i) {
		auto genimp = [&](int i, auto& rec) {
			if (monDone[i]) return;
			if (GameInfo::PokemonPrevEvo[i] != GameInfo::NO_POKEMON) {
				rec(GameInfo::PokemonPrevEvo[i] - 1, rec);
			}
			GameInfo::Pokemon& mon = m_customPokemon[i];
			mon = speciesGen.Generate(i, mon);
			monDone[i] = true;
		};
		genimp(i, genimp);
	};
	for (int i = 0; i < 251; i++) {
		gen(i);
	}
	m_romReplacements.emplace_back(RomReplacements{ (uint8_t*)m_customPokemon, sizeof(GameInfo::Pokemons), 0x98f20 });

	m_genLog << "Generating Species: \n";
	auto& textIt = m_romText->begin()[TableInfo::POKEMON_NAMES];
	for (int i = 0; i < 251; i++) {
		auto& mon = m_customPokemon[i];
		m_genLog << textIt[i] << ":\t" << std::string(5 - (strlen(textIt[i]) + 1) / 4, '\t')
			<< GameInfo::TypeNames[mon.type1] << "/" << GameInfo::TypeNames[mon.type2] << "\t\t"
			<< "bsts: " << (int)mon.baseHp << " " << (int)mon.baseAtk << " " << (int)mon.baseDef << " " << (int)mon.baseSpd << " "
			<< (int)mon.baseSpA << " " << (int)mon.baseSpD << "\n";
	}
}
void Randomizer::RandomizeMoves()
{
	MoveGenerator moveGen;
	moveGen.balanced = m_settings->uberMovesBalance;
	moveGen.randomType = m_settings->uberMovesType;
	moveGen.randomAcc = m_settings->uberMovesAcc;
	moveGen.accDist = m_settings->uberMovesAccDist;
	moveGen.randomPp = m_settings->uberMovesPp;
	moveGen.ppDist = m_settings->uberMovesPpDist;
	moveGen.randomBp = m_settings->uberMovesBp;
	moveGen.stayCloseToBp = m_settings->uberMovesCloseBp;
	moveGen.bpDist = m_settings->uberMovesBpDist;
	moveGen.randomSecondary = m_settings->uberMovesSecEffect;
	moveGen.gainSecondaryEffectChance = _ttoi(m_settings->uberMovesSecAddPercent);
	moveGen.looseSecondaryEffectChance = _ttoi(m_settings->uberMovesSecRemPercent);
	moveGen.randomEffectChance = m_settings->uberMovesSecEffectChance;
	moveGen.ecDist = m_settings->uberMovesSecEffectChanceDist;
	moveGen.randomStatusMoveEffect = m_settings->uberMovesStatusMoves;

	m_customMoves = new GameInfo::Move[_countof(GameInfo::Moves)];
	memcpy_s(m_customMoves, sizeof(GameInfo::Moves), GameInfo::Moves, sizeof(GameInfo::Moves));
	for (int i = 0; i < _countof(GameInfo::Moves); i++) {
		GameInfo::Move& move = m_customMoves[i];
		move = moveGen.Generate(move);
	}
	m_romReplacements.emplace_back(RomReplacements{ (uint8_t*)m_customMoves, sizeof(GameInfo::Moves), 0x98430 });

	m_genLog << "Generating Moves: \n";
	auto& textIt = m_romText->begin()[TableInfo::MOVE_NAMES];
	for (int i = 0; i < 250; i++) {
		auto& move = m_customMoves[i];
		m_genLog << textIt[i] << ":\t" << std::string(5 - (strlen(textIt[i]) + 1) / 4, '\t')
			<< (int)move.basePower << "bp\t" << (move.basePower <= 9 ? "\t" : "") << (int)move.pp << "/" << (int)move.pp << "pp\t"
			<< (int)move.accuracy << "/255acc\t" << GameInfo::TypeNames[move.type] << "\t"
			<< (int)move.effectId << "[" << GameInfo::MoveEffectInfos[move.effectId].name << "]" << "\t(" << (int)move.effectChance << "/255 chance)"
			<< "\t rated " << moveGen.RateMove(move) << "\n";
	}
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
	pokeGen.statsDist = m_settings->randEvIvDist;
	pokeGen.levelDist = m_settings->randLevelsDist;
	pokeGen.changeHappiness = m_settings->randRentalHappiness;

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
			GameInfo::PokemonId species = m_cupRules[cup].legalMonListN == 0 ? (GameInfo::PokemonId)(i + 1) : m_cupRules[cup].legalMonList[i];
			DefPokemon newMon = pokeGen.Generate(species);
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
		}
		else {
			//all pokecups
			AddRentalSet(tid(29), GenRentalSetAndAdd(POKECUP));
		}

		//littlecup
		AddRentalSet(tid(33), GenRentalSetAndAdd(LITTLECUP));
		//primecup r2 has its own allready, so no need for it
		AddRentalSet(tid(34), -1); //end it
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

	//
	//Collect data
	//

	struct TrainerInfo {
		DefTrainer& def;		//def in file-buffer, can be edited in-place
		CupRulesId cup;
		int cupId;
		enum {
			ROUND_2 = 1,
			IS_BOSS = 2,
			LAST_OF_CUP = 4
		};
		int flags;
		bool is(int flag) { return (flags & flag) == flag; }
	};

	std::vector<TrainerInfo> trainerList;					//list of all trainers to iterate through
	std::vector<int> shuffleBosses[2], shuffleTrainers[2];	//trainers eligible for shuffling. bosses is empty if both are shuffled
	std::vector<int> shuffleInCustoms[2];					//trainers eligible for replacement
	trainerList.reserve(500);

	auto isBoss = [](const DefTrainer& trainer) {
		switch (trainer.trainerCat) {
		case GameInfo::GYM_LEADER:
		case GameInfo::ELITE_FOUR:
		case GameInfo::CHAMPION:
		case GameInfo::RIVAL:
		case GameInfo::RIVAL17:
			return true;
			break;
		case GameInfo::POKEMON_TRAINER:
			return trainer.trainerId - 1 == TableInfo::RED;
			break;
		default:
			break;
		}
		return false;
	};

	{
		auto it = m_romRoster->trainerBegin() + 2;
		auto addTrainer = [&, this](int cupId, CupRulesId crid, int round) {
			for (int j = 0; j < it->nTrainers; j++) {
				//
				//gen trainer
				DefTrainer& trainer = it->trainers[j];
				bool bIsBoss = isBoss(trainer);
				int flags = 0;
				if (round == 1)				flags |= TrainerInfo::ROUND_2;
				if (bIsBoss)				flags |= TrainerInfo::IS_BOSS;
				if (j == it->nTrainers - 1) flags |= TrainerInfo::LAST_OF_CUP;

				trainerList.push_back(TrainerInfo{ trainer, crid, cupId, flags });

				//
				//shuffle data
				if (m_settings->shuffleRegulars || m_settings->shuffleBosses) {
					if (!bIsBoss && m_settings->shuffleRegulars || bIsBoss && m_settings->shuffleBosses && m_settings->shuffleCross) {
						shuffleTrainers[round].push_back(trainerList.size() - 1);
					}
					else if (bIsBoss && m_settings->shuffleBosses) {
						shuffleBosses[round].push_back(trainerList.size() - 1);
					}
				}
				
				//
				//custom replacement data
				if (m_settings->mixCustomsInBosses || m_settings->mixCustomsInTrainers) {
					if (m_settings->mixCustomsInBosses && bIsBoss || m_settings->mixCustomsInTrainers && !bIsBoss) {
						shuffleInCustoms[round].push_back(trainerList.size() - 1);
					}
				}
			}
			++it;
		};
		for (int i = 2; i <= 5; i++) addTrainer(i, POKECUP, 0);
		for (int i = 6; i <= 6; i++) addTrainer(i, LITTLECUP, 0);
		for (int i = 7; i <= 7; i++) addTrainer(i, PRIMECUP, 0);
		for (int i = 8; i <= 28; i++) addTrainer(i, GLC, 0);

		for (int i = 29; i <= 32; i++) addTrainer(i, POKECUP, 1);
		for (int i = 33; i <= 33; i++) addTrainer(i, LITTLECUP, 1);
		for (int i = 34; i <= 34; i++) addTrainer(i, PRIMECUP, 1);
		for (int i = 35; i <= 55; i++) addTrainer(i, GLC, 1);

		for (int i = 56; i <= 56; i++) addTrainer(i, CHALLENGECUP_1, 0);
		for (int i = 57; i <= 57; i++) addTrainer(i, CHALLENGECUP_2, 0);
		for (int i = 58; i <= 58; i++) addTrainer(i, CHALLENGECUP_3, 0);
		for (int i = 59; i <= 59; i++) addTrainer(i, CHALLENGECUP_4, 0);

		for (int i = 60; i <= 60; i++) addTrainer(i, CHALLENGECUP_1, 1);
		for (int i = 61; i <= 61; i++) addTrainer(i, CHALLENGECUP_2, 1);
		for (int i = 62; i <= 62; i++) addTrainer(i, CHALLENGECUP_3, 1);
		for (int i = 63; i <= 63; i++) addTrainer(i, CHALLENGECUP_4, 1);
	}

	//
	// shuffle trainers if requested
	//
	auto shuffleArr = [&](std::vector<int>& ts) {
		int size = ts.size();
		for (int i = 0; i < size - 1; i++) {
			int useIndex = Random::GetInt(i, size-1);
			using namespace std;
			swap(ts[i], ts[useIndex]);
			swap(trainerList[i].def.trainerId, trainerList[useIndex].def.trainerId);
			swap(trainerList[i].def.trainerCat, trainerList[useIndex].def.trainerCat);
			swap(trainerList[i].def.textId, trainerList[useIndex].def.textId);
		}
	};
	shuffleArr(shuffleBosses[0]);
	shuffleArr(shuffleTrainers[0]);
	shuffleArr(shuffleBosses[1]);
	shuffleArr(shuffleTrainers[1]);

	std::vector<std::vector<std::string>> customFaceList;
	TextMods customTrainerTextMods;


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
	auto oldOneMoveFilter = tgen.gen.minOneMoveFilter;
	tgen.changePokes = m_settings->trainerRandPoke;
	tgen.usefulItem = m_settings->battleItems;
	tgen.gen.changeLevel = m_settings->trainerRandLevels;
	tgen.gen.changeSpecies = m_settings->trainerRandSpecies;
	tgen.gen.changeMoves = m_settings->trainerRandMoves;
	tgen.changeName = m_settings->trainerRandName;
	tgen.changePokemonNicknames = m_settings->trainerRandMonNames;
	tgen.gen.changeEvsIvs = m_settings->trainerRandEvIv;
	tgen.gen.randEvs = m_settings->trainerRandEvIv;
	tgen.gen.randIvs = m_settings->trainerRandEvIv;
	tgen.gen.statsDist = m_settings->trainerRandIvEvDist;
	tgen.gen.changeHappiness = m_settings->trainerRandHappiness;
	tgen.gen.changeItem = m_settings->trainerRandItems;
	tgen.gen.levelDist = m_settings->randLevelsDist;
	tgen.stayCloseToBST = m_settings->stayCloseToBST;
	tgen.stayCloseToBSTThreshold = 30;


	SetPartialProgress(0.1);

	//
	// randomize trainers with generator
	//
	CupRulesId lastRules = (CupRulesId)-1;
	for (int i = 0; i < trainerList.size(); i++) {
		auto& trainer = trainerList[i];
		//set trainer generator rules to cup rules
		if (trainer.cup != lastRules) {
			lastRules = trainer.cup;

			const CupRule& rules = m_cupRules[trainer.cup];
			tgen.minLevel = rules.minLevel;
			tgen.maxLevel = rules.maxLevel;
			tgen.levelSum = rules.levelSum;
			if (trainer.cup == GLC) {
				tgen.gen.changeLevel = true; //always adjust level for glc ruleset 
			}
			else {
				tgen.gen.changeLevel = m_settings->trainerRandLevels;
			}
			if (m_settings->legalMovesOnly) {
				if (rules.legalMoveFilter) {
					using namespace std::placeholders;
					tgen.gen.generalMoveFilter = std::bind(std::logical_and<bool>(),
						std::bind(FilterLegalMovesOnly, _1, _2),
						std::bind(rules.legalMoveFilter, _1, _2));
				}
				else {
					tgen.gen.generalMoveFilter = FilterLegalMovesOnly;
				}
			}
			else
				tgen.gen.generalMoveFilter = rules.legalMoveFilter;
			tgen.gen.speciesFilterBuffer = rules.legalMonList;
			tgen.gen.speciesFilterBufferN = rules.legalMonListN;
		}

		bool bIsBoss = trainer.is(TrainerInfo::LAST_OF_CUP) || isBoss(trainer.def);

		if (trainer.cupId == 55) { //rival round 2
			AddRival2Pokemon(trainer.def);
		}

		//set trainer specific options
		if (bIsBoss) {
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

		DefTrainer newDef = tgen.Generate(trainer.def);
		if (m_settings->min1Buttons == 5)
			for (int i = 0; i < newDef.nPokes; i++)
			{
				newDef.pokemon[i].move1 = newDef.pokemon[i].move2 = newDef.pokemon[i].move3 = newDef.pokemon[i].move4 = GameInfo::METRONOME;
			}

		trainer.def = newDef;

		//restore trainer specific options
		if (bIsBoss) {
			tgen.stayCloseToBST = m_settings->stayCloseToBST;
			tgen.gen.minOneMoveFilter = oldOneMoveFilter;
		}
		if (m_settings->mixCustomsInBosses) {
			tgen.changeName = m_settings->trainerRandName;
		}

		newDef.Print(m_romText, m_genLog);
		m_genLog << "\n";
		

		SetPartialProgress(0.1 + (i - 1) * (0.8 / trainerList.size()));
	}

	

	//
	// shuffle in custom trainers if reqeusted
	//
	int maxCustomTrainers = min(atoi(m_settings->strCustomTrainerN), GlobalConfig::CustomTrainers.customTrainers.size());
	std::vector<int> possibleCustomTrainers;
	possibleCustomTrainers.reserve(maxCustomTrainers);
	if (m_settings->mixCustomsInBosses || m_settings->mixCustomsInTrainers) {
		//determine which trainers to shuffle in
		for (int i = 0; i < maxCustomTrainers; i++) {
			int customN = GlobalConfig::CustomTrainers.customTrainers.size();
			int usedN = possibleCustomTrainers.size();
			//get unused trainer id (adjust random number)
			int randomIndex = Random::GetInt(0, customN - usedN - 1);
			for (int i = 0; i < usedN; i++) {
				if (possibleCustomTrainers[i] == randomIndex) randomIndex++;
				else break;
			}
			possibleCustomTrainers.push_back(randomIndex);
		}
		auto shuffleInCustom = [&, this](std::vector<int>& vec) {
			int nCustoms = 0;
			for (int i = 0; i < vec.size(); i++) {
				TrainerInfo& trainer = trainerList[vec[i]];
				int nLeft = vec.size() - 1 - i;
				int nCustomsLeft = possibleCustomTrainers.size() - nCustoms;
				if (Random::GetInt(0, nLeft + nCustomsLeft) > nLeft) {
					auto customTrainer = GlobalConfig::CustomTrainers.GenPermutation(possibleCustomTrainers[nCustoms], customFaceList);
					m_customStrings.AddStringOverride(TableInfo::TEXT_TRAINER_NAMES, 110 + nCustoms - 1, customTrainer.name); //-1 as they are 1 based
					int oldId = trainer.def.trainerId;
					trainer.def.trainerId = 110 + nCustoms;
					for (int i = 0; i < 40; i++) {
						if (customTrainer.textLines[i].size() > 0) {
							customTrainerTextMods.AddChange(TableInfo::TEXT_TRAINER_TEXT_FIRST + trainer.def.textId,
								i, customTrainer.textLines[i]);
						}
					}

					m_genLog << "shuffling trainer " << oldId << " with custom trainer " << customTrainer.name << "\r\n";
					nCustoms++;
				}
			}
		};
		shuffleInCustom(shuffleInCustoms[0]);
		shuffleInCustom(shuffleInCustoms[1]);
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
		if (faces->nFaces > 3) {
			throw std::exception("faces can only have up to 3 expressions!");
		}
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
				face->width = 64;
				face->height = 64;
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

void Randomizer::AddRival2Pokemon(DefTrainer& trainer) {
	trainer.nPokes = 6;
	
	trainer.pokemon[3].species = GameInfo::ZAPDOS;
	trainer.pokemon[3].item = GameInfo::MAGNET;
	trainer.pokemon[3].move1 = GameInfo::THUNDERBOLT;
	trainer.pokemon[3].move2 = GameInfo::DRILL_PECK;
	trainer.pokemon[3].move3 = GameInfo::REST;
	trainer.pokemon[3].move4 = GameInfo::SLEEP_TALK;
	trainer.pokemon[3].happiness = 0;
	trainer.pokemon[3].dvs = 0xFF;
	trainer.pokemon[3].evAtk = trainer.pokemon[3].evDef = trainer.pokemon[3].evHp
		= trainer.pokemon[3].evSpc = trainer.pokemon[3].evSpd = 0xFF;
	trainer.pokemon[3].level = 100;
	trainer.pokemon[3].unknown1 = trainer.pokemon[0].unknown1;
	trainer.pokemon[3].unknown2 = trainer.pokemon[0].unknown2;
	trainer.pokemon[3].unknown3 = trainer.pokemon[0].unknown3;

	trainer.pokemon[4].species = GameInfo::SUICUNE;
	trainer.pokemon[4].item = GameInfo::MYSTIC_WATER;
	trainer.pokemon[4].move1 = GameInfo::SURF;
	trainer.pokemon[4].move2 = GameInfo::TOXIC;
	trainer.pokemon[4].move3 = GameInfo::PROTECT;
	trainer.pokemon[4].move4 = GameInfo::ROAR;
	trainer.pokemon[4].happiness = 0;
	trainer.pokemon[4].dvs = 0xFF;
	trainer.pokemon[4].evAtk = trainer.pokemon[4].evDef = trainer.pokemon[4].evHp
		= trainer.pokemon[4].evSpc = trainer.pokemon[4].evSpd = 0xFF;
	trainer.pokemon[4].level = 100;
	trainer.pokemon[4].unknown1 = trainer.pokemon[1].unknown1;
	trainer.pokemon[4].unknown2 = trainer.pokemon[1].unknown2;
	trainer.pokemon[4].unknown3 = trainer.pokemon[1].unknown3;

	trainer.pokemon[5].species = GameInfo::CELEBI;
	trainer.pokemon[5].item = GameInfo::TWISTEDSPOON;
	trainer.pokemon[5].move1 = GameInfo::HEAL_BELL;
	trainer.pokemon[5].move2 = GameInfo::RECOVER;
	trainer.pokemon[5].move3 = GameInfo::LEECH_SEED;
	trainer.pokemon[5].move4 = GameInfo::PSYCHIC_M;
	trainer.pokemon[5].happiness = 0;
	trainer.pokemon[5].dvs = 0xFF;
	trainer.pokemon[5].evAtk = trainer.pokemon[5].evDef = trainer.pokemon[5].evHp
		= trainer.pokemon[5].evSpc = trainer.pokemon[5].evSpd = 0xFF;
	trainer.pokemon[5].level = 100;
	trainer.pokemon[5].unknown1 = trainer.pokemon[5].unknown1;
	trainer.pokemon[5].unknown2 = trainer.pokemon[5].unknown2;
	trainer.pokemon[5].unknown3 = trainer.pokemon[5].unknown3;
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
	m_customStrings.Finalize();
	bool insertStringData = m_customStrings.tables.size() > 1;

	uint32_t offsetIts[_countof(emptySpaces)];
	for (int i = 0; i < _countof(emptySpaces); i++) offsetIts[i] = emptySpaces[i].offStart;
	auto AddData = [&](uint8_t* buffer, unsigned int size) {
		//find a free space where it still fits
		for (int i = 0; i < _countof(offsetIts); i++) {
			Align(offsetIts[i]);
			uint32_t remainingSpace = emptySpaces[i].offEnd - offsetIts[i];
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

	if (insertStringData) {
		unsigned int size;

		//our custom code
		m_customStringsHelperInjectCode = InjectedStringsHelper::CreateInjection(&size);
		uint32_t helperAddr = AddData(m_customStringsHelperInjectCode, size);
		m_customStringsInitInjectCode = InjectedStringsInit::CreateInjection(&size);
		InjectedStringsInit::SetJalHelperAddress(m_customStringsInitInjectCode, helperAddr);
		uint32_t initAddr = AddData(m_customStringsInitInjectCode, size);
		m_customStringsGetInjectCode = InjectedStringsGet::CreateInjection(&size);
		InjectedStringsGet::SetJalHelperAddress(m_customStringsGetInjectCode, helperAddr);
		uint32_t getAddr = AddData(m_customStringsGetInjectCode, size);

		//the redirect in their code
		m_customStringsInitRedirectCode = InjectedStringsInit::CreateRedirect(&size);
		m_romReplacements.emplace_back(RomReplacements{ m_customStringsInitRedirectCode, size, StringInitRedirectAdr() });
		InjectedStringsInit::SetRedirectTarget(m_customStringsInitRedirectCode, initAddr + 0xB0000000);
		m_customStringsGetRedirectCode = InjectedStringsGet::CreateRedirect(&size);
		m_romReplacements.emplace_back(RomReplacements{ m_customStringsGetRedirectCode, size, StringGetRedirectAdr() });
		InjectedStringsGet::SetRedirectTarget(m_customStringsGetRedirectCode, getAddr + 0xB0000000);
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
	}
	if (insertRentalData) {
		m_genRosterTableOffset = AddDataVec(m_customRInfoTable);
		m_genLog << "placed custom rental table header at " << m_genRosterTableOffset << "\n";
		InjectedRental::SetInjectionTableAddress(m_customRentalInjectCode, m_genRosterTableOffset + 0xB0000000);
	}
	if (insertFaceData) {
		m_genFaceTableOffset = AddDataVec(m_customFInfoTable);
		m_genLog << "placed custom face table header at " << m_genFaceTableOffset << "\n";
		InjectedFace2::SetInjectionTableAddress(m_customFaceInjectCode, m_genFaceTableOffset + 0xB0000000);
	}
	if (insertStringData) {
		m_genStringTableOffset = AddDataVec(m_customStrings.tables);
		m_genLog << "placed custom face table header at " << m_genStringTableOffset << "\n";
		InjectedStringsHelper::SetInjectionTableAddress(m_customStringsHelperInjectCode, m_genStringTableOffset + 0xB0000000);
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
				for (int j = oldSize; j < m_customItemTables[i].size(); j++) m_customItemTables[i][j] = GameInfo::NO_ITEM;
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
			m_genLog << "placed custom face nr. " << i << " at " << romOffset << " - " << romOffset + m_customFaceTable[i].size() << "\n";
		}
	}
	if (insertStringData) {
		for (int i = 0; i < m_customStrings.finalizedSinfoTables.size(); i++) {
			uint32_t romOffset = AddDataVec(m_customStrings.finalizedSinfoTables[i]);
			//adjust length and offset bad on original table position
			m_customStrings.tables[i].tableOffset = romOffset - m_genStringTableOffset;
			m_genLog << "placed custom string table nr. " << i << " at " << romOffset << " - " << romOffset + m_customStrings.finalizedSinfoTables[i].size() << "\n";
		}
	}
	m_genLog << "done placing custom data; used\n";
	for (int i = 0; i < _countof(emptySpaces); i++) {
		m_genLog << "\t" << offsetIts[i] - emptySpaces[i].offStart << " / " << emptySpaces[i].size << " in space " << i << "\n";
	}
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
	m_customStrings.Curate(false);
	SetPartialProgress(0.85);

	//all gathered, now do swaps
	for (auto& rep : m_romReplacements) {
		DoSwaps(rep.buffer, rep.bufferSize);
	}
	SetPartialProgress(1.0);
}
