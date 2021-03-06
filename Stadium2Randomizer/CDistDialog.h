#pragma once

#include "DiscreteDistribution.h"

class CDistDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CDistDialog)

public:
	CDistDialog(CWnd* pParent, DiscreteDistribution dist);
	CDistDialog(CWnd* pParent, DiscreteDistribution dist, int minMin, int maxMax);
	virtual ~CDistDialog();

	DiscreteDistribution dist;
	bool useMinMax = false;
	int minMin, maxMax;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DISTRDISTDIALOG };
#endif

	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:

	CStatic stCanvas;
	CComboBox cbbType;
	UINT_PTR redrawTimer;
	bool timerStarted = false;

	static constexpr int DOUBLE_STEPS = 1000;
	static constexpr int REDRAW_TIMER = 16;

protected:

	struct VarCtrl {
		CDistDialog* owner;
		CStatic st;
		CEdit ed;
		CSpinButtonCtrl spin;
		CSliderCtrl slider;

		bool useDouble;
		union {
			struct {
				int* ival;
				int imin;
				int imax;
			};
			struct {
				double* dval;
				double dmin;
				double dmax;
			};
		};

		void Pair();
		void Reset(int* ival, int min, int max);
		void Reset(double* dval, double min, double max);
		void SetMinMax(int min, int max);
		void SetMinMax(double min, double max);
		void SetVal(int v, bool refreshEdit = true, bool refreshSpinner = true, bool refreshSlider = true);
		void SetVal(double v, bool refreshEdit = true, bool refreshSpinner = true, bool refreshSlider = true);
		
		void SyncFrom(bool edit, bool spinner, bool slider);

		inline void EnableWindow(BOOL bEnable) {
			ed.EnableWindow(bEnable);
			spin.EnableWindow(bEnable);
			slider.EnableWindow(bEnable);
		}
	};
	VarCtrl varMin;
	VarCtrl varMax;
	VarCtrl varCtrls[5];
	
	bool ignoreUpdates;

	class VarCtrlIt {
		CDistDialog& owner;
		int i;
	public:
		inline VarCtrlIt(CDistDialog& o, int i) : owner(o), i(i) {}
		inline bool operator==(const VarCtrlIt& rhs) { return owner == rhs.owner && i == rhs.i; }
		inline bool operator!=(const VarCtrlIt& rhs) { return !(*this == rhs); }
		inline VarCtrlIt& operator++() { i++; return *this; }
		inline VarCtrlIt operator++(int dummy) { auto copy = *this; i++; return copy; }
		inline VarCtrl& operator*() { if (i == 0) return owner.varMin; else if (i == 1) return owner.varMax; else return owner.varCtrls[i - 2]; }
		inline VarCtrl* operator->() { return &operator*(); }
	};
	inline VarCtrlIt VarCtrlBegin() { return VarCtrlIt{ *this, 0 }; }
	inline VarCtrlIt VarCtrlEnd() { return VarCtrlIt{ *this, 2 + _countof(varCtrls) }; }
public:
	afx_msg void OnCbnSelchangeCbbtype();
	void SwitchToUniform();
	void SwitchToNormal();
	void SwitchToTriangle();
	afx_msg void OnEditUpdated(UINT nId);
	afx_msg void OnSliderUpdated(UINT nId, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSpinUpdated(UINT nId, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
