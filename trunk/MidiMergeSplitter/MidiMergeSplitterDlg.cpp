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

// MidiMergeSplitterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MidiMergeSplitter.h"
#include "MidiMergeSplitterDlg.h"
#include "afxdialogex.h"
#include "AboutDlg.h"

#include "PopupCombo.h"
#include "Midi.h"
#include "dbt.h"	// for device change types

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDM_ABOUTBOX 0x0010

const CGridCtrl::COL_INFO CMidiMergeSplitterDlg::m_aColInfo[COLUMNS] = {
	{IDS_COL_NUMBER, LVCFMT_LEFT, 25},
	{IDS_COL_DEVICE, LVCFMT_LEFT, 300},
};

const LPCTSTR CMidiMergeSplitterDlg::m_aDeviceTypeTag[DEVICE_TYPES] = {
	_T("In"), 
	_T("Out")
};

// CMidiMergeSplitterDlg dialog

IMPLEMENT_DYNAMIC(CMidiMergeSplitterDlg, CDialogEx)

CMidiMergeSplitterDlg::CMidiMergeSplitterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMidiMergeSplitterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	InitDeviceTypes();	// essential; sets device index members
}

// CDeviceType: nested class to manage device types

inline CMidiMergeSplitterDlg::CDeviceType::CDeviceType()
{
	m_iDevType = -1;	// subtle reminder to call InitDeviceTypes
}

void CMidiMergeSplitterDlg::CDeviceType::InitGridControl()
{
	m_grid.CreateColumns(m_aColInfo, COLUMNS);
	m_grid.SetExtendedStyle(LVS_EX_FULLROWSELECT);
}

void CMidiMergeSplitterDlg::CDeviceType::MakeDefaultRows(int nRows)
{
	ASSERT(m_aRowDevSel.IsEmpty());	// valid during init only
	for (int iRow = 0; iRow < nRows; iRow++) {	// for each initial row
		m_aRowDevSel.Add(static_cast<DWORD>(-1));
	}
	m_grid.SetItemCountEx(m_aRowDevSel.GetSize(), 0);	// update grid control
}

bool CMidiMergeSplitterDlg::CDeviceType::ReadState()
{
	CString	sTypeKey(m_aDeviceTypeTag[m_iDevType]);
	int	nRows = theApp.GetProfileInt(sTypeKey, _T("Rows"), -1);	// read row count
	if (nRows < 0) {	// if missing or invalid row count
		return false;
	}
	m_aRowDevSel.SetSize(nRows);	// allocate device selection array
	CStringArrayEx	aDevName;
	CStringArrayEx	aDevID;
	aDevName.SetSize(nRows);
	aDevID.SetSize(nRows);
	// read row's state first
	for (int iRow = 0; iRow < nRows; iRow++) {	// for each row
		m_aRowDevSel[iRow] = -1;	// device is initially closed
		// create row's subkey
		CString	sRow;
		sRow.Format(_T("%d"), iRow);
		CString	sRowKey(sTypeKey + '\\' + sRow);
		// read row's device name and ID
		aDevName[iRow] = theApp.GetProfileString(sRowKey, _T("Name"));
		aDevID[iRow] = theApp.GetProfileString(sRowKey, _T("ID"));
	}
	m_grid.SetItemCountEx(m_aRowDevSel.GetSize(), 0);	// update grid control
	// now try opening devices
	for (int iRow = 0; iRow < nRows; iRow++) {	// for each row
		CString	sDevName(aDevName[iRow]);
		CString	sDevID(aDevID[iRow]);
		// try finding device by its name and ID first
		int	iDevice = theApp.m_midiDevs.Find(m_iDevType, sDevName, sDevID);
		if (iDevice < 0) {	// device not found
			int	nDevNameCount = theApp.m_midiDevs.GetNameCount(m_iDevType, sDevName);
			int	nRegNameCount = GetCount(aDevName, sDevName);
			// if name is unique in both system device list and our registry state
			if (nDevNameCount == 1 && nRegNameCount == 1) {
				// try finding device again by its name only
				iDevice = theApp.m_midiDevs.Find(m_iDevType, sDevName);
			}
		}
		OpenDevice(iRow, iDevice);	// open selected device
	}
	return true;
}

