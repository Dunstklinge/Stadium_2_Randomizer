// CMainDialog.cpp : implementation file
//

#include "CMainDialog.h"
#include "afxdialogex.h"
#include "resource.h"

#include <iostream>
#include <vector>
#include <deque>
#include <functional>

#include "CMain.h"
#include "CDistDialog.h"
#include "DefRoster.h"
#include "DefText.h"
#include "PokemonGenerator.h"
#include "TrainerGenerator.h"
#include "Tables.h"
#include "Filters.h"
#include "GlobalRandom.h"
#include "Crc.h"
#include "Randomizer.h"
#include "GlobalConfigs.h"


// CMainDialog dialog

IMPLEMENT_DYNAMIC(CMainDialog, CDialogEx)


constexpr unsigned int CMainDialog::WM_WORKERENDED;

CMainDialog::CMainDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CMainDialog::~CMainDialog()
{
	delete currRandomizer;
}


BOOL CMainDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	((CButton*)this->GetDlgItem(IDC_CB_TRAINER_RANDLEVELS))->SetCheck(0);
	((CButton*)this->GetDlgItem(IDC_CB_TRAINER_RANDMONNAMES))->SetCheck(1);
	((CButton*)this->GetDlgItem(IDC_CB_TRAINER_RANDNAME))->SetCheck(1);
	((CButton*)this->GetDlgItem(IDC_CB_TRAINER_RANDSPECIES))->SetCheck(1);

	((CButton*)this->GetDlgItem(IDC_CBRANDCHOOSEITEMS))->SetCheck(1);
	((CButton*)this->GetDlgItem(IDC_CBRANDMOVES))->SetCheck(1);
	((CButton*)this->GetDlgItem(IDC_CBRANDLEVEL))->SetCheck(0);
	((CButton*)this->GetDlgItem(IDC_CBRANDOMEVIV))->SetCheck(1);

	((CButton*)this->GetDlgItem(IDC_CBBATTLEITEMS))->SetCheck(1);

	((CEdit*)this->GetDlgItem(IDC_EDCHANGEITEMN))->SetWindowText(TEXT("8"));
	((CButton*)this->GetDlgItem(IDC_RANDPRIMELEVELS))->SetCheck(0);
	((CButton*)this->GetDlgItem(IDC_CBMULTIPLEGLCRENTALS))->SetCheck(0);
	((CEdit*)this->GetDlgItem(IDC_EDGLCLEVEL))->SetWindowText(TEXT("75"));
	((CButton*)this->GetDlgItem(IDC_CBSPECIESEVIV))->SetCheck(1);
	((CButton*)this->GetDlgItem(IDC_CBBOSSSTAYCLOSETOBST))->SetCheck(1);

	((CButton*)this->GetDlgItem(IDC_CBRANDOMRENTALS))->SetCheck(1);
	((CButton*)this->GetDlgItem(IDC_CB_TRAINER_RANDOM))->SetCheck(1);
	((CButton*)this->GetDlgItem(IDC_CB_TRAINER_RANDPOKE))->SetCheck(1);
	((CButton*)this->GetDlgItem(IDC_CB_TRAINER_RANDMOVES))->SetCheck(1);
	
	((CComboBox*)this->GetDlgItem(IDC_CBSPECIAL));
	CString customTrainerN;
	customTrainerN.Format(TEXT("%u"), GlobalConfig::CustomTrainers.customTrainers.size());
	((CEdit*)this->GetDlgItem(IDC_EDCUSTOMTRAINERN))->SetWindowText(customTrainerN);

	((CButton*)this->GetDlgItem(IDC_BTNGENERATE))->EnableWindow(FALSE);

	((CSpinButtonCtrl*)this->GetDlgItem(IDC_SPIN1))->SetRange32(0, 100);
	((CSpinButtonCtrl*)this->GetDlgItem(IDC_SPIN2))->SetRange32(0, 100);
	((CSpinButtonCtrl*)this->GetDlgItem(IDC_SPIN3))->SetRange32(0, 100);
	((CSpinButtonCtrl*)this->GetDlgItem(IDC_SPIN4))->SetRange32(0, 100);


	std::random_device dev;
	TCHAR seedStr[256];
	_ui64tot_s(dev(), seedStr, _countof(seedStr), 10);
	edSeed.SetWindowText(seedStr);
	((CButton*)this->GetDlgItem(IDC_EDPRIMECUPLEVEL))->SetWindowText(TEXT("95"));
	((CButton*)this->GetDlgItem(IDC_EDRPTADDPERCENT))->SetWindowText(TEXT("50"));
	((CButton*)this->GetDlgItem(IDC_EDRPTREMPERCENT))->SetWindowText(TEXT("50"));
	((CButton*)this->GetDlgItem(IDC_EDRMSECADDPERCENT))->SetWindowText(TEXT("50"));
	((CButton*)this->GetDlgItem(IDC_EDRMSECREMPERCENT))->SetWindowText(TEXT("50"));

	RefreshEnableStates();


	AddTooltip(IDC_STATIC, TEXT("Changes from this part apply to both rentals and trainer pokemon."));
	
	AddTooltip(IDC_RD1NOTHING, TEXT("Does not put a restriction on the first move."));
	AddTooltip(IDC_RD1ATK, TEXT("At least one move of each pokemon has a base power >= 1."));
	AddTooltip(IDC_RD1STAB, TEXT("At least one move of each pokemon has a base power >= 1 and is stab."));
	AddTooltip(IDC_RD1STRONGATK, TEXT("At least one move of each pokemon has a base power >= 75."));
	AddTooltip(IDC_RD1STRONGSTAB, TEXT("At least one move of each pokemon has a base power >= 75 and is stab."));
	AddTooltip(IDC_RDYOLONOME, TEXT("All Moves of all Pokemon are Metronome."));

	AddTooltip(IDC_CBLEGALMOVESONLY, TEXT("All Moves of all Pokemon must be legal, except if Metronome was selected. ")
		TEXT("Above restrictions still apply as long as at least one such move exists."));
	AddTooltip(IDC_RANDPRIMELEVELS, TEXT("Levels of both rentals and enemy pokemon in prime cup will be between this level and 100. ")
		TEXT(" (Trainers only if \"Random Levels\" is checked in the Trainer category."));

	AddTooltip(IDC_CB_TRAINER_RANDOM, TEXT("Enables or Disables the Randomization of Trainer Properties (expect their Pokemon)"));
	AddTooltip(IDC_CB_TRAINER_RANDNAME, TEXT("Trainers will be given random names taken from the appropriate config file."));
	AddTooltip(IDC_CBSHUFFLEBOSS, TEXT("Gym Leaders, Elite 4 and Rivals will be shuffled around so that e.g a new set of 4 ")
		TEXT("bosses will make up the elite 4. Both Round 1 and Round 2 will be shuffled independently. ")
		TEXT("This only shuffles their text and faces, but not their pokemon."));
	AddTooltip(IDC_CBSHUFFLEREGULAR, TEXT("Non-Bosses will be shuffled around like the bosses. ")
		TEXT("This only shuffles their text and faces, but not their pokemon."));
	AddTooltip(IDC_CBSHUFFLECROSS, TEXT("If active, random trainers can take the place of bosses and vise-versa."));
	AddTooltip(IDC_CBSHUFFLECUSTOMS, TEXT("Custom Trainers from config/trainers may replace random bosses. This ")
		TEXT("includes all of their appearances in both rounds. Only affects text and faces, but not their pokemon. "));
	AddTooltip(IDC_CBSHUFFLECUSTOMSINTOREGS, TEXT("Custom Trainers from config/trainers may replace random regular non-boss trainers. This ")
		TEXT("includes all of their appearances in both rounds. Only affects text and faces, but not their pokemon. "));
	
	AddTooltip(IDC_CBRANDOMRENTALS, TEXT("Enables or Disables Rental Modification."));
	AddTooltip(IDC_CBRANDMOVES, TEXT("Rentals have random moves."));
	AddTooltip(IDC_CBRANDMOVES_BALANCED, TEXT("Stronger rentals will have weaker moves, like in the original stadium."));
	AddTooltip(IDC_CBRANDLEVEL, TEXT("Rentals for the Pokecup have a random level between 50 and 55 instead of always being 50."));
	AddTooltip(IDC_CBRANDHAPPINESS, TEXT("Rentals have random happiness."));
	AddTooltip(IDC_CBRANDOMEVIV, TEXT("Rental Pokemon have randomized gene strength."));
	AddTooltip(IDC_CBSPECIESEVIV, TEXT("Random Ivs/Evs are limited to a certain Range which depends on ")
		TEXT("the pokemons BST: weak pokemon will have high Ivs/Evs, strong Pokemon lower ones."));

	AddTooltip(IDC_CBMORERENTALTABLES, TEXT("Uses assembly hacks to add additional rental sets that the original game does not have. ")
		TEXT("\r\nNOTE: Rental registration does not follow these bounds and may be used to cheese."));
	AddTooltip(IDC_CBPOKECUPRENTALS, TEXT("The 4 Poke-Cups (Pokeball to Masterball) have their own rental sets. "));
	AddTooltip(IDC_CBR2RENTALS, TEXT("All rental sets are different in round 2. This includes rental sets added ")
		TEXT("by this hack such as seperate PokeCup or GLC rentals."));
	AddTooltip(IDC_CBMULTIPLEGLCRENTALS, TEXT("Adds a new set of rentals for the gym leader castle. Originally, the GLC ")
		TEXT("used PokeCup rentals, which may be a bad thing combined with PokeCup rentals random levels. ")
		TEXT("This option adds seperate Sets for Falkner-Chuck, Jasmine-Lance and Kanto"));
	AddTooltip(IDC_CBGLCRENTALLEVELS, TEXT("Sets level of GLC rentals as well as the Rivals Pokemon. ")
		TEXT("\r\nNOTE: these include ubers. This can not be turned off; if you dont want to use ubers, please ignore them."));
	
	AddTooltip(IDC_CBRANDCHOOSEITEMS, TEXT("Uses assembly hacks to change the items choosable in the rental screen and/or ")
		TEXT("add additional sets of items for the rental selection."));
	AddTooltip(IDC_CBMONSPECIFICITEMS, TEXT("If unchecked, only general battle-items such as berries and ")
		TEXT("quick claw are used. If checked, type-boosting items such as magnet and pokemon-specific items ")
		TEXT("such as thick club are used as well."));
	AddTooltip(IDC_CBCHANGEITEMN, TEXT("Changes the amount of items avialable per individual item set. The original game has 6."));
	AddTooltip(IDC_CBITEMSPERRENTAL, TEXT("Every rental set, including ones generated by this hack with the options above, have ")
		TEXT("their own item set."));
	
	AddTooltip(IDC_CB_TRAINER_RANDPOKE, TEXT("Trainers Teams will be modified."));
	AddTooltip(IDC_CB_TRAINER_RANDSPECIES, TEXT("Trainers have random Pokemon species (within the bounds of the options below)."));
	AddTooltip(IDC_CB_TRAINER_RANDLEVELS, TEXT("Trainers in PokeCup have random levels bewteen 50 and 55 for their pokemon. ")
		TEXT("Trainers in PrimeCup have random levels if this was selected in the All-Pokemon category."));
	AddTooltip(IDC_CB_TRAINER_RANDMOVES, TEXT("Trainers Pokemon have random moves."));
	AddTooltip(IDC_RD_TRAINER_RANDMOVES_FULLRAND, TEXT("The moves are entirely random; no move is more likely than any other."));
	AddTooltip(IDC_RD_TRAINER_RANDMOVES_BALANCED, TEXT("Similar to the rental setting, stronger pokemon will tend to have weaker moves."));
	AddTooltip(IDC_RD_TRAINER_RANDMOVES_STAYCLOSE, TEXT("Pokemon that had generally more powerfull moves in the original game will ")
		TEXT("generally randomize to have strong moves as well."));
	AddTooltip(IDC_CB_TRAINER_RANDHAPPINESS, TEXT("Trainers Pokemon have random happiness."));
	AddTooltip(IDC_CB_TRAINER_RANDITEMS, TEXT("Trainers Pokemon have random items."));
	AddTooltip(IDC_CBBATTLEITEMS, TEXT("If unchecked, pokemon may have any item, including useless ones such as ")
		TEXT("TMs or Repels. If checked, only useful items are generated."));
	AddTooltip(IDC_CB_TRAINER_RANDEVIV, TEXT("Trainers Pokemon have Evs and Ivs."));
	AddTooltip(IDC_CB_TRAINER_RANDMONNAMES, TEXT("Trainers have random nicknames for their Pokemon taken from the appropriate config file."));
	AddTooltip(IDC_CB_TRAINERPOKE_MIN1ATK, TEXT("Every trainer pokemon has at least one move with base power > 0. ")
		TEXT("\r\nNOTE: consider that you can pick the best 6 from 250 random pokemon, but trainers can only pick 6 from 6."));
	AddTooltip(IDC_CBSTAYCLOSETOBST, TEXT("Prevents regular trainers from having Mewtwos."));
	AddTooltip(IDC_CBBOSSSTAYCLOSETOBST, TEXT("Prevents Rival from having a weedle and a magikarp"));

	AddTooltip(IDC_CBUBERTYPES, TEXT("Changes Types of pokemon Species to random types, excluding the ??? type"));
	AddTooltip(IDC_CBUBERBST, TEXT("Randomize species stats"));
	AddTooltip(IDC_CBRPBSTEVOBST, TEXT("Prevents e.g Kadabra from having lower BST than Abra"));
	AddTooltip(IDC_CBRPBSTEVOSTATS, TEXT("Prevents e.g Blastoise from having lower defense than Squirtle.")
		TEXT("\r\nNOTE: this also applies to pokemon like Scizor and Beedrill which usually do not follow these rules."));
	AddTooltip(IDC_RDRPBSTKEEPBST, TEXT("Species keep their BST, but their stat distribution gets changed."));
	AddTooltip(IDC_RDRPBSTCLOSEBST, TEXT("BST will only get changed += 10%"));
	AddTooltip(IDC_RDRPBSTRANDBST, TEXT("BST is completely random"));
	
	AddTooltip(IDC_CBUBERMOVES, TEXT("Randomize move effects. Could e.g turn tackle into a multi-hit poison type move."));
	AddTooltip(IDC_CBRMBALANCE, TEXT("Tries to keep moves equally strong according to a point system."));
	AddTooltip(IDC_CBRMTYPE, TEXT("Random move type, both for attack and status moves."));
	AddTooltip(IDC_CBRMBP, TEXT("Random BP for moves that have a BP"));
	AddTooltip(IDC_CBRMCLOSEBP, TEXT("BP gets only changed += 10% from the original BP."));
	AddTooltip(IDC_CBRMSEC, TEXT("Attacking moves (i.e moves that have a BP) get a random secondary effects. The following restrictions apply:\r\n")
		TEXT("If no random bp was selected, damage moves that ignore BP (like OHKO moves or Counter) are not randomised.\r\n")
		TEXT("If no random effect chance was selected, moves with chance-effects (e.g 10% burn) are only given other chance effects ")
		TEXT("and vice versa"));
	AddTooltip(IDC_CBRMEFFECTCHANCE, TEXT("Random chance for secondary chance-effects."));
	AddTooltip(IDC_CBRMRANDSTATUS, TEXT("Status moves (i.e moves with no BP) get a random effect."));
	AddTooltip(IDC_CBRMPP, TEXT("Random power point maximum."));
	AddTooltip(IDC_CBRMACC, TEXT("Random Accuracy for all moves (except self-targeted status moves)."));
		
	AddTooltip(IDC_PATCHCIC, TEXT("When ticked, the ROMS CRC are not recalculated, but the bootcode is modified to skip the check instead."));

	AddTooltip(IDC_BTNGENERATE, TEXT("Click to generate a randomized rom."));
	AddTooltip(IDC_BTNLOADPRESET, TEXT("Click to select and load a set of randomizer settings that were previously saved."));
	AddTooltip(IDC_BTNSAVEPRESET, TEXT("Click to select and save a set of randomizer settings that can be loaded later."));
	

	progressBar.SetRange(0, 1000);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainDialog::AddTooltip(int dlgId, const TCHAR* tooltip)
{
	HWND wnd = this->GetDlgItem(dlgId)->m_hWnd;

	HWND wndTooltip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		this->m_hWnd, NULL, program.m_hInstance, NULL);

	::SendMessage(wndTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 0x7FFF);//_tcslen(tooltip) * 100);
	::SendMessage(wndTooltip, TTM_SETMAXTIPWIDTH, 0, 500);

	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = this->m_hWnd;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)wnd;
	toolInfo.lpszText = (TCHAR*)tooltip;
	::SendMessage(wndTooltip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

//adapter for int -> bool
void AFXAPI DDX_Check(CDataExchange* pDX, int nIDC, bool& value) {
	int ival = value;
	DDX_Check(pDX, nIDC, ival);
	value = ival;
}

void CMainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CBBATTLEITEMS, data.trainerMons.battleItemsOnly);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDLEVELS, data.trainerMons.randLevels);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDMONNAMES, data.trainerMons.randMonNames);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDNAME, data.trainers.randName);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDSPECIES, data.trainerMons.randSpecies);
	DDX_Check(pDX, IDC_CBRANDLEVEL, data.rentals.randLevels);
	DDX_Check(pDX, IDC_CBRANDOMEVIV, data.rentals.randEvIv);
	DDX_Control(pDX, IDC_EDPATH, romPath);
	DDX_Control(pDX, IDC_STSTATUS, stStatus);
	int min1Buttons;
	DDX_Radio(pDX, IDC_RD1NOTHING, min1Buttons);
	data.min1Buttons = min1Buttons;
	DDX_Check(pDX, IDC_RANDPRIMELEVELS, data.rentals.randPrimecupLevels);
	DDX_Control(pDX, IDC_PROGRESS, progressBar);
	DDX_Control(pDX, IDC_EDSEED, edSeed);
	{
		CString strSeed;
		edSeed.GetWindowText(strSeed);
		unsigned long seed = _tcstoul(strSeed, NULL, 10);
		data.seed = seed;
	}
	DDX_Check(pDX, IDC_CBRANDCHOOSEITEMS, data.rentals.randChooseItems);
	DDX_Text(pDX, IDC_EDPRIMECUPLEVEL, data.rentals.strPrimeCupLevel);
	DDX_Check(pDX, IDC_CBMONSPECIFICITEMS, data.rentals.randIncludeMonspecificItems);
	DDX_Check(pDX, IDC_CBGLCRENTALLEVELS, data.rentals.changeGlcRentalLevel);
	DDX_Text(pDX, IDC_EDGLCLEVEL, data.rentals.seperateGlcRentalsLevel);
	DDX_Check(pDX, IDC_CBSPECIESEVIV, data.rentals.rentalSpeciesEvIv);
	DDX_Check(pDX, IDC_CB_TRAINERPOKE_MIN1ATK, data.trainerMons.trainerMin1Atk);
	DDX_Check(pDX, IDC_CBSTAYCLOSETOBST, data.trainerMons.stayCloseToBST);
	DDX_Check(pDX, IDC_CBBOSSSTAYCLOSETOBST, data.trainerMons.bossStayCloseToBST);
	DDX_Check(pDX, IDC_CBCHANGEITEMN, data.rentals.changeItemN);
	DDX_Check(pDX, IDC_CBITEMSPERRENTAL, data.rentals.itemsPerRentalSet);
	DDX_Check(pDX, IDC_CBMORERENTALTABLES, data.rentals.moreRentalTables);
	DDX_Check(pDX, IDC_CBMULTIPLEGLCRENTALS, data.rentals.multipleGlcRentals);
	DDX_Check(pDX, IDC_CBPOKECUPRENTALS, data.rentals.multiplePokecupRentals);
	DDX_Check(pDX, IDC_CBR2RENTALS, data.rentals.multipleR2Rentals);
	DDX_Text(pDX, IDC_EDCHANGEITEMN, data.rentals.changeItemNAmount);
	DDX_Check(pDX, IDC_CBLEGALMOVESONLY, data.legalMovesOnly);
	DDX_Check(pDX, IDC_CBSHUFFLEBOSS, data.trainers.shuffleBosses);
	DDX_Check(pDX, IDC_CBSHUFFLECROSS, data.trainers.shuffleCross);
	DDX_Check(pDX, IDC_CBSHUFFLEREGULAR, data.trainers.shuffleRegulars);
	DDX_Check(pDX, IDC_CBSHUFFLECUSTOMS, data.trainers.mixCustomsInBosses);
	DDX_Check(pDX, IDC_CBSHUFFLECUSTOMSINTOREGS, data.trainers.mixCustomsInTrainers);
	DDX_Text(pDX, IDC_EDCUSTOMTRAINERN, data.trainers.strCustomTrainerN);
	DDX_Check(pDX, IDC_CBRANDMOVES, data.rentals.randMoves);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDMOVES, data.trainerMons.trainerRandMoves);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDEVIV, data.trainerMons.trainerRandEvIv);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDHAPPINESS, data.trainerMons.trainerRandHappiness);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDITEMS, data.trainerMons.trainerRandItems);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDPOKE, data.trainerMons.trainerRandPoke);
	DDX_Check(pDX, IDC_CBRANDHAPPINESS, data.rentals.randRentalHappiness);
	DDX_Check(pDX, IDC_CBRANDOMRENTALS, data.rentals.randRentals);
	DDX_Check(pDX, IDC_CB_TRAINER_RANDOM, data.trainerMons.trainerRand);
	DDX_Check(pDX, IDC_CBUBERMOVES, data.uber.moves.randomize);
	DDX_Check(pDX, IDC_CBRMBALANCE, data.uber.moves.balance);
	DDX_Check(pDX, IDC_CBRMACC, data.uber.moves.acc);
	DDX_Check(pDX, IDC_CBRMBP, data.uber.moves.bp);
	DDX_Check(pDX, IDC_CBRMCLOSEBP, data.uber.moves.closeBp);
	DDX_Check(pDX, IDC_CBRMPP, data.uber.moves.pp);
	DDX_Check(pDX, IDC_CBRMRANDSTATUS, data.uber.moves.statusMoves);
	DDX_Check(pDX, IDC_CBRMTYPE, data.uber.moves.type);
	DDX_Check(pDX, IDC_CBRMEFFECTCHANCE, data.uber.moves.secEffectChance);
	DDX_Check(pDX, IDC_CBRMSEC, data.uber.moves.secEffect);
	DDX_Check(pDX, IDC_CBUBERBST, data.uber.species.randomizeBst);
	DDX_Check(pDX, IDC_CBUBERTYPES, data.uber.species.randomizeTypes);
	DDX_Check(pDX, IDC_CBRPBSTEVOBST, data.uber.species.evoBst);
	DDX_Check(pDX, IDC_CBRPBSTEVOSTATS, data.uber.species.evoStats);
	int bstTypeButtons = data.uber.species.bstType;
	DDX_Radio(pDX, IDC_RDRPBSTKEEPBST, bstTypeButtons);
	data.uber.species.bstType = (decltype(data.uber.species.bstType))bstTypeButtons;
	DDX_Text(pDX, IDC_EDRMSECADDPERCENT, data.uber.moves.secEffectAddPercent);
	DDX_Text(pDX, IDC_EDRMSECREMPERCENT, data.uber.moves.secEffectRemPercent);
	DDX_Text(pDX, IDC_EDRPTADDPERCENT, data.uber.species.addTypePercent);
	DDX_Text(pDX, IDC_EDRPTREMPERCENT, data.uber.species.removeTypePercent);
	DDX_Check(pDX, IDC_PATCHCIC, patchCic);
	DDX_Check(pDX, IDC_CBRANDMOVES_BALANCED, data.rentals.randMovesBalanced);
	DDX_Radio(pDX, IDC_RD_TRAINER_RANDMOVES_FULLRAND, data.trainerMons.trainerRandMovesDetails);
}


