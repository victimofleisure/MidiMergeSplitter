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

// MidiMergeSplitter.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "WinAppCK.h"
#include "MidiDevices.h"
#include "MidiWrap.h"

// CMidiMergeSplitterApp:
// See MidiMergeSplitter.cpp for the implementation of this class
//

class CMidiMergeSplitterApp : public CWinAppCK
{
public:
	CMidiMergeSplitterApp();

// Operations
	bool	OpenDevice(int iDevType, int iDevice);
	bool	CloseDevice(int iDevType, int iDevice);
	bool	IsDeviceOpen(int iDevType, int iDevice);

// Overrides
public:
	virtual BOOL InitInstance();

// Types
	typedef CArrayEx<CMidiIn, CMidiIn&> CMidiInArray;
	typedef CArrayEx<CMidiOut, CMidiOut&> CMidiOutArray;

// Constants
	enum {	// device types
		INPUT = CMidiDevices::INPUT,
		OUTPUT = CMidiDevices::OUTPUT,
		DEVICE_TYPES = CMidiDevices::DEVICE_TYPES,
	};

// Data members
	CMidiDevices	m_midiDevs;		// MIDI device data
	CMidiInArray	m_aMidiIn;		// array of MIDI input drivers
	CMidiOutArray	m_aMidiOut;		// array of MIDI output drivers
	CIntArrayEx		m_aDevIdx[DEVICE_TYPES];	// array of device index arrays

// Helpers
	void	DumpState();
	static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, W64UINT dwInstance, W64UINT dwParam1, W64UINT dwParam2);
	void	OnOpenError(int iDevType, int iDevice);

// Implementation
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CMidiMergeSplitterApp theApp;

inline bool CMidiMergeSplitterApp::IsDeviceOpen(int iDevType, int iDevice)
{
	return m_aDevIdx[iDevType].Find(iDevice) >= 0;
}
