#pragma once

#include "CMainDialog.h"

class CMain : public CWinApp
{
public:
	virtual BOOL InitInstance();

	CMainDialog* mainDialog;
};

extern CMain program;