BEGIN_MESSAGE_MAP(CMainDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BTNBROWSE, &CMainDialog::OnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTNGENERATE, &CMainDialog::OnClickedBtnGenerate)
	ON_EN_CHANGE(IDC_EDSEED, &CMainDialog::OnChangeEdSeed)
	ON_EN_CHANGE(IDC_EDPATH, &CMainDialog::OnChangeEdPath)
	ON_MESSAGE(CMainDialog::WM_WORKERENDED, &CMainDialog::OnRandomizeEnd)
ON_BN_CLICKED(IDC_CBRANDLEVEL_BTNDIST, &CMainDialog::OnBnClickedCbrandlevelBtndist)
ON_BN_CLICKED(IDC_CBRANDOMEVIV_BTNDIST, &CMainDialog::OnBnClickedCbrandomevivBtndist)
ON_BN_CLICKED(IDC_CB_TRAINER_RANDEVIV_BTNDIST, &CMainDialog::OnBnClickedCbTrainerRandevivBtndist)
ON_BN_CLICKED(IDC_BTNRPBSTDIST, &CMainDialog::OnBnClickedBtnrpbstdist)
ON_BN_CLICKED(IDC_BTNRMBPDIST, &CMainDialog::OnBnClickedBtnrmbpdist)
ON_BN_CLICKED(IDC_BTNRMPPDIST, &CMainDialog::OnBnClickedBtnrmppdist)
ON_BN_CLICKED(IDC_BTNRMACCDIST, &CMainDialog::OnBnClickedBtnrmaccdist)
ON_BN_CLICKED(IDC_BTNRMECDIST, &CMainDialog::OnBnClickedBtnrmecdist)
ON_BN_CLICKED(IDC_CBRANDMOVES_BTNDIST, &CMainDialog::OnBnClickedCbrandmovesBtndist)
ON_BN_CLICKED(IDC_CB_TRAINER_RANDMOVES_BTNDIST, &CMainDialog::OnBnClickedCbTrainerRandmovesBtndist)
ON_BN_CLICKED(IDC_BTNLOADPRESET, &CMainDialog::OnBnClickedBtnloadpreset)
ON_BN_CLICKED(IDC_BTNSAVEPRESET, &CMainDialog::OnBnClickedBtnsavepreset)
ON_CONTROL_RANGE(BN_CLICKED, 1000, 9999, &CMainDialog::OnAnyBnChecked)
END_MESSAGE_MAP()


