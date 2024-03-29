#pragma once
/*
* Note: Auto generated class.
* Thats why it looks so terrible.
* Best not to touch it.
* Should probably clean it up at some point, but we both know that will never happen
*/

#include <array>
#include <functional>
#include <vector>
#include <map>
#include <fstream>
#include <stdint.h>

#include "Constants.h"
#include "CustomRosterInfo.h"
#include "DiscreteDistribution.h"
#include "RandomizationParams.h"
#include "CDistDialog.h"

// CMainDialog dialog

class CMainDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CMainDialog)

public:
	CMainDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMainDialog();

	static constexpr unsigned int WM_WORKERENDED = WM_USER + 1;

	const RandomizationParams& GetParams() const { return data; }

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	struct ControlNode {
		int cid;
		std::vector<int> parents;
		std::vector<int> children;
		
	};
	static const std::map<int, ControlNode> uiControls;

	void AddTooltip(int dlgId, const TCHAR* tooltip);
	void DisableAll();
	void RestoreAll();
	static BOOL CALLBACK DisableAllChildProc(HWND hwnd, LPARAM we);
	static BOOL CALLBACK EnableAllChildProc(HWND hwnd, LPARAM we);
	void RefreshEnableStates();
	void RefreshEnableState_Child(const ControlNode& node, bool parentEnabled);

	friend class Randomizer;
	Randomizer* currRandomizer;
	CWinThread* currRandomizerThread;
	static UINT RandomizeThreadProc(LPVOID param);
	LRESULT OnProgress(WPARAM wparam, LPARAM lparam);
	LRESULT OnRandomizeEnd(WPARAM wparam, LPARAM lparam);

	template<typename... Args>
	void DistSelect(DiscreteDistribution& dist, bool bias, CDistDialog::BiasMapFunc* biasFunc, Args... p);
	void DistSelect(DiscreteDistribution& dist, bool bias, CDistDialog::BiasMapFunc* biasFunc);
public:

	afx_msg void OnClickedBtnBrowse();
	afx_msg void OnClickedBtnGenerate();

	afx_msg void OnChangeEdSeed();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEdPath();

	afx_msg void OnAnyBnChecked(UINT id);
	afx_msg void OnBnClickedCbrandlevelBtndist();
	afx_msg void OnBnClickedCbrandomevivBtndist();
	afx_msg void OnBnClickedCbTrainerRandevivBtndist();
	afx_msg void OnBnClickedBtnrpbstdist();
	afx_msg void OnBnClickedBtnrmbpdist();
	afx_msg void OnBnClickedBtnrmppdist();
	afx_msg void OnBnClickedBtnrmaccdist();
	afx_msg void OnBnClickedCbrandmovesBtndist();
	afx_msg void OnBnClickedCbTrainerRandmovesBtndist();
	afx_msg void OnBnClickedBtnrmecdist();
	afx_msg void OnBnClickedBtnloadpreset();
	afx_msg void OnBnClickedBtnsavepreset();
	afx_msg void OnBnClickedBtnusebstdist();
	afx_msg void OnBnClickedBtnbossbstdist();

protected:
	RandomizationParams data;
	BOOL patchCic; //not currently used

	DECLARE_MESSAGE_MAP()

	CEdit romPath;
	CStatic stStatus;

	CProgressCtrl progressBar;
	CEdit edSeed;
};
