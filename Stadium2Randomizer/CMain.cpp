#include "CMain.h"

#include "DefRoster.h"
#include "Tables.h"
#include "GlobalRandom.h"
#include "TextMods.h"
#include "PokemonGenerator.h"
#include "TrainerGenerator.h"
#include "GlobalConfigs.h"

CMain program;

BOOL CMain::InitInstance()
{
	CWinApp::InitInstance();

	mainDialog = new CMainDialog();
	m_pMainWnd = mainDialog;

	GameInfo::InitTables();
	Random::Init();
	GlobalConfig::Init();


	mainDialog->DoModal();

	return TRUE;
}