// CMainDialog message handlers
#include "DefFaces.h"

void CMainDialog::OnClickedBtnBrowse()
{
	const TCHAR filters[] = _T("n64 rom files|*.n64;*.z64;*.v64||");
	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_HIDEREADONLY, filters, this);
	if (dlg.DoModal() == IDOK) {
		romPath.SetWindowText(dlg.GetPathName());
	}

}
void CMainDialog::OnClickedBtnGenerate()
{
	UpdateData(true);

	DisableAll();

	delete currRandomizer;
	currRandomizer = new Randomizer(data);
	currRandomizerThread = AfxBeginThread(RandomizeThreadProc, this);
}

UINT CMainDialog::RandomizeThreadProc(LPVOID param) {
	CMainDialog* settings = (CMainDialog*)param;
	CString path;
	settings->romPath.GetWindowText(path);
	settings->currRandomizer->RandomizeRom(path, settings);
	settings->SendMessage(WM_WORKERENDED);
	return 0;
}

LRESULT CMainDialog::OnRandomizeEnd(WPARAM wparam, LPARAM lparam)
{
	RestoreAll();
	return 0;
}

void CMainDialog::DisableAll()
{
	::EnumChildWindows(this->m_hWnd, DisableAllChildProc, (LPARAM)this);
}