int	CMidiMergeSplitterDlg::CDeviceType::GetCount(const CStringArrayEx& aStr, CString sVal)
{
	int	nItems = aStr.GetSize();
	int	nCount = 0;
	for (int iItem = 0; iItem < nItems; iItem++) {
		if (aStr[iItem] == sVal) {
			nCount++;
		}
	}
	return nCount;
}

void CMidiMergeSplitterDlg::CDeviceType::WriteState()
{
	CString	sTypeKey(m_aDeviceTypeTag[m_iDevType]);
	int	nRows = m_aRowDevSel.GetSize();
	theApp.WriteProfileInt(sTypeKey, _T("Rows"), nRows);	// write row count
	for (int iRow = 0; iRow < nRows; iRow++) {	// for each row
		int	iDevice = m_aRowDevSel[iRow];
		CString	sName;
		CString	sID;
		if (iDevice >= 0) {	// if device is valid
			sName = theApp.m_midiDevs.GetName(m_iDevType, iDevice);
			sID = theApp.m_midiDevs.GetID(m_iDevType, iDevice);
		}
		// create row's subkey
		CString	sRow;
		sRow.Format(_T("%d"), iRow);
		CString	sRowKey(sTypeKey + '\\' + sRow);
		// write row's device name and ID
		theApp.WriteProfileString(sRowKey, _T("Name"), sName);
		theApp.WriteProfileString(sRowKey, _T("ID"), sID);
	}
}

bool CMidiMergeSplitterDlg::CDeviceType::OpenDevice(int iRow, int iDevice)
{
	// this method can also be used to close the device, by specifying -1 for iDevice
	int	iOldDevice = m_aRowDevSel[iRow];
	if (iDevice == iOldDevice) {	// if device selection didn't change
		return true;	// nothing to do
	}
	if (iOldDevice >= 0) {	// if old selection was a valid device
		theApp.CloseDevice(m_iDevType, iOldDevice);	// close old device
	}
	m_aRowDevSel[iRow] = iDevice;	// set row's device selection
	if (iDevice >= 0) {	// if new selection is a valid device
		if (!theApp.OpenDevice(m_iDevType, iDevice)) {	// open new device
			m_aRowDevSel[iRow] = -1;	// reset row's device selection
			return false;	// open failed
		}
	}
	DumpState();
	return true;	// open succeeded
}

void CMidiMergeSplitterDlg::CDeviceType::InsertRow()
{
	int	iInsRow = m_grid.GetSelection();
	if (iInsRow < 0) {	// if no selection
		iInsRow = m_grid.GetItemCount();	// add to end of list
	}
	InsertRow(iInsRow);
	DumpState();
}

void CMidiMergeSplitterDlg::CDeviceType::InsertRow(int iRow)
{
	ASSERT(iRow >= 0 && iRow <= m_grid.GetItemCount());
	m_aRowDevSel.InsertAt(iRow, static_cast<DWORD>(-1));
	m_grid.SetItemCountEx(m_aRowDevSel.GetSize(), 0);	// update grid control
	m_grid.SelectOnly(iRow);	// select newly inserted row
}

void CMidiMergeSplitterDlg::CDeviceType::DeleteSelectedRows()
{
	CIntArrayEx	aSelection;
	m_grid.GetSelection(aSelection);
	// don't assume selection is ordered, though it probably is
	aSelection.Sort();
	int	nSels = aSelection.GetSize();
	// reverse iterate for deletion stability
	for (int iSel = nSels - 1; iSel >= 0; iSel--) {	// for each selected row
		int	iRow = aSelection[iSel];	// selection element is row index
		DeleteRow(iRow);	// delete selected row
	}
	m_grid.SetItemCountEx(m_aRowDevSel.GetSize(), 0);	// update grid control
	DumpState();
}

