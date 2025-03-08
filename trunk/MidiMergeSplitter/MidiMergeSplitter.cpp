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

// MidiMergeSplitter.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MidiMergeSplitter.h"
#include "MidiMergeSplitterDlg.h"

#include "Win32Console.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMidiMergeSplitterApp

BEGIN_MESSAGE_MAP(CMidiMergeSplitterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CMidiMergeSplitterApp construction

CMidiMergeSplitterApp::CMidiMergeSplitterApp()
{
}

// The one and only CMidiMergeSplitterApp object

CMidiMergeSplitterApp theApp;

// CMidiMergeSplitterApp initialization

BOOL CMidiMergeSplitterApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

#ifdef _DEBUG
	Win32Console::Create();
#endif

	CWinApp::InitInstance();

	AfxEnableControlContainer();

#if 0
	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;
#endif

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Anal Software"));

	m_midiDevs.Update();	// update MIDI devices

#if _DEBUG
	m_midiDevs.Dump();	// dump MIDI devices
#endif

	CMidiMergeSplitterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

#if 0
	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CMidiMergeSplitterApp::ExitInstance()
{
	return CWinAppCK::ExitInstance();
}

void CMidiMergeSplitterApp::DumpState()
{
#ifdef _DEBUG
	int	nInSlots = m_aMidiIn.GetSize();
	ASSERT(m_aDevIdx[INPUT].GetSize() == nInSlots);
	printf("%d inputs\n", nInSlots);
	for (int iInSlot = 0; iInSlot < nInSlots; iInSlot++) {
		int	iDevice = m_aDevIdx[INPUT][iInSlot];
		_tprintf(_T("\t%d: %d '%s'\n"), iInSlot, iDevice, m_midiDevs.GetName(INPUT, iDevice));
	}
	int	nOutSlots = m_aMidiOut.GetSize();
	ASSERT(m_aDevIdx[OUTPUT].GetSize() == nOutSlots);
	printf("%d outputs\n", nOutSlots);
	for (int iOutSlot = 0; iOutSlot < nOutSlots; iOutSlot++) {
		int	iDevice = m_aDevIdx[OUTPUT][iOutSlot];
		_tprintf(_T("\t%d: %d '%s'\n"), iOutSlot, iDevice, m_midiDevs.GetName(OUTPUT, iDevice));
	}
#endif
}

bool CMidiMergeSplitterApp::OpenDevice(int iDevType, int iDevice)
{
	if (iDevType == INPUT) {	// if input device
		int	iDevSlot = m_aMidiIn.GetSize();
		m_aMidiIn.SetSize(iDevSlot + 1);	// allocate slot for device
		MMRESULT	mr = m_aMidiIn[iDevSlot].Open(iDevice, reinterpret_cast<W64UINT>(MidiInProc), NULL, CALLBACK_FUNCTION);
		if (MIDI_FAILED(mr)) {	// if open failed
			m_aMidiIn.SetSize(iDevSlot);	// remove failed slot
			OnOpenError(iDevType, iDevice);	// handle error
			return false;
		}
		m_aDevIdx[INPUT].Add(iDevice);	// update device index array
		m_aMidiIn[iDevSlot].Start();	// start MIDI input flowing
	} else {	// output device
		int	iDevSlot = m_aMidiOut.GetSize();
		m_aMidiOut.SetSize(iDevSlot + 1);	// allocate slot for device
		MMRESULT	mr = m_aMidiOut[iDevSlot].Open(iDevice, NULL, NULL, CALLBACK_NULL);
		if (MIDI_FAILED(mr)) {	// if open failed
			m_aMidiOut.SetSize(iDevSlot);	// remove failed slot
			OnOpenError(iDevType, iDevice);	// handle error
			return false;
		}
		m_aDevIdx[OUTPUT].Add(iDevice);	// update device index array
	}
	DumpState();
	return true;
}

bool CMidiMergeSplitterApp::CloseDevice(int iDevType, int iDevice)
{
	if (iDevType == INPUT) {	// if input device
		INT_PTR	iDevSlot = m_aDevIdx[INPUT].Find(iDevice);
		if (iDevSlot < 0) {	// if device not found
			return false;
		}
		MMRESULT	mr = m_aMidiIn[iDevSlot].Close();
		if (MIDI_FAILED(mr)) {	// if close failed
			ASSERT(0);	// shouldn't happen, but continue if it does
		}
		m_aMidiIn.FastRemoveAt(iDevSlot);
		m_aDevIdx[INPUT].RemoveAt(iDevSlot);
	} else {	// output device
		INT_PTR	iDevSlot = m_aDevIdx[OUTPUT].Find(iDevice);
		if (iDevSlot < 0) {	// if device not found
			return false;
		}
		MMRESULT	mr = m_aMidiOut[iDevSlot].Close();
		if (MIDI_FAILED(mr)) {	// if close failed
			ASSERT(0);	// shouldn't happen, but continue if it does
		}
		m_aMidiOut.FastRemoveAt(iDevSlot);
		m_aDevIdx[OUTPUT].RemoveAt(iDevSlot);
	}
	DumpState();
	return true;
}

void CMidiMergeSplitterApp::OnOpenError(int iDevType, int iDevice)
{
	static const int nErrorMsgID[DEVICE_TYPES] = {
		IDS_ERR_OPEN_INPUT_DEVICE,
		IDS_ERR_OPEN_OUTPUT_DEVICE,
	};
	CString	sErrorMsg;
	sErrorMsg.LoadString(nErrorMsgID[iDevType]);
	CString	sDevName(m_midiDevs.GetName(iDevType, iDevice));
	if (!sDevName.IsEmpty()) {
		sErrorMsg += '\n' + sDevName;
	}
	AfxMessageBox(sErrorMsg);
}

void CALLBACK CMidiMergeSplitterApp::MidiInProc(HMIDIIN hMidiIn, UINT wMsg, W64UINT dwInstance, W64UINT dwParam1, W64UINT dwParam2)
{
	// This callback function runs in a worker thread context; 
	// data shared with other threads may require serialization.
	UNREFERENCED_PARAMETER(hMidiIn);
	UNREFERENCED_PARAMETER(dwInstance);
	UNREFERENCED_PARAMETER(dwParam2);
#if _DEBUG
//	_tprintf(_T("MidiInProc %d %d\n"), GetCurrentThreadId(), ::GetThreadPriority(GetCurrentThread()));
#endif
	switch (wMsg) {
	case MIM_DATA:
		{
#if _DEBUG
//			_tprintf(_T("%x %d\n"), dwParam1, dwParam2);
#endif
			int	nOutDevs = theApp.m_aMidiOut.GetSize();
			for (int iOutDev = 0; iOutDev < nOutDevs; iOutDev++) {	// for each output device
				DWORD	nMidiMsg = static_cast<DWORD>(dwParam1);
				theApp.m_aMidiOut[iOutDev].OutShortMsg(nMidiMsg);	// send MIDI message to device
			}
		}
		break;
	}
}