BOOL CALLBACK CMainDialog::DisableAllChildProc(HWND hwnd, LPARAM we) {
	CMainDialog* ptr = (CMainDialog*)we;
	TCHAR name[128];
	*name = 0;
	::GetClassName(hwnd, name, 128);
	if (_tcsicmp(name, "BUTTON") == 0 || _tcsicmp(name, "EDIT")) {
		::EnableWindow(hwnd, FALSE);
	}
	return TRUE;
}

void CMainDialog::RestoreAll() 
{
	RefreshEnableStates();
}

const std::map<int, CMainDialog::ControlNode> CMainDialog::uiControls = []() {
	std::map<int, CMainDialog::ControlNode> retVal;

	struct Rel {
		int pid;
		std::vector<Rel> children;
		Rel(int pid) : pid(pid) {}
		Rel(int pid, std::vector<Rel> children) : pid(pid), children(children) {}
		Rel& operator| (Rel rhs) {
			children.push_back(std::move(rhs));
			return *this;
		}
	};
	std::vector<Rel> rels = {
		Rel(IDC_RD1NOTHING)
		,Rel(IDC_RD1ATK)
		,Rel(IDC_RD1STAB)
		,Rel(IDC_RD1STRONGATK)
		,Rel(IDC_RD1STRONGSTAB)
		,Rel(IDC_RDYOLONOME)
		,Rel(IDC_CBLEGALMOVESONLY)
		,Rel(IDC_RANDPRIMELEVELS)
			| Rel(IDC_EDPRIMECUPLEVEL)
		,Rel(IDC_CB_TRAINER_RANDOM)
			| Rel(IDC_CB_TRAINER_RANDNAME)
			| (Rel(IDC_CBSHUFFLEBOSS)
				| Rel(IDC_CBSHUFFLECROSS)
			  )
			| (Rel(IDC_CBSHUFFLEREGULAR)
				| Rel(IDC_CBSHUFFLECROSS)
			  )
			| (Rel(IDC_CBSHUFFLECUSTOMS)
				| Rel(IDC_EDCUSTOMTRAINERN)
			  )
			| (Rel(IDC_CBSHUFFLECUSTOMSINTOREGS)
				| Rel(IDC_EDCUSTOMTRAINERN)
			  )
		,Rel(IDC_CBRANDOMRENTALS)
			| (Rel(IDC_CBRANDMOVES)
				| (Rel(IDC_CBRANDMOVES_BALANCED)
					| Rel(IDC_CBRANDMOVES_BTNDIST)
				)
			  )
			| (Rel(IDC_CBRANDLEVEL)
				| Rel(IDC_CBRANDLEVEL_BTNDIST)
			  )
			| Rel(IDC_CBRANDHAPPINESS)
			| (Rel(IDC_CBRANDOMEVIV)
				| Rel(IDC_CBRANDOMEVIV_BTNDIST)
				| Rel(IDC_CBSPECIESEVIV)
			  )
			| (Rel(IDC_CBMORERENTALTABLES)
				| Rel(IDC_CBPOKECUPRENTALS)
				| Rel(IDC_CBR2RENTALS)
				| Rel(IDC_CBMULTIPLEGLCRENTALS)
				| (Rel(IDC_CBGLCRENTALLEVELS)
					| Rel(IDC_EDGLCLEVEL)
				  )
			  )
			| (Rel(IDC_CBRANDCHOOSEITEMS)
				| Rel(IDC_CBMONSPECIFICITEMS)
				| (Rel(IDC_CBCHANGEITEMN)
					| Rel(IDC_EDCHANGEITEMN)
				  )
				| Rel(IDC_CBITEMSPERRENTAL)
			  )
		,Rel(IDC_CB_TRAINER_RANDPOKE)
			| Rel(IDC_CB_TRAINER_RANDSPECIES)
			| (Rel(IDC_CB_TRAINER_RANDMOVES)
				| Rel(IDC_RD_TRAINER_RANDMOVES_FULLRAND)
				| (Rel(IDC_RD_TRAINER_RANDMOVES_BALANCED)
					| Rel(IDC_CB_TRAINER_RANDMOVES_BTNDIST))
				| (Rel(IDC_RD_TRAINER_RANDMOVES_STAYCLOSE)
					| Rel(IDC_CB_TRAINER_RANDMOVES_BTNDIST))
			  )
			| Rel(IDC_CB_TRAINER_RANDLEVELS)
			| (Rel(IDC_CB_TRAINER_RANDITEMS)
				| Rel(IDC_CBBATTLEITEMS)
			  )
			| (Rel(IDC_CB_TRAINER_RANDEVIV)
				| Rel(IDC_CB_TRAINER_RANDEVIV_BTNDIST)
			  )
			| Rel(IDC_CB_TRAINER_RANDHAPPINESS)
			| Rel(IDC_CB_TRAINER_RANDMONNAMES)
			| Rel(IDC_CB_TRAINERPOKE_MIN1ATK)
			| Rel(IDC_CBSTAYCLOSETOBST)
			| Rel(IDC_CBBOSSSTAYCLOSETOBST)
		,Rel(IDC_CBUBERTYPES)
			| Rel(IDC_EDRPTADDPERCENT)
			| Rel(IDC_EDRPTREMPERCENT)
		,Rel(IDC_CBUBERBST)
			| Rel(IDC_BTNRPBSTDIST)
			| Rel(IDC_CBRPBSTEVOBST)
			| Rel(IDC_CBRPBSTEVOSTATS)
			| Rel(IDC_RDRPBSTKEEPBST)
			| Rel(IDC_RDRPBSTCLOSEBST)
			| Rel(IDC_RDRPBSTRANDBST)
		,Rel(IDC_CBUBERMOVES)
			| Rel(IDC_CBRMBALANCE)
			| Rel(IDC_CBRMTYPE)
			| (Rel(IDC_CBRMBP)
				| Rel(IDC_CBRMCLOSEBP)
				| Rel(IDC_BTNRMBPDIST)
			  )
			| Rel(IDC_CBRMSEC)
			| (Rel(IDC_CBRMEFFECTCHANCE)
				| Rel(IDC_BTNRMECDIST)
				| Rel(IDC_EDRMSECADDPERCENT)
				| Rel(IDC_EDRMSECREMPERCENT)
			  )
			| Rel(IDC_CBRMRANDSTATUS)
			| (Rel(IDC_CBRMPP)
				| Rel(IDC_BTNRMPPDIST)
			  )
			| (Rel(IDC_CBRMACC)
				| Rel(IDC_BTNRMACCDIST)
			  )
	};

	struct queueElem {
		const Rel& rel;
		const Rel* parent = nullptr;
	};
	std::deque<queueElem> treeNodes;
	for (const Rel& rel : rels) {
		queueElem elem{ rel, nullptr };
		treeNodes.push_back(elem);
	}

	while (!treeNodes.empty()) {
		queueElem elem = treeNodes.front();
		const Rel& rel = elem.rel;
		treeNodes.pop_front();

		CMainDialog::ControlNode& node = retVal[rel.pid];
		node.cid = rel.pid;
		if (elem.parent)
			node.parents.push_back(elem.parent->pid);
		for (const Rel& child : rel.children) {
			node.children.push_back(child.pid);
			queueElem elem{ child, &rel };
			treeNodes.push_front(elem);
		}
	}
	return retVal;
}();

