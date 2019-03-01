#pragma once

#include "Constants.h"
#include "DefRoster.h"
#include "DefText.h"
#include "CustomRosterInfo.h"

#include <vector>
#include <stdint.h>
#include <fstream>
#include <functional>
#include <array>


class CMainDialog;

class Randomizer
{
public:
	Randomizer(CMainDialog* settings);
	~Randomizer();

	static void Randomize(const CString& path, CMainDialog* settings);

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
	std::vector<CustomRosterInfo>		 m_customRInfoTable;  //sorted from largest to smallest
	std::vector<std::vector<uint8_t>> m_customRentalTables;	  //indexed by rentalOffset, which is used as index temporarily
															  //uint8_t -> DefRoster::PokemonList, i.e 4 bytes, then DefPokemon array
	std::vector<CustomItemInfo>				   m_customIInfoTable; //sorted from largest to smallest
	std::vector<std::vector<GameInfo::ItemId>> m_customItemTables; //indexed by itemPtr, which is used as index temporarily
	uint8_t* m_customItemRedirectCode;
	uint8_t* m_customItemInjectCode;
	uint8_t* m_customRentalRedirectCode;
	uint8_t* m_customRentalInjectCode;

	CupRules m_cupRules;
	char m_romRegion;	//E = US, P = EU, rest unsupported

	uint32_t m_genItemTableOffset;
	uint32_t m_genRosterTableOffset;

	void RandomizeRom(const CString& path);

	bool AnalyseRom();
	CupRules GenerateCupRules();
	void RandomizeRegularRentals();
	void RandomizeHackedRentals();
	void RandomizeTrainers();
	void RandomizeItems();
	void SortInjectedData();

	int m_progressBarMin, m_progressBarMax;
	void SetProgress(double percent);
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

	static constexpr uint32_t emptySpace1Start = 0x2589400;
	static constexpr uint32_t emptySpace1End = 0x258d000;
	static constexpr uint32_t emptySpace1Size = emptySpace1End - emptySpace1Start;

	static constexpr uint32_t emptySpace2Start = 0x16061c0;
	static constexpr uint32_t emptySpace2End = 0x1638000;
	static constexpr uint32_t emptySpace2Size = emptySpace2End - emptySpace2Start;
};

