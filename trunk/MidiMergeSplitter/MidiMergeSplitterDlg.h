// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		07mar25	initial version

*/

// MidiMergeSplitterDlg.h : header file
//

#pragma once

#include "afxcmn.h"

#include "GridCtrl.h"

// CMidiMergeSplitterDlg dialog
class CMidiMergeSplitterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMidiMergeSplitterDlg)
// Construction
public:
	CMidiMergeSplitterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MIDIMERGESPLITTER_DIALOG };

// Implementation
protected:
	HICON m_hIcon;

// Types
	class CModGridCtrl : public CGridCtrl {
	public:
		CModGridCtrl();
		virtual	CWnd*	CreateEditCtrl(LPCTSTR pszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
		virtual	void	OnItemChange(LPCTSTR pszText);
	};
	class CDeviceType {
	public:
// Construction
		CDeviceType();

// Operations
		void	InitGridControl();
		void	MakeDefaultRows(int nRows);
		bool	ReadState();
		void	WriteState();
		bool	OpenDevice(int iRow, int iDevice);
		void	InsertRow();
		void	InsertRow(int iRow);
		void	DeleteSelectedRows();
		void	DeleteRow(int iRow);
		void	DumpState();
		static int	GetCount(const CStringArrayEx& aStr, CString sVal);

// Member data
		CModGridCtrl m_grid;		// grid control
		CIntArrayEx	m_aRowDevSel;	// for each row, index of selected device, or -1 if none
		int		m_iDevType;			// index of this device type
	};

// Constants
	enum {	// grid columns
		COL_NUMBER,
		COL_DEVICE,
		COLUMNS
	};
	enum {	// device types
		INPUT = CMidiDevices::INPUT,
		OUTPUT = CMidiDevices::OUTPUT,
		DEVICE_TYPES = CMidiDevices::DEVICE_TYPES,
	};
	static const CGridCtrl::COL_INFO m_aColInfo[COLUMNS];
	static const LPCTSTR m_aDeviceTypeTag[DEVICE_TYPES];

// Data members
	CString	m_sNone;
	CDeviceType	m_aDevType[DEVICE_TYPES];	// array of device types

// Overrides
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();

// Helpers
	static bool	IsValidDeviceType(int iDevType);
	const CDeviceType&	GetDeviceType(int iDevType) const;
	CDeviceType&	GetDeviceType(int iDevType);
	void	InitDeviceTypes();
	void	InitGridControls();
	void	MakeDefaultRows(int nRows);
	bool	ReadState();
	void	WriteState();
	void	DumpState();
	CString	GetDeviceName(int iDevType, int iRow) const;
	bool	OpenDevice(int iDevType, int iRow, int iDevice);
	void	InsertRow(int iDevType);
	void	DeleteSelectedRows(int iDevType);
	int		GetFocusDeviceType() const;
	void	PopulateCombo(int iDevType, int iRow, CComboBox& combo) const;

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnDelayedCreate(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnDeviceChange(UINT nEventType, W64ULONG dwData);
	afx_msg LRESULT	OnDeviceNodeChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGetdispinfoList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickedInputsAddBtn();
	afx_msg void OnClickedInputsRemoveBtn();
	afx_msg void OnClickedOutputsAddBtn();
	afx_msg void OnClickedOutputsRemoveBtn();
	afx_msg void OnUpdateInputsRemoveBtn(CCmdUI *pCmdUI);
	afx_msg void OnUpdateOutputsRemoveBtn(CCmdUI *pCmdUI);
};

inline bool CMidiMergeSplitterDlg::IsValidDeviceType(int iDevType)
{
	return iDevType >= 0 && iDevType < DEVICE_TYPES;
}

inline const CMidiMergeSplitterDlg::CDeviceType& CMidiMergeSplitterDlg::GetDeviceType(int iDevType) const
{
	ASSERT(IsValidDeviceType(iDevType));
	return m_aDevType[iDevType];
}

inline CMidiMergeSplitterDlg::CDeviceType& CMidiMergeSplitterDlg::GetDeviceType(int iDevType)
{
	ASSERT(IsValidDeviceType(iDevType));
	return m_aDevType[iDevType];
}
