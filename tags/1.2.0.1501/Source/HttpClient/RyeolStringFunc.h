/*!
 * \file	RyeolStringFunc.h
 * \brief	Functions related to string handling.
 * \author	Jo Hyeong-ryeol
 * \since	2003.11.12
 * \version	$LastChangedRevision: 99 $
 *			$LastChangedDate: 2006-02-03 23:16:57 +0900 (금, 03 2 2006) $
 *
 * This file contains functions related to string handling.
 * \n\n\n
 * Copyright &copy; 2006 by <a href="mailto:hyeongryeol@gmail.com">Jo Hyeong-ryeol</a>\n
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 */
#pragma once

#include <windows.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>			// for String... functions
#include <crtdbg.h>				// for _ASSERTE 

/*!
 * \brief	The namespace of the Ryeol's library
 *
 * This is the namespace for source codes written by Jo Hyeong-ryeol.
 */
namespace Ryeol {

	/*! \brief	Returns the number of bytes which is required to convert an UNICODE string into an ANSI string */
	HRESULT NeededCharsForUnicode2Ansi (LPCWSTR wszSrc, int * pcbNeeded) ;
	/*! \brief	Converts an UNICODE string into an ANSI string */
	HRESULT Unicode2Ansi (LPSTR szDest, int cbDest, LPCWSTR wszSrc) ;
	/*! \brief	Converts an UNICODE string into an ANSI string */
	HRESULT Unicode2Ansi (LPSTR * pszDest, LPCWSTR wszSrc, HANDLE hHeap) ;

	/*! \brief	Returns the number of bytes which is required to convert an UNICODE string into an UTF-8 string */
	HRESULT NeededCharsForUnicode2UTF8 (LPCWSTR wszSrc, int * pcbNeeded) ;
	/*! \brief	Converts an UNICODE string into an UTF-8 string */
	HRESULT Unicode2UTF8 (LPSTR szDest, int cbDest, LPCWSTR wszSrc) ;
	/*! \brief	Converts an UNICODE string into an UTF-8 string */
    HRESULT Unicode2UTF8 (LPSTR * pszDest, LPCWSTR wszSrc, HANDLE hHeap) ;

	/*! \brief	Returns the number of characters which is required to convert an ANSI string into an UNICODE string */
	HRESULT NeededCharsForAnsi2Unicode (LPCSTR szSrc, int * pcchNeeded) ;
	/*! \brief	Converts an ANSI string into an UNICODE string */
	HRESULT Ansi2Unicode (LPWSTR wszDest, int cchDest, LPCSTR szSrc) ;
	/*! \brief	Converts an ANSI string into an UNICODE string */
	HRESULT Ansi2Unicode (LPWSTR * pwszDest, LPCSTR szSrc, HANDLE hHeap) ;

}

