// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19jan04	initial version
		01		23nov07	support Unicode
        02      22feb25	replace header guard with pragma

        retrieve version information
 
*/

#pragma once

class CVersionInfo {
public:
	static	bool	GetFileInfo(VS_FIXEDFILEINFO& Info, LPCTSTR Path);
	static	bool	GetModuleInfo(VS_FIXEDFILEINFO& Info, LPCTSTR ModuleName);
};
