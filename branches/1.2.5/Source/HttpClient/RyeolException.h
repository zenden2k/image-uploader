/*!
 * \file	RyeolException.h
 * \brief	exception classes which extends the std::exception class.
 * \author	Jo Hyeong-ryeol
 * \since	2004.04.12
 * \version	$LastChangedRevision: 89 $
 *			$LastChangedDate: 2006-01-30 11:20:19 +0900 (월, 30 1 2006) $
 *
 * This file contains exception classes which extends the standard exception class.
 * \n\n\n
 * Copyright &copy; 2006 by <a href="mailto:hyeongryeol@gmail.com">Jo Hyeong-ryeol</a>\n
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 */
#pragma once

#include <windows.h>			// for generic types, .. etc
#include <stdarg.h>				// for va_arg, va_start, va_end
#include <stdexcept>			// for std::exception class

#pragma warning (push)
#pragma warning (disable: 4290)	// avoids 'C++ Exception Specification ignored' message

/*!
 * \brief	The namespace of the Ryeol's library
 *
 * This is the namespace for source codes written by Jo Hyeong-ryeol.
 */
namespace Ryeol {

enum {
	EXCEPTION_BUFF_SIZE = 512		//!< Maximum buffer size for an error message stored 
									//! in the varerrmsg_exception class.
} ;

/*!
 * \brief	A simple exception class with a error message which is allocated in heap by malloc. (Ansi Ver.)
 *
 * This is a simple exception class derived from the standard exception class.
 * This class can set or get an error message. The message is internally allocated in heap by malloc.
 * So it is possible to lose the error message if memory allocation failed.
 */
class errmsg_exceptionA : public std::exception {
public:
	/*! \brief	Default constructor */
	errmsg_exceptionA (void) throw () ;
	/*! \brief	Constructor with an initial error message */
	errmsg_exceptionA (LPCSTR szErrMsg) throw () ;
	/*! \brief	Copy constructor */
	errmsg_exceptionA (const errmsg_exceptionA & ee) throw () ;
	/*! \brief	Default destructor */
	virtual ~errmsg_exceptionA (void) throw () ;

	/*! \brief	Returns an error message. */
	LPCSTR what (void) const throw () ;
	/*! \brief	Returns an error message. */
	virtual LPCSTR errmsg (void) const throw () ;
	/*! \brief	Assigns a new error message. */
	virtual void seterrmsg (LPCSTR szErrMsg) throw () ;
	/*! \brief	Assignment operator. */
	errmsg_exceptionA & operator=(const errmsg_exceptionA & ee) throw () ;

protected:
	LPSTR			m_szErrMsg ;	//!< An internal error message pointer.
} ;


/*!
 * \brief	A simple exception class with a error message which is allocated in heap by malloc. (Unicode Ver.)
 *
 * This is a simple exception class derived from the standard exception class.
 * This class can set or get an error message. The message is internally allocated in heap by malloc.
 * So it is possible to lose the error message if memory allocation failed.
 */
class errmsg_exceptionW : public std::exception {
public:
	/*! \brief	Default constructor */
	errmsg_exceptionW (void) throw () ;
	/*! \brief	Constructor with an initial error message */
	errmsg_exceptionW (LPCWSTR szErrMsg) throw () ;
	/*! \brief	Copy constructor */
	errmsg_exceptionW (const errmsg_exceptionW & ee) throw () ;
	/*! \brief	Default destructor */
	virtual ~errmsg_exceptionW (void) throw () ;

	/*! \brief	Returns "errmsg_exceptionW" */
	LPCSTR what (void) const throw () ;
	/*! \brief	Returns an error message. */
	virtual LPCWSTR errmsg (void) const throw () ;
	/*! \brief	Assigns a new error message. */
	virtual void seterrmsg (LPCWSTR szErrMsg) throw () ;
	/*! \brief	Assignment operator. */
	errmsg_exceptionW & operator=(const errmsg_exceptionW & ee) throw () ;

protected:
	LPWSTR			m_szErrMsg ;	//!< An internal error message pointer.
} ;

#ifdef UNICODE
	typedef errmsg_exceptionW		errmsg_exception ;
#else
	typedef errmsg_exceptionA		errmsg_exception ;
#endif


/*!
 * \brief	A simple exception class which can take a variable number of arguments. (Ansi Ver.)
 *
 * This class takes a format string and optional arguments and then stores a formatted 
 * error message using _vsnprintf. The stored error message can not exceed 
 * EXCEPTION_BUFF_SIZE - 1 characters. If an error occured, the stored error messsage will be NULL.
 */
class varerrmsg_exceptionA : public errmsg_exceptionA {
public:
	/*! \brief	Default constructor */
	varerrmsg_exceptionA (void) throw () ;
	/*! \brief	Constructor with a format string and optional arguments */
	varerrmsg_exceptionA (LPCSTR szErrMsg, ...) throw () ;
	/*! \brief	Constructor with a format string and a pointer to list of arguments */
	varerrmsg_exceptionA (LPCSTR szErrMsg, va_list arg) throw () ;

	/*! \brief	Assigns a new error message using _vsnprintf */
	virtual void seterrmsg (LPCSTR szErrMsg, ...) throw () ;
	/*! \brief	Assigns a new error message using _vsnprintf */
	virtual void seterrmsg (LPCSTR szErrMsg, va_list args) throw () ;
} ;


/*!
 * \brief	A simple exception class which can take a variable number of arguments. (Unicode Ver.)
 *
 * This class takes a format string and optional arguments and then stores a formatted 
 * error message using _vsnwprintf. The stored error message can not exceed 
 * EXCEPTION_BUFF_SIZE - 1 characters. If an error occured, the stored error messsage will be NULL.
 */
class varerrmsg_exceptionW : public errmsg_exceptionW {
public:
	/*! \brief	Default constructor */
	varerrmsg_exceptionW (void) throw () ;
	/*! \brief	Constructor with a format string and optional arguments */
	varerrmsg_exceptionW (LPCWSTR szErrMsg, ...) throw () ;
	/*! \brief	Constructor with a format string and a pointer to list of arguments */
	varerrmsg_exceptionW (LPCWSTR szErrMsg, va_list args) throw () ;

	/*! \brief	Returns "varerrmsg_exceptionW" */
	virtual LPCSTR what (void) const throw () ;
	/*! \brief	Assigns a new error message using _vsnwprintf */
	virtual void seterrmsg (LPCWSTR szErrMsg, ...) throw () ;
	/*! \brief	Assigns a new error message using _vsnwprintf */
	virtual void seterrmsg (LPCWSTR szErrMsg, va_list args) throw () ;
} ;

#ifdef UNICODE
	typedef varerrmsg_exceptionW		varerrmsg_exception ;
#else
	typedef varerrmsg_exceptionA		varerrmsg_exception ;
#endif
}

#pragma warning (pop)	// restores the default