void CMainDialog::RefreshEnableStates() 
{
	//start with top level nodes;
	//when recursive down into children, passing the decision from the parent
	for (auto& mapElem : uiControls) {
		auto& node = mapElem.second;
		if (!node.parents.empty()) continue;
		int id = node.cid;
		CButton* box = ((CButton*)this->GetDlgItem(id));
		for (int child : node.children) {
			RefreshEnableState_Child(uiControls.at(child), true);
		}
		
	}
}

void CMainDialog::RefreshEnableState_Child(const ControlNode& node, bool parentEnabled) {
	CButton* box = ((CButton*)this->GetDlgItem(node.cid));
	if (!parentEnabled) {
		box->EnableWindow(false);
		for (int child : node.children) {
			RefreshEnableState_Child(uiControls.at(child), false);
		}
	}
	else if(!node.parents.empty() ){
		bool all = true;
		bool any = false;
		for (int parent : node.parents) {
			CButton* parentBox = ((CButton*)this->GetDlgItem(parent));
			bool checked = parentBox->GetCheck() == BST_CHECKED;
			any |= checked;
			all &= checked;
		}
		box->EnableWindow(any);
		for (int child : node.children) {
			RefreshEnableState_Child(uiControls.at(child), any);
		}
	}
	else {
		box->EnableWindow(true);
		for (int child : node.children) {
			RefreshEnableState_Child(uiControls.at(child), true);
		}
	}
}