void CMidiMergeSplitterDlg::CDeviceType::DeleteRow(int iRow)
{
	ASSERT(iRow >= 0 && iRow < m_grid.GetItemCount());
	int	iDevice = m_aRowDevSel[iRow];
	if (iDevice >= 0) {	// if row owns a device
		theApp.CloseDevice(m_iDevType, iDevice);	// close device
	}
	m_aRowDevSel.RemoveAt(iRow);	// delete row
	// caller is responsible for updating grid control
}

void CMidiMergeSplitterDlg::CDeviceType::DumpState()
{
#ifdef _DEBUG
	_tprintf(_T("%s ["), CMidiDevices::GetTypeCaption(m_iDevType));
	for (int iRow = 0; iRow < m_aRowDevSel.GetSize(); iRow++) {
		if (iRow)
			printf(", ");
		printf("%d", m_aRowDevSel[iRow]);
	}
	printf("]\n");
#endif
}

// CMidiMergeSplitterDlg operations

void CMidiMergeSplitterDlg::InitDeviceTypes()
{
	for (int iDevType = 0; iDevType < DEVICE_TYPES; iDevType++) {
		GetDeviceType(iDevType).m_iDevType = iDevType;
	}
}

void CMidiMergeSplitterDlg::InitGridControls()
{
	for (int iDevType = 0; iDevType < DEVICE_TYPES; iDevType++) {
		GetDeviceType(iDevType).InitGridControl();
	}
}

void CMidiMergeSplitterDlg::MakeDefaultRows(int nRows)
{
	for (int iDevType = 0; iDevType < DEVICE_TYPES; iDevType++) {
		GetDeviceType(iDevType).MakeDefaultRows(nRows);
	}
}

bool CMidiMergeSplitterDlg::ReadState()
{
	int	nTypesRead = 0;
	for (int iDevType = 0; iDevType < DEVICE_TYPES; iDevType++) {
		if (GetDeviceType(iDevType).ReadState()) {
			nTypesRead++;
		}
	}
	return nTypesRead > 0;
}

void CMidiMergeSplitterDlg::WriteState()
{
	for (int iDevType = 0; iDevType < DEVICE_TYPES; iDevType++) {
		GetDeviceType(iDevType).WriteState();
	}
}

void CMidiMergeSplitterDlg::DumpState()
{
#ifdef _DEBUG
	for (int iDevType = 0; iDevType < DEVICE_TYPES; iDevType++) {
		GetDeviceType(iDevType).DumpState();
	}
#endif
}

bool CMidiMergeSplitterDlg::OpenDevice(int iDevType, int iRow, int iDevice)
{
	CDeviceType& devType = GetDeviceType(iDevType);
	return devType.OpenDevice(iRow, iDevice);
}

void CMidiMergeSplitterDlg::InsertRow(int iDevType)
{
	CDeviceType& devType = GetDeviceType(iDevType);
	devType.InsertRow();
}

void CMidiMergeSplitterDlg::DeleteSelectedRows(int iDevType)
{
	CDeviceType& devType = GetDeviceType(iDevType);
	devType.DeleteSelectedRows();
}

CString CMidiMergeSplitterDlg::GetDeviceName(int iDevType, int iRow) const
{
	const CDeviceType& devType = GetDeviceType(iDevType);
	int	iDevice = devType.m_aRowDevSel[iRow];
	if (iDevice < 0) {	// if device is none
		return m_sNone;
	}
	return theApp.m_midiDevs.GetName(iDevType, iDevice);
}

int CMidiMergeSplitterDlg::GetFocusDeviceType() const
{
	HWND	hWnd = ::GetFocus();
	for (int iDevType = 0; iDevType < DEVICE_TYPES; iDevType++) {	// for each device type
		if (hWnd == GetDeviceType(iDevType).m_grid.m_hWnd) {	// if that device type's grid has focus
			return iDevType;	// return device type's index
		}
	}
	return -1;	// focus is elsewhere
}

