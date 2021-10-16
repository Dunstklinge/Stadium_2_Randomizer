#pragma once

#include "DiscreteDistribution.h"

#include <functional>

class CDistDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CDistDialog)

public:
	CDistDialog(CWnd* pParent, DiscreteDistribution dist, bool showBiasSlider);
	CDistDialog(CWnd* pParent, DiscreteDistribution dist, bool showBiasSlider, int minMin, int maxMax);
	virtual ~CDistDialog();

	void SwitchToUniform();
	void SwitchToNormal();
	void SwitchToTriangle();
	DiscreteDistribution GetResultDDist() { return ddist; }

protected:

	CStatic stCanvas;
	CComboBox cbbType;
	UINT_PTR redrawTimer;
	bool timerStarted = false;

	static constexpr int DOUBLE_STEPS = 1000;
	static constexpr int REDRAW_TIMER = 16;
	static constexpr int RELATIVE_MIN = 0;
	static constexpr int RELATIVE_MAX = 100;

	bool lockMinMax;
	bool showBiasSlider;
	DiscreteDistribution ddist;

	int minMin, maxMax;

	struct VarCtrl {
		CDistDialog* owner;
		CStatic st;
		CEdit ed;
		CSpinButtonCtrl spin;
		CSliderCtrl slider;

		bool useDouble;
		union {
			struct {
				int ival;
				int imin;
				int imax;
			};
			struct {
				double dval;
				double dmin;
				double dmax;
			};
		};
		std::function<void(int)> OnChangeInt;
		std::function<void(double)> OnChangeDouble;

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
	VarCtrl biasCtrl;
	
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
		inline VarCtrl& operator*() {
			if (i == 0) return owner.varMin; else if (i == 1) return owner.varMax;
			else if (i == 2) return owner.biasCtrl; else return owner.varCtrls[i - 3];
		}
		inline VarCtrl* operator->() { return &operator*(); }
	};
	inline VarCtrlIt VarCtrlBegin() { return VarCtrlIt{ *this, 0 }; }
	inline VarCtrlIt VarCtrlEnd() { return VarCtrlIt{ *this, 3 + _countof(varCtrls) }; }



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnCbnSelchangeCbbtype();
	afx_msg void OnEditUpdated(UINT nId);
	afx_msg void OnSliderUpdated(UINT nId, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSpinUpdated(UINT nId, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedDistEdgeclamp();
	afx_msg void OnBnClickedDistEdgereroll();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DISTRDISTDIALOG };
#endif
	afx_msg void OnBnClickedDistPropabilities();
	afx_msg void OnBnClickedDistExample();
};
