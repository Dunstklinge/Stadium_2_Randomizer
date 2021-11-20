#pragma once

#include <vector>
#include <stdint.h>
#include <fstream>
#include <functional>
#include <array>

#include "Constants.h"
#include "DefRoster.h"
#include "DefText.h"
#include "PokemonTable.h"
#include "MoveTable.h"
#include "CustomRosterInfo.h"
#include "CustomTrainerDefs.h"
#include "RandomizationParams.h"

class Randomizer
{
public:
	Randomizer(const RandomizationParams& settings);
	~Randomizer();

	static void Randomize(const CString& path, const RandomizationParams& settings, CWnd* owner);
	void RandomizeRom(const CString& path, CWnd* owner);

	static constexpr unsigned int WM_PROGRESS = WM_USER + 2;

	double GetCurrProgress() const {
		return m_currProgress;
	}
private:

	enum CupRulesId {
		POKECUP = 0,
		LITTLECUP = 1,
		PRIMECUP = 2,
		CHALLENGECUP_1 = 3, CHALLENGECUP_2 = 4, CHALLENGECUP_3 = 5, CHALLENGECUP_4 = 6, GLC = 7
	};
	struct CupRule {
		int minLevel;
		int maxLevel;
		int levelSum;

		std::function<bool(GameInfo::MoveId, GameInfo::PokemonId)> legalMoveFilter = nullptr;
		GameInfo::PokemonId* legalMonList = nullptr;
		unsigned int legalMonListN = 0;
	};
	typedef std::array<CupRule, 8> CupRules;

	const RandomizationParams* m_settings;	// randomization choices made by the user
	CWnd* m_owner;							// owner of popus, receives progress notifications
	double m_currProgress = 0.0;				// current randomization progress

	std::ofstream m_genLog;		// a file which protocols what was randomized and how
	std::ifstream m_in;			// stream to the ROM file
	CString m_romPath;			// path to the currently edited rom

	struct RomReplacements {
		uint8_t* buffer;
		uint32_t bufferSize;
		uint32_t romOffset;
	};
	std::vector<RomReplacements> m_romReplacements;	// Holds a list of byte-wise replacement rules that are generated
													// over the varying steps. At the end, these rules are what transforms
													// the regular rom into the randomized one.

	DefRoster* m_romRoster;
	DefText* m_romText;

	//
	//generated data
	//

	std::vector<CustomRosterInfo>				m_customRInfoTable;		//sorted from largest to smallest
	std::vector<std::vector<uint8_t>>			m_customRentalTables;	//indexed by rentalOffset, which is used as index temporarily
																		//uint8_t -> DefRoster::PokemonList, i.e 4 bytes, then DefPokemon array
	std::vector<CustomItemInfo>					m_customIInfoTable;		//sorted from largest to smallest
	std::vector<std::vector<GameInfo::ItemId>>	m_customItemTables;		//indexed by itemPtr, which is used as index temporarily
	std::vector<CustomFaceInfo>					m_customFInfoTable;
	std::vector<std::vector<uint8_t>>			m_customFaceTable;		//list of custom faces to be inserted. uint8_t -> actually DefFace
	std::vector<CustomTrainerDefs::Trainer>		m_customTrainers;
	struct CustomStringData {
		std::vector<CustomStringTableInfo> tables;
		std::vector<CustomStringInfo> sinfos;
		std::vector<std::vector<std::string>> strings;

		std::vector<std::vector<uint8_t>> finalizedSinfoTables;

		int AddStringOverride(uint8_t tableId, uint8_t strIdStart, const std::string& str);
		void Finalize();
		void Curate(bool forth);
	}											m_customStrings;

	GameInfo::Move* m_customMoves = nullptr;
	GameInfo::Pokemon* m_customPokemon = nullptr;
	uint8_t* m_customItemRedirectCode = nullptr;
	uint8_t* m_customItemInjectCode = nullptr;
	uint8_t* m_customRentalRedirectCode = nullptr;
	uint8_t* m_customRentalInjectCode = nullptr;
	uint8_t* m_customFaceRedirectCode = nullptr;
	uint8_t* m_customFaceInjectCode = nullptr;
	uint8_t* m_customStringsHelperInjectCode = nullptr;
	uint8_t* m_customStringsInitRedirectCode = nullptr;
	uint8_t* m_customStringsInitInjectCode = nullptr;
	uint8_t* m_customStringsGetRedirectCode = nullptr;
	uint8_t* m_customStringsGetInjectCode = nullptr;


	CupRules m_cupRules;
	char m_romRegion;	//E = US, P = EU, rest unsupported

	uint32_t m_genItemTableOffset;
	uint32_t m_genRosterTableOffset;
	uint32_t m_genFaceTableOffset;
	uint32_t m_genStringTableOffset;

	//general functions that make up the randomization functions
	void DoSetup();			// setus up output file and resets variables
	void AnalyseRom();		// gather information from rom thats needed for randomization process
	void RandomizeData();	// create randomized data and generate m_romReplacements out of it
	void SaveRom();			// create modified file and safe it