void CMainDialog::OnAnyBnChecked(UINT id) {
	RefreshEnableStates();
}

void CMainDialog::OnChangeEdSeed()
{
	CString strSeed;
	edSeed.GetWindowText(strSeed);
	unsigned long seed = _tcstoul(strSeed, NULL, 10);
	data.seed = seed;
	Random::Generator.seed(seed);
}




void CMainDialog::OnChangeEdPath()
{
	bool valid = false;
	CString path;
	romPath.GetWindowText(path);

	DWORD attr = GetFileAttributes(path);

	char romRegion = '?';
	if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
		std::ifstream in(path, std::ios::binary);
		uint8_t buffer[0x40];
		in.read((char*)buffer, 0x40);
		in.seekg(0, in.end);
		unsigned int size = (unsigned int)in.tellg();
		if (size == 0x4000000) {
			uint8_t firstByte = buffer[0];
			bool m_normal = firstByte == 0x80, m_byteswapped = firstByte == 0x37, m_wordswapped = firstByte == 0x40;
			if (m_normal || m_byteswapped || m_wordswapped) {
				auto DoSwaps = [&](void* buffer, size_t size) {
					uint8_t* bBuffer = (uint8_t*)buffer;
					if (m_normal) return;
					else if (m_byteswapped) for (size_t i = 0; i < size; i += 2) SwitchEndianness(*((uint16_t*)(bBuffer + i)));
					else if (m_wordswapped) for (size_t i = 0; i < size; i += 4) SwitchEndianness(*((uint32_t*)(bBuffer + i)));
				};
				DoSwaps(buffer, 0x40);
				char* gameName = (char*)buffer + 0x20;
				romRegion = *((char*)buffer + 0x3E);
				if (strcmp(gameName, "POKEMON STADIUM 2   ") == 0 && (romRegion == 'E' /*|| romRegion == 'P'*/)) {
					valid = true;
				}
			}
		}
	}

	if (valid) {
		static TCHAR valid[] = "valid (X)";
		valid[7] = romRegion;
		stStatus.SetWindowText(valid);
		((CButton*)this->GetDlgItem(IDC_BTNGENERATE))->EnableWindow(TRUE);
	}
	else {
		static const TCHAR* invalid = "invalid!";
		stStatus.SetWindowText(invalid);
		((CButton*)this->GetDlgItem(IDC_BTNGENERATE))->EnableWindow(FALSE);
	}
}


