#pragma once

#include <vector>
#include <stdint.h>
#include <fstream>
#include <functional>
#include <array>

#include "Constants.h"
#include "DefRoster.h"
#include "DefText.h"
#include "CustomRosterInfo.h"
#include "CustomTrainerDefs.h"


class CMainDialog;

class Randomizer
{
public:
	Randomizer(CMainDialog* settings);
	~Randomizer();

	static void Randomize(const CString& path, CMainDialog* settings);
	void RandomizeRom(const CString& path);

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

	CMainDialog* m_settings;

	std::ofstream m_genLog;
	std::ifstream m_in;

	bool m_needsNewCrc;

	struct RomReplacements {
		uint8_t* buffer;
		uint32_t bufferSize;
		uint32_t romOffset;
	};
	std::vector<RomReplacements> m_romReplacements;

	DefRoster* m_romRoster;
	DefText* m_romText;
	std::vector<CustomRosterInfo>				m_customRInfoTable;		//sorted from largest to smallest
	std::vector<std::vector<uint8_t>>			m_customRentalTables;	//indexed by rentalOffset, which is used as index temporarily
																		//uint8_t -> DefRoster::PokemonList, i.e 4 bytes, then DefPokemon array
	std::vector<CustomItemInfo>					m_customIInfoTable;		//sorted from largest to smallest
	std::vector<std::vector<GameInfo::ItemId>>	m_customItemTables;		//indexed by itemPtr, which is used as index temporarily
	std::vector<CustomFaceInfo>					m_customFInfoTable;
	std::vector<std::vector<uint8_t>>			m_customFaceTable;		//list of custom faces to be inserted. uint8_t -> actually DefFace
	std::vector<CustomTrainerDefs::Trainer>		m_customTrainers;
	uint8_t* m_customItemRedirectCode;
	uint8_t* m_customItemInjectCode;
	uint8_t* m_customRentalRedirectCode;
	uint8_t* m_customRentalInjectCode;
	uint8_t* m_customFaceRedirectCode;
	uint8_t* m_customFaceInjectCode;

	CupRules m_cupRules;
	char m_romRegion;	//E = US, P = EU, rest unsupported

	uint32_t m_genItemTableOffset;
	uint32_t m_genRosterTableOffset;
	uint32_t m_genFaceTableOffset;


	bool AnalyseRom();
	CupRules GenerateCupRules();
	void RandomizeRegularRentals();
	void RandomizeHackedRentals();
	void RandomizeTrainers();
	void RandomizeItems();
	void SortInjectedData();

	void AddRival2Pokemon(DefTrainer& trainer);

	int m_progressBarMin, m_progressBarMax;
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