	//subfunctions
	CupRules GenerateCupRules();
	void RandomizeSpecies();
	void RandomizeMoves();
	void RandomizeRegularRentals();
	void RandomizeHackedRentals();
	void RandomizeTrainers();
	void RandomizeItems();
	void SortInjectedData();

	void AddRival2Pokemon(DefTrainer& trainer);

	void SetProgress(double percent);
	double m_progressPartMinPercent, m_progressPartMaxPercent;
	void SetPartialProgress(double percent);

	bool m_normal, m_byteswapped, m_wordswapped;
	void DoSwaps(void* buffer, size_t size);


	static constexpr uint32_t naRentalItemOffsets[]{ 0xD963B, 0xD9643, 0xD9653, 0xD9663, 0xD9673,0xD9683 };
	static constexpr uint32_t palRentalItemOffsets[]{ 0xD979B, 0xD97A3, 0xD97B3, 0xD97C3, 0xD97D3,0xD97E3 }; //after swaps

	static constexpr uint32_t naRentalItemFuncAddr = 0xD9630;
	static constexpr uint32_t euRentalItemFuncAddr = 0xD9790;
	inline uint32_t RentalItemFuncAddr() { return m_romRegion == 'E' ? naRentalItemFuncAddr : euRentalItemFuncAddr; }

	static constexpr uint32_t naRentalRosterFuncAddr = 0xFA740;
	static constexpr uint32_t euRentalRosterFuncAddr = 0xFA890;
	inline uint32_t RentalRosterFuncAddr() { return m_romRegion == 'E' ? naRentalRosterFuncAddr : euRentalRosterFuncAddr; }

	static constexpr uint32_t naFaceRedirectAdr = 0x142758;
	static constexpr uint32_t euFaceRedirectAdr = 0x1428C8;
	inline uint32_t FaceRedirectAdr() { return m_romRegion == 'E' ? naFaceRedirectAdr : euFaceRedirectAdr; }

	static constexpr uint32_t naFace2RedirectAdr = 0x4ECF0;
	static constexpr uint32_t euFace2RedirectAdr = 0x4EDD0;
	inline uint32_t Face2RedirectAdr() { return m_romRegion == 'E' ? naFace2RedirectAdr : euFace2RedirectAdr; }

	static constexpr uint32_t naStringInitRedirectAdr = 0x4D0DC;
	static constexpr uint32_t euStringInitRedirectAdr = 0x4EDD0; //TODO
	inline uint32_t StringInitRedirectAdr() { return m_romRegion == 'E' ? naStringInitRedirectAdr : euStringInitRedirectAdr; }

	static constexpr uint32_t naStringGetRedirectAdr = 0x4D474;
	static constexpr uint32_t euStringGetRedirectAdr = 0x4EDD0; //TODO
	inline uint32_t StringGetRedirectAdr() { return m_romRegion == 'E' ? naStringGetRedirectAdr : euStringGetRedirectAdr; }

	/*static constexpr uint32_t emptySpace1Start = 0x2589400;
	static constexpr uint32_t emptySpace1End = 0x258d000;
	static constexpr uint32_t emptySpace1Size = emptySpace1End - emptySpace1Start;

	static constexpr uint32_t emptySpace2Start = 0x16061c0;	//eu 1606560
	static constexpr uint32_t emptySpace2End = 0x1638000;
	static constexpr uint32_t emptySpace2Size = emptySpace2End - emptySpace2Start;*/


	struct EmptySpace {
		uint32_t offStart;
		uint32_t offEnd;
		uint32_t size;
		constexpr EmptySpace(uint32_t offStart, uint32_t offEnd) : offStart(offStart), offEnd(offEnd), size(offEnd - offStart) {}
	};
	static const inline EmptySpace emptySpaces[] = {
		{ 0x2589400, 0x258d000 },
		{ 0x1606560, 0x1638000 },
		{ 0x2222600, 0x2230000 },
		{ 0x225a820, 0x2268000 },
		{ 0x188a900, 0x1898000 },
	};

	/*
	FF-areas
	0x16061c0 - 0x1638000 (0x31e40)
	0x16fb090 - 0x1708000 (0xcf70)
	0x188a8cc - 0x1898000 (0xd734)
	0x1d63410 - 0x1d70000 (0xcbf0)
	0x1e2de72 - 0x1e40000 (0x1218e)
	0x1e69154 - 0x1e70000 (0x6eac)
	0x1ffbee0 - 0x2000000 (0x4120)
	0x2222590 - 0x2230000 (0xda70)
	0x225a817 - 0x2268000 (0xd7e9)
	0x2589400 - 0x258d000 (0x3c00)
	0x2668100 - 0x266b000 (0x2f00)
	0x2673668 - 0x2675000 (0x1998)
	0x27e9e20 - 0x27ed000 (0x31e0)
	0x2d77c80 - 0x2d7d000 (0x5380)
	0x3fd1b30 - 0x3fd5000 (0x34d0)
	*/
};