template<typename... Args>
void CMainDialog::DistSelect(DiscreteDistribution& dist, bool bias, Args... p) {
	CDistDialog d(this, dist, bias, p...);
	if (d.DoModal() == IDOK) {
		dist = d.GetResultDDist();
	}
}

void CMainDialog::DistSelect(DiscreteDistribution& dist, bool bias) {
	CDistDialog d(this, dist, bias);
	if (d.DoModal() == IDOK) {
		dist = d.GetResultDDist();
	}
}

void CMainDialog::OnBnClickedCbrandlevelBtndist()
{
	DistSelect(data.rentals.randLevelsDist, false, data.rentals.randLevelDistBounds.first, data.rentals.randLevelDistBounds.second);
}


void CMainDialog::OnBnClickedCbrandomevivBtndist()
{
	UpdateData(true);
	DistSelect(data.rentals.randRelEvIvDist, data.rentals.rentalSpeciesEvIv);
}


void CMainDialog::OnBnClickedCbTrainerRandevivBtndist()
{
	DistSelect(data.trainerMons.trainerRandRelIvEvDist, false);
}


void CMainDialog::OnBnClickedBtnrpbstdist()
{
	DistSelect(data.uber.species.bstDist, false, data.uber.species.bstDistBounds.first, data.uber.species.bstDistBounds.second);
}