void CMidiMergeSplitterDlg::PopulateCombo(int iDevType, int iRow, CComboBox& combo) const
{
	const CDeviceType& devType = GetDeviceType(iDevType);
	combo.AddString(m_sNone);	// add none item
	combo.SetItemData(0, static_cast<DWORD_PTR>(-1));	// none item has no device
	int	nDevices = theApp.m_midiDevs.GetCount(iDevType);
	int	iSelItem = 0;	// selection defaults to none
	for (int iDevice = 0; iDevice < nDevices; iDevice++) {	// for each device
		bool	bIsOurDev = iDevice == devType.m_aRowDevSel[iRow];
		// if device is ours, or device is closed
		if (bIsOurDev || !theApp.IsDeviceOpen(iDevType, iDevice)) {
			int	iItem = combo.AddString(theApp.m_midiDevs.GetName(iDevType, iDevice));
			combo.SetItemData(iItem, iDevice);
			if (bIsOurDev) {	// if device is ours
				iSelItem = iItem;	// set combo selection to this item
			}
		}
	}
	combo.SetCurSel(iSelItem);
}

// CModGridCtrl: derived grid control

CMidiMergeSplitterDlg::CModGridCtrl::CModGridCtrl()
{
	m_bDragEnable = false;
}

CWnd *CMidiMergeSplitterDlg::CModGridCtrl::CreateEditCtrl(LPCTSTR pszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	UNREFERENCED_PARAMETER(pszText);
	UNREFERENCED_PARAMETER(dwStyle);
	UNREFERENCED_PARAMETER(pParentWnd);
	UNREFERENCED_PARAMETER(nID);
	switch (m_iEditCol) {
	case COL_NUMBER:
		break;
	case COL_DEVICE:
		{
			CPopupCombo	*pCombo = CPopupCombo::Factory(0, rect, this, 0, 100);
			if (pCombo == NULL)
				return NULL;
			int	nGridID = GetDlgCtrlID();
			int iDevType = nGridID == IDC_INPUTS_LIST ? INPUT : OUTPUT;
			CMidiMergeSplitterDlg *pDlg = STATIC_DOWNCAST(CMidiMergeSplitterDlg, theApp.GetMainWnd());
			pDlg->PopulateCombo(iDevType, m_iEditRow, *pCombo);
			pCombo->ShowDropDown();
			return pCombo;
		}
		break;
	default:
		NODEFAULTCASE;
	}
	return NULL;
}

void CMidiMergeSplitterDlg::CModGridCtrl::OnItemChange(LPCTSTR pszText)
{
	UNREFERENCED_PARAMETER(pszText);
	switch (m_iEditCol) {
	case COL_NUMBER:
		break;
	case COL_DEVICE:
		{
			CPopupCombo	*pCombo = STATIC_DOWNCAST(CPopupCombo, m_pEditCtrl);
			int	iSelItem = pCombo->GetCurSel();	// index of changed item
			if (iSelItem < 0) {	// if no selection
				return;	// shouldn't happen
			}
			int	iDevice = static_cast<int>(pCombo->GetItemData(iSelItem));
			CMidiMergeSplitterDlg *pDlg = STATIC_DOWNCAST(CMidiMergeSplitterDlg, theApp.GetMainWnd());
			int	nGridID = GetDlgCtrlID();
			int iDevType = nGridID == IDC_INPUTS_LIST ? INPUT : OUTPUT;
			pDlg->OpenDevice(iDevType, m_iEditRow, iDevice);
		}
		break;
	default:
		NODEFAULTCASE;
	}
}

// CMidiMergeSplitterDlg overrides

BOOL CMidiMergeSplitterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_sNone.LoadString(IDS_NONE);

	InitGridControls();
	PostMessage(UWM_DELAYED_CREATE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMidiMergeSplitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INPUTS_LIST, m_aDevType[INPUT].m_grid);
	DDX_Control(pDX, IDC_OUTPUTS_LIST, m_aDevType[OUTPUT].m_grid);
}

BOOL CMidiMergeSplitterDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_INSERT:
		case VK_DELETE:
			{
				int iDevType = GetFocusDeviceType();
				if (iDevType >= 0) {
					if (pMsg->wParam == VK_INSERT) {
						InsertRow(iDevType);
					} else {
						DeleteSelectedRows(iDevType);
					}
				}
			}
			break;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CMidiMergeSplitterDlg::DestroyWindow()
{
	WriteState();
	return CDialogEx::DestroyWindow();
}

