// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jul05	initial version
		01		02sep05	add QuoteSpaces
		02		23nov07	support Unicode
		03		03mar25	modernize style

		wrap useful shell path functions
 
*/

#include "stdafx.h"
#include "PathStr.h"

BOOL CPathStr::Append(LPCTSTR pszMore)
{
	LPTSTR	p = GetBuffer(MAX_PATH);
	BOOL	retc = PathAppend(p, pszMore);
	ReleaseBuffer();
	return retc;
}

void CPathStr::RemoveExtension()
{
	LPTSTR	p = GetBuffer(MAX_PATH);
	PathRemoveExtension(p);
	ReleaseBuffer();
}

BOOL CPathStr::RemoveFileSpec()
{
	LPTSTR	p = GetBuffer(MAX_PATH);
	BOOL	retc = PathRemoveFileSpec(p);
	ReleaseBuffer();
	return retc;
}

BOOL CPathStr::RenameExtension(LPCTSTR pszExt)
{
	LPTSTR	p = GetBuffer(MAX_PATH);
	BOOL	retc = PathRenameExtension(p, pszExt);
	ReleaseBuffer();
	return retc;
}

void CPathStr::QuoteSpaces()
{
	LPTSTR	p = GetBuffer(MAX_PATH);
	PathQuoteSpaces(p);
	ReleaseBuffer();
}