void CMainDialog::OnBnClickedBtnrmbpdist()
{
	UpdateData(true);
	if (data.uber.moves.closeBp) {
		DistSelect(data.uber.moves.bpRelDist, true);
	}
	else {
		DistSelect(data.uber.moves.bpDist, false, data.uber.moves.bpDistBounds.first, data.uber.moves.bpDistBounds.second);
	}
	
}


void CMainDialog::OnBnClickedBtnrmppdist()
{
	DistSelect(data.uber.moves.ppDist, false, data.uber.moves.ppDistBounds.first, data.uber.moves.ppDistBounds.second);
}


void CMainDialog::OnBnClickedBtnrmaccdist()
{
	DistSelect(data.uber.moves.accDist, false, data.uber.moves.accDistBounds.first, data.uber.moves.accDistBounds.second);
}


void CMainDialog::OnBnClickedBtnrmecdist()
{
	DistSelect(data.uber.moves.secEffectChanceDist, false, data.uber.moves.accDistBounds.first, data.uber.moves.accDistBounds.second);
}

void CMainDialog::OnBnClickedCbrandmovesBtndist()
{
	DistSelect(data.rentals.randRelMovesBalancedDist, true);
}


void CMainDialog::OnBnClickedCbTrainerRandmovesBtndist()
{
	DistSelect(data.trainerMons.trainerRandRelMovesDetailsDist, true);
}

const static TCHAR presetFilter[] = _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||");
void CMainDialog::OnBnClickedBtnloadpreset()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, presetFilter, this);
	if (dlg.DoModal() == IDOK) {
		HANDLE hfile = CreateFile(dlg.GetPathName(), FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hfile == NULL || hfile == INVALID_HANDLE_VALUE) {
			int errcode = GetLastError();
			TCHAR buf[256];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				buf, (sizeof(buf) / sizeof(TCHAR)), NULL);
			CString errMsg;
			errMsg.Format(TEXT("Could not open file:\r\n%s\r\nError code %d: %s"), dlg.GetPathName(), errcode, buf);
			MessageBox(errMsg, TEXT("Failed to open file"), MB_ICONERROR | MB_OK);
			return;
		}
		DWORD hi;
		DWORD size = GetFileSize(hfile, &hi);
		std::unique_ptr<char[]> fdata(new char[size]);
		ReadFile(hfile, fdata.get(), size, &hi, NULL);
		CloseHandle(hfile);

		RandomizationParams params;
		bool succ = params.Deserialize(std::string_view(fdata.get(), size));
		if (!succ) {
			MessageBox(TEXT("Failed to parse file. Make sure its a valid file that was created by a previous save opreation."),
				TEXT("Failed to parse file"), MB_ICONERROR | MB_OK);
			return;
		}
		this->data = params;
		UpdateData(false);
		RefreshEnableStates();
	}
}


void CMainDialog::OnBnClickedBtnsavepreset()
{
	UpdateData(true);
	CFileDialog dlg(FALSE, TEXT("txt"), NULL, OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, presetFilter, this);
	if (dlg.DoModal() == IDOK) {
		std::string str = this->data.Serialize();
		HANDLE hfile = CreateFile(dlg.GetPathName(), FILE_GENERIC_WRITE | FILE_GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
		if (hfile == NULL || hfile == INVALID_HANDLE_VALUE) {
			int errcode = GetLastError();
			TCHAR buf[256];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				buf, (sizeof(buf) / sizeof(TCHAR)), NULL);
			CString errMsg;
			errMsg.Format(TEXT("Could not open file:\r\n%s\r\nError code %d: %s"), dlg.GetPathName(), errcode, buf);
			MessageBox(errMsg, TEXT("Failed to open file"), MB_ICONERROR | MB_OK);
			return;
		}
		DWORD written;
		WriteFile(hfile, str.data(), str.size(), &written, NULL);
		CloseHandle(hfile);
	}
}
