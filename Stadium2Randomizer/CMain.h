#pragma once

#include "CMainDialog.h"

#include <iostream>

class CMain : public CWinApp
{
	std::vector<TCHAR> launchDirectory;
public:
	virtual BOOL InitInstance();

	CMainDialog* mainDialog;
	const TCHAR* GetLaunchDirectory() {
		return launchDirectory.data();
	}
};

extern CMain program;