// CMidiMergeSplitterDlg message map

BEGIN_MESSAGE_MAP(CMidiMergeSplitterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(UWM_DELAYED_CREATE, OnDelayedCreate)
	ON_WM_DEVICECHANGE()
	ON_MESSAGE(UWM_DEVICE_NODE_CHANGE, OnDeviceNodeChange)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_INPUTS_LIST, OnGetdispinfoList)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_OUTPUTS_LIST, OnGetdispinfoList)
	ON_BN_CLICKED(IDC_INPUTS_ADD_BTN, OnClickedInputsAddBtn)
	ON_BN_CLICKED(IDC_INPUTS_REMOVE_BTN, OnClickedInputsRemoveBtn)
	ON_BN_CLICKED(IDC_OUTPUTS_ADD_BTN, OnClickedOutputsAddBtn)
	ON_BN_CLICKED(IDC_OUTPUTS_REMOVE_BTN, OnClickedOutputsRemoveBtn)
	ON_UPDATE_COMMAND_UI(IDC_INPUTS_REMOVE_BTN, OnUpdateInputsRemoveBtn)
	ON_UPDATE_COMMAND_UI(IDC_OUTPUTS_REMOVE_BTN, OnUpdateOutputsRemoveBtn)
END_MESSAGE_MAP()

// CMidiMergeSplitterDlg message handlers

void CMidiMergeSplitterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMidiMergeSplitterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMidiMergeSplitterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMidiMergeSplitterDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	UpdateDialogControls(this, FALSE); 
	return 0;
}

LRESULT	CMidiMergeSplitterDlg::OnDelayedCreate(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if (!ReadState()) {	// if no state was read
		MakeDefaultRows(2);	// assume first run
	}
	return 0;
}

BOOL CMidiMergeSplitterDlg::OnDeviceChange(UINT nEventType, W64ULONG dwData)
{
#if _DEBUG
//	_tprintf(_T("OnDeviceChange %x %x\n"), nEventType, dwData);
#endif
	BOOL	retc = CDialogEx::OnDeviceChange(nEventType, dwData);
	if (nEventType == DBT_DEVNODES_CHANGED) {
		// use post so device change completes before our handler runs
		PostMessage(UWM_DEVICE_NODE_CHANGE);
	}
	return retc;	// true to allow device change
}

LRESULT	CMidiMergeSplitterDlg::OnDeviceNodeChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	// should try to recover here
	return 0;
}

void CMidiMergeSplitterDlg::OnGetdispinfoList(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pResult);
	const NMLVDISPINFO*	pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	int iDevType = pDispInfo->hdr.idFrom == IDC_INPUTS_LIST ? INPUT : OUTPUT;
	const LVITEM&	item = pDispInfo->item;
	int	iItem = item.iItem;
	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
		case COL_NUMBER:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), iItem + 1); // make one-based
			break;
		case COL_DEVICE:
			_stprintf_s(item.pszText, item.cchTextMax, GetDeviceName(iDevType, iItem));
			break;
		}
	}
	*pResult = 0;
}

void CMidiMergeSplitterDlg::OnClickedInputsAddBtn()
{
	InsertRow(INPUT);
}

void CMidiMergeSplitterDlg::OnClickedInputsRemoveBtn()
{
	DeleteSelectedRows(INPUT);
}

void CMidiMergeSplitterDlg::OnClickedOutputsAddBtn()
{
	InsertRow(OUTPUT);
}

void CMidiMergeSplitterDlg::OnClickedOutputsRemoveBtn()
{
	DeleteSelectedRows(OUTPUT);
}

void CMidiMergeSplitterDlg::OnUpdateInputsRemoveBtn(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_aDevType[INPUT].m_grid.GetSelectedCount() > 0);
}

void CMidiMergeSplitterDlg::OnUpdateOutputsRemoveBtn(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_aDevType[OUTPUT].m_grid.GetSelectedCount() > 0);
}
