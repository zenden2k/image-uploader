/*!
 * \file	RyeolException.cpp
 * \brief	Implementations of Ryeol's exception classes.
 * \author	Jo Hyeong-ryeol
 * \since	2004.04.12
 * \version	$LastChangedRevision: 89 $
 *			$LastChangedDate: 2006-01-30 11:20:19 +0900 (월, 30 1 2006) $
 *
 * This file contains implementations of exception classes.
 * \n\n\n
 * Copyright &copy; 2006 by <a href="mailto:hyeongryeol@gmail.com">Jo Hyeong-ryeol</a>\n
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 */
#include "stdafx.h"
#include "RyeolException.h"

#pragma warning (disable: 4290)	// avoids 'C++ Exception Specification ignored' message
#pragma warning (disable: 4996)	// avoids 'This function or variable may be unsafe' message


/*!
 * \brief	The namespace of the Ryeol's library
 *
 * This is the namespace for source codes written by Jo Hyeong-ryeol.
 */
namespace Ryeol {

/*!
 * This is a default constructor with no argument.
 */
errmsg_exceptionA::errmsg_exceptionA (void)
	throw ()
{
	m_szErrMsg = NULL ;
}

/*!
 * This is a constructor with an initial error message.
 * If memory allocation failed, the error message will not be copied
 * and Internal error message will point to NULL.
 *
 * \param szErrMsg		[in] An initial error message.
 */
errmsg_exceptionA::errmsg_exceptionA (LPCSTR szErrMsg)
	throw ()
{
	m_szErrMsg = NULL ;
	if ( szErrMsg == NULL )
		return ;

	seterrmsg (szErrMsg) ;
}

/*!
 * This is a copy constructor.
 * If memory allocation failed, the error message will not be copied
 * and Internal error message will point to NULL.
 *
 * \param ee			[in] An errmsg_exceptionA object.
 */
errmsg_exceptionA::errmsg_exceptionA (const errmsg_exceptionA & ee)
	throw ()
{
	m_szErrMsg = NULL ;
	operator=(ee) ;
}

/*!
 * This is a default destructor.
 */
errmsg_exceptionA::~errmsg_exceptionA (void)
	throw ()
{
	::free (m_szErrMsg) ;
}

/*!
 * This method returns an error message. It guarantees that it does
 * not return NULL (Returns "NULL" instead of NULL pointer). 
 * This method equals to the errmsg method in Ansi version.
 *
 * \return				A message describing the error.
 * \sa					errmsg
 */
LPCSTR errmsg_exceptionA::what (void) const
	throw ()
{
	return errmsg () ;
}

/*!
 * This method returns an error message. It guarantees that it does
 * not return NULL (Returns "NULL" instead of NULL pointer).
 *
 * \return				A message describing the error.
 */
LPCSTR errmsg_exceptionA::errmsg (void) const
	throw ()
{
	if ( m_szErrMsg == NULL )
		return "NULL" ;

	return m_szErrMsg ;
}

/*!
 * This method assigns a new error message. If memory allocation failed,
 * NULL is assigned.
 *
 * \param szErrMsg		A new error message.
 */
void errmsg_exceptionA::seterrmsg (LPCSTR szErrMsg)
	throw ()
{
	::free (m_szErrMsg) ;
	m_szErrMsg = NULL ;

	if ( szErrMsg == NULL )
		return ;

	size_t		cchErrMsg = ::strlen (szErrMsg) ;
	m_szErrMsg = (LPSTR) ::malloc (sizeof (CHAR) * (cchErrMsg + 1)) ;
	if ( m_szErrMsg == NULL )
		return ;

	::strcpy (m_szErrMsg, szErrMsg) ;
}

/*!
 * This operator assigns an error message using seterrmsg method.
 * If memory allocation failed, the error message will not be copied
 * and Internal error message will point to NULL.
 *
 * \param ee			[in] An errmsg_exceptionA object.
 * \return				The exception object itself.
 */
errmsg_exceptionA & errmsg_exceptionA::operator=(const errmsg_exceptionA & ee)
	throw ()
{
	if ( this == &ee )
		return *this ;

	seterrmsg (ee.errmsg ()) ;
	return *this ;
}


/*!
 * This is a default constructor with no argument.
 */
errmsg_exceptionW::errmsg_exceptionW (void)
	throw ()
{
	m_szErrMsg = NULL ;
}

/*!
 * This is a constructor with an initial error message.
 * If memory allocation failed, the error message will not be copied
 * and Internal error message will point to NULL.
 *
 * \param szErrMsg		[in] An initial error message.
 */
errmsg_exceptionW::errmsg_exceptionW (LPCWSTR szErrMsg)
	throw ()
{
	m_szErrMsg = NULL ;
	if ( szErrMsg == NULL )
		return ;

	seterrmsg (szErrMsg) ;
}

/*!
 * This is a copy constructor.
 * If memory allocation failed, the error message will not be copied
 * and Internal error message will point to NULL.
 *
 * \param ee			[in] An errmsg_exceptionA object.
 */
errmsg_exceptionW::errmsg_exceptionW (const errmsg_exceptionW & ee)
	throw ()
{
	operator=(ee) ;
}

/*!
 * This is a default destructor.
 */
errmsg_exceptionW::~errmsg_exceptionW (void)
	throw ()
{
	::free (m_szErrMsg) ;
}

/*!
 * This is not supported in Unicode version.
 * It always returns "errmsg_exceptionW".
 *
 * \return				"errmsg_exceptionW"
 */
LPCSTR errmsg_exceptionW::what (void) const
	throw ()
{
	return "errmsg_exceptionW" ;
}

/*!
 * This method returns an error message. It guarantees that it does
 * not return NULL (Returns "NULL" instead of NULL pointer).
 *
 * \return				A message describing the error.
 */
LPCWSTR errmsg_exceptionW::errmsg (void) const
	throw ()
{
	if ( m_szErrMsg == NULL )
		return L"NULL" ;

	return m_szErrMsg ;
}

/*!
 * This method assigns a new error message. If memory allocation failed,
 * NULL is assigned.
 *
 * \param szErrMsg		A new error message.
 */
void errmsg_exceptionW::seterrmsg (LPCWSTR szErrMsg)
	throw ()
{
	::free (m_szErrMsg) ;
	m_szErrMsg = NULL ;

	if ( szErrMsg == NULL )
		return ;

	size_t		cchErrMsg = ::wcslen (szErrMsg) ;
	m_szErrMsg = (LPWSTR) ::malloc (sizeof (WCHAR) * (cchErrMsg + 1)) ;
	if ( m_szErrMsg == NULL )
		return ;

	::wcscpy (m_szErrMsg, szErrMsg) ;
}

/*!
 * This operator assigns an error message using seterrmsg method.
 * If memory allocation failed, the error message will not be copied
 * and Internal error message will point to NULL.
 *
 * \param ee			[in] An errmsg_exceptionA object.
 * \return				The exception object itself.
 */
errmsg_exceptionW & errmsg_exceptionW::operator=(const errmsg_exceptionW & ee)
	throw ()
{
	if ( this == &ee )
		return *this ;

	seterrmsg (ee.m_szErrMsg) ;
	return *this ;
}


/*!
 * This is a default constructor with no argument.
 */
varerrmsg_exceptionA::varerrmsg_exceptionA (void)
	throw ()
: errmsg_exceptionA ()
{
	;
}

/*!
 * This is a constructor with a format string and optional arguments.
 * The stored error message can not exceed EXCEPTION_BUFF_SIZE - 1 characters.
 * If an error occured, the stored error messsage will be NULL.
 *
 * \param szErrMsg		[in] A format-control error message 
 */
varerrmsg_exceptionA::varerrmsg_exceptionA (LPCSTR szErrMsg, ...)
	throw ()
{
	va_list		args ;

	va_start (args, szErrMsg) ;
	seterrmsg (szErrMsg, args) ;
	va_end (args) ;
}

/*!
 * This is a constructor with a format string and a pointer to list of arguments.
 * The stored error message can not exceed EXCEPTION_BUFF_SIZE - 1 characters.
 * If an error occured, the stored error messsage will be NULL.
 *
 * \param szErrMsg		[in] A format-control error message 
 * \param args			[in] A pointer to list of arguments 
 */
varerrmsg_exceptionA::varerrmsg_exceptionA (LPCSTR szErrMsg, va_list args)
	throw ()
{
	seterrmsg (szErrMsg, args) ;
}

/*!
 * This method assigns a new error message using _vsnprintf.
 * The stored error message can not exceed EXCEPTION_BUFF_SIZE - 1 characters.
 * If an error occured, the stored error messsage will be NULL.
 *
 * \param szErrMsg		[in] A format-control error message 
 */
void varerrmsg_exceptionA::seterrmsg (LPCSTR szErrMsg, ...)
	throw ()
{
	va_list		args ;

	va_start (args, szErrMsg) ;
	seterrmsg (szErrMsg, args) ;
	va_end (args) ;
}

/*!
 * This method assigns a new error message using _vsnprintf.
 * The stored error message can not exceed EXCEPTION_BUFF_SIZE - 1 characters.
 * If an error occured, the stored error messsage will be NULL.
 *
 * \param szErrMsg		[in] A format-control error message 
 * \param args			[in] A pointer to list of arguments 
 */
void varerrmsg_exceptionA::seterrmsg (LPCSTR szErrMsg, va_list args)
	throw ()
{
	if ( szErrMsg == NULL ) {
		errmsg_exceptionA::seterrmsg (szErrMsg) ;
		return ;
	}

	CHAR			szBuff[EXCEPTION_BUFF_SIZE] ;
	int				cchWritten ;
	cchWritten = _vsnprintf (szBuff, EXCEPTION_BUFF_SIZE - 1, szErrMsg, args) ;

	// If the error message exceeds the buffer size...
	if ( cchWritten == -1 )
		szBuff[EXCEPTION_BUFF_SIZE - 1] = '\0' ;
	// If an error occured..
	else if ( cchWritten < 0 ) {
		errmsg_exceptionA::seterrmsg (NULL) ;
		return ;
	} else
		szBuff[cchWritten] = '\0' ;

	errmsg_exceptionA::seterrmsg (szBuff) ;
}


/*!
 * This is a default constructor with no argument.
 */
varerrmsg_exceptionW::varerrmsg_exceptionW (void)
	throw ()
: errmsg_exceptionW ()
{
	;
}

/*!
 * This is a constructor with a format string and optional arguments.
 * The stored error message can not exceed EXCEPTION_BUFF_SIZE - 1 characters.
 * If an error occured, the stored error messsage will be NULL.
 *
 * \param szErrMsg		[in] A format-control error message 
 */
varerrmsg_exceptionW::varerrmsg_exceptionW (LPCWSTR szErrMsg, ...)
	throw ()
{
	va_list		args ;

	va_start (args, szErrMsg) ;
	seterrmsg (szErrMsg, args) ;
	va_end (args) ;
}

/*!
 * This is a constructor with a format string and a pointer to list of arguments.
 * The stored error message can not exceed EXCEPTION_BUFF_SIZE - 1 characters.
 * If an error occured, the stored error messsage will be NULL.
 *
 * \param szErrMsg		[in] A format-control error message 
 * \param args			[in] A pointer to list of arguments 
 */
varerrmsg_exceptionW::varerrmsg_exceptionW (LPCWSTR szErrMsg, va_list args)
	throw ()
{
	seterrmsg (szErrMsg, args) ;
}

/*!
 * This is not supported in Unicode version.
 * It always returns "varerrmsg_exceptionW".
 *
 * \return				"varerrmsg_exceptionW"
 */
LPCSTR varerrmsg_exceptionW::what () const
	throw ()
{
	return "varerrmsg_exceptionW" ;
}

/*!
 * This method assigns a new error message using _vsnwprintf.
 * The stored error message can not exceed EXCEPTION_BUFF_SIZE - 1 characters.
 * If an error occured, the stored error messsage will be NULL.
 *
 * \param szErrMsg		[in] A format-control error message 
 */
void varerrmsg_exceptionW::seterrmsg (LPCWSTR szErrMsg, ...)
	throw ()
{
	va_list		args ;

	va_start (args, szErrMsg) ;
	seterrmsg (szErrMsg, args) ;
	va_end (args) ;
}

/*!
 * This method assigns a new error message using _vsnwprintf.
 * The stored error message can not exceed EXCEPTION_BUFF_SIZE - 1 characters.
 * If an error occured, the stored error messsage will be NULL.
 *
 * \param szErrMsg		[in] A format-control error message 
 * \param args			[in] A pointer to list of arguments 
 */
void varerrmsg_exceptionW::seterrmsg (LPCWSTR szErrMsg, va_list args)
	throw ()
{
	if ( szErrMsg == NULL ) {
		errmsg_exceptionW::seterrmsg (szErrMsg) ;
		return ;
	}

	WCHAR			szBuff[EXCEPTION_BUFF_SIZE] ;
	int				cchWritten ;
	cchWritten = _vsnwprintf (szBuff, EXCEPTION_BUFF_SIZE - 1, szErrMsg, args) ;

	// If the error message exceeds the buffer size...
	if ( cchWritten == -1 )
		szBuff[EXCEPTION_BUFF_SIZE - 1] = '\0' ;
	// If an error occured..
	else if ( cchWritten < 0 ) {
		errmsg_exceptionW::seterrmsg (NULL) ;
		return ;
	} else
		szBuff[cchWritten] = '\0' ;

	errmsg_exceptionW::seterrmsg (szBuff) ;
}

}
