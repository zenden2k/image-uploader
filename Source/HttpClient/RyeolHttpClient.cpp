/*!
 * \file	RyeolHttpClient.cpp
 * \brief	Implementations of Ryeol's HTTP client classes.
 * \author	Jo Hyeong-ryeol
 * \since	2004.04.12
 * \version	$LastChangedRevision: 103 $LYOUL
 *			$LastChangedDate: 2006-02-05 00:38:41 +0900 (일, 05 2 2006) $
 * 
 * <dl compact>
 * <dt><b>Requirements:</b></dt>
 * <dd>Requires Internet Explorer 4.0 or later.</dd><br>
 * <dd>Unicode version class support on Windows Me/98/95 requires Microsoft Layer for Unicode.</dd><br>
 * <dd>UTF-8 encoding support on Windows 95 requires Microsoft Layer for Unicode.</dd>
 * </dl>
 * This file contains implementations of Ryeol's HTTP client classes.
 * \n\n\n
 * Copyright &copy; 2006 by <a href="mailto:hyeongryeol@gmail.com">Jo Hyeong-ryeol</a>\n
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 */
#include "stdafx.h"
#include "RyeolHttpClient.h"

#pragma warning (disable: 4290)	// avoids 'C++ Exception Specification ignored' message
#pragma warning (disable: 4660)
#pragma warning (disable: 4996)	// avoids 'This function or variable may be unsafe' message
#pragma comment (lib, "wininet.lib")
#pragma comment (lib, "urlmon.lib")

/*!
 * \brief	The namespace of the Ryeol's library
 *
 * This is the namespace for source codes written by Jo Hyeong-ryeol.
 */
namespace Ryeol {

/////////////////////////////// Global constant message table ////////////////////////////////////
#ifndef SAFEFREE
#	define SAFEFREE(x) if(x){ ::free((void *)x); x = NULL; }
#endif

#ifndef INVALID_SET_FILE_POINTER
#	define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

#define	HTTPCLIENT_POSTCNTX_BUFF_SIZE			(1024 * 56)


// An assertion macro for the CHttpToolA and the CHttpToolW class
#define HTTPTOOL_ASSERT(expr, msg)										\
																		\
	if ( !(expr) ) {													\
		_ASSERTE ( expr ) ; ThrowException (msg) ;						\
	}

// An assertion macro for the CHttpClient related classes
#define HTTPCLIENT_ASSERT(expr, msg)									\
																		\
	if ( !(expr) ) {													\
		 _ASSERTE ( expr ) ;											\
		if ( HttpTool::IsAnsi () )										\
			CHttpToolA::ThrowException (msg) ;							\
		else															\
			CHttpToolW::ThrowException (L##msg) ;						\
   }

// An assertion macro for the CHttpClient related classes
#define HTTPCLIENT_ASSERTA(expr, msg)									\
																		\
	if ( !(expr) ) {													\
		 _ASSERTE ( expr ) ;											\
		CHttpToolA::ThrowException (msg) ;								\
   }

// An assertion macro for the CHttpClient related classes
#define HTTPCLIENT_ASSERTW(expr, msg)									\
																		\
	if ( !(expr) ) {													\
		 _ASSERTE ( expr ) ;											\
		CHttpToolW::ThrowException (L##msg) ;							\
   }


// not specified
static LPCSTR		g_NotSpecifiedA[] = {
	"Not specified"
} ;

static LPCWSTR		g_NotSpecifiedW[] = {
	L"Not specified"
} ;

// error messages
static LPCSTR		g_NormalMsgA[] = {
	"Unexpected error occurred."
	, "The index is out of range."
	, "Out of memory."
	, "The requested URL is not a valid URL."
	, "The post context is not started yet."
	, "Couldn't read expected bytes from a file."
	, "The post context has not been finished yet."
	, "The port number is not valid."
	, "std::exception occurred."
	, "The encoded URL is not valid."
	, "The UTF8 string contains an invalid character."
	, "An unexpected arithmetic error has been occurred."
	, "An arithmetic overflow error has been occurred."
	, "An interger divide by zero exception has been occurred."
	, "The file (%s) aleady exists."
} ;

static LPCWSTR		g_NormalMsgW[] = {
	L"Unexpected error occurred."
	, L"The index is out of range."
	, L"Out of memory."
	, L"The requested URL is not a valid URL."
	, L"The post context is not started yet."
	, L"Couldn't read expected bytes from a file."
	, L"The post context has not been finished yet."
	, L"The port number is not valid."
	, L"std::exception occurred."
	, L"The encoded URL is not valid."
	, L"The UTF8 string contains an invalid character."
	, L"An unexpected arithmetic error has been occurred."
	, L"An arithmetic overflow error has been occurred."
	, L"An interger divide by zero exception has been occurred."
	, L"The file (%s) aleady exists."
} ;

// error messages (which has a win32 error code) - Reserved
static LPCSTR		g_NormalMsgWin32A[] = {""} ;
static LPCWSTR		g_NormalMsgWin32W[] = {L""} ;

// WinInet error messages (which has a win32 error code)
static LPCSTR		g_WinInetMsgWin32A[] = {
	"::HttpQueryInfo failed."
	, "::InternetReadFile failed."
	, "::InternetOpen failed."
	, "::InternetConnect failed."
	, "::HttpOpenRequest failed."
	, "::HttpAddRequestHeaders failed."
	, "::HttpSendRequest failed."
	, "::HttpSendRequestEx failed."
	, "::InternetWriteFile failed."
	, "::HttpEndRequest failed."
	, "::InternetSetOption failed."
} ;

static LPCWSTR		g_WinInetMsgWin32W[] = {
	L"::HttpQueryInfo failed."
	, L"::InternetReadFile failed."
	, L"::InternetOpen failed."
	, L"::InternetConnect failed."
	, L"::HttpOpenRequest failed."
	, L"::HttpAddRequestHeaders failed."
	, L"::HttpSendRequest failed."
	, L"::HttpSendRequestEx failed."
	, L"::InternetWriteFile failed."
	, L"::HttpEndRequest failed."
	, L"::InternetSetOption failed."
} ;

// Win32 API error messages except the WinInet API (They have a win32 error code)
static LPCSTR		g_Win32MsgWin32A[] = {
	"::WideCharToMultiByte failed."
	, "::MultiByteToWideChar failed."
	, "::ReadFile failed."
	, "OpenFile (::CreateFile) failed (\"%s\")."
	, "::SetFilePointer failed."
	, "::GetFileSize failed (\"%s\")."
	, "::WriteFile failed (\"%s\")."
} ;

static LPCWSTR		g_Win32MsgWin32W[] = {
	L"::WideCharToMultiByte failed."
	, L"::MultiByteToWideChar failed."
	, L"::ReadFile failed."
	, L"OpenFile (::CreateFile) failed (\"%s\")."
	, L"::SetFilePointer failed."
	, L"::GetFileSize failed (\"%s\")."
	, L"::WriteFile failed (\"%s\")."
} ;


// user-defined error message
static LPCSTR		g_UsrErrMsgA[] = {
	"user-defined error message"
} ;

static LPCWSTR		g_UsrErrMsgW[] = {
	L"user-defined error message"
} ;


static LPCSTR		HTTPCLIENT_DEF_MIMETYPE = "application/octet-stream" ;

// Constants in the CHttpToolT
LPCSTR CHttpToolA::szDefUsrAgent = "Ryeol HTTP Client Class" ;
LPCSTR CHttpToolA::szGET = "GET" ;
LPCSTR CHttpToolA::szPost = "POST" ;
LPCSTR CHttpToolA::szHTTP = "HTTP://" ;
LPCSTR CHttpToolA::szHTTPS = "HTTPS://" ;
LPCSTR CHttpToolA::szSlash = "/" ;
LPCSTR CHttpToolA::szCacheControl = "Cache-Control" ;
LPCSTR CHttpToolA::szNoCache = "no-cache" ;
LPCSTR CHttpToolA::szContentType = "Content-Type" ;
LPCSTR CHttpToolA::szMultipartFormDataBoundary = "multipart/form-data; boundary=" ;
LPCSTR CHttpToolA::szFormUrlEncoded = "application/x-www-form-urlencoded" ;
LPCSTR CHttpToolA::szDefBoundary = "-----FB3B405B7EAE495aB0C0295C54D4E096" ;
LPCSTR CHttpToolA::szDefUploadContType = "multipart/form-data; boundary=" "-----FB3B405B7EAE495aB0C0295C54D4E096" ;
LPCSTR CHttpToolA::szNULL = "NULL" ;
LPCSTR CHttpToolA::szEmptyString = "" ;
LPCSTR CHttpToolA::szColonSlashSlash = "://" ;


LPCWSTR CHttpToolW::szDefUsrAgent = L"Ryeol HTTP Client Class" ;
LPCWSTR CHttpToolW::szGET = L"GET" ;
LPCWSTR CHttpToolW::szPost = L"POST" ;
LPCWSTR CHttpToolW::szHTTP = L"HTTP://" ;
LPCWSTR CHttpToolW::szHTTPS = L"HTTPS://" ;
LPCWSTR CHttpToolW::szSlash = L"/" ;
LPCWSTR CHttpToolW::szCacheControl = L"Cache-Control" ;
LPCWSTR CHttpToolW::szNoCache = L"no-cache" ;
LPCWSTR CHttpToolW::szContentType = L"Content-Type" ;
LPCWSTR CHttpToolW::szMultipartFormDataBoundary = L"multipart/form-data; boundary=" ;
LPCWSTR CHttpToolW::szFormUrlEncoded = L"application/x-www-form-urlencoded" ;
LPCWSTR CHttpToolW::szDefBoundary = L"-----FB3B405B7EAE495aB0C0295C54D4E096" ;
LPCWSTR CHttpToolW::szDefUploadContType = L"multipart/form-data; boundary=" L"-----FB3B405B7EAE495aB0C0295C54D4E096" ;
LPCWSTR CHttpToolW::szNULL = L"NULL" ;
LPCWSTR CHttpToolW::szEmptyString = L"" ;
LPCWSTR CHttpToolW::szColonSlashSlash = L"://" ;
/////////////////////////////// Global constant message table ////////////////////////////////////


///////////////////////////////////////// Explicit Instanciation /////////////////////////////////////////
template class CHttpClientMapT<CHttpToolA> ;
template class CHttpClientMapT<CHttpToolW> ;
template class CHttpResponseT<CHttpToolA> ;
template class CHttpResponseT<CHttpToolW> ;
template class CHttpPostStatT<CHttpToolA> ;
template class CHttpPostStatT<CHttpToolW> ;
template class CHttpUrlAnalyzerT<CHttpToolA> ;
template class CHttpUrlAnalyzerT<CHttpToolW> ;
template class CHttpClientT<CHttpToolA, CHttpEncoderA> ;
template class CHttpClientT<CHttpToolW, CHttpEncoderW> ;
///////////////////////////////////////// Explicit Instanciation /////////////////////////////////////////


///////////////////////////////////////// httpclientexception /////////////////////////////////////////
/*!
 * This is a default constructor with no argument.
 */
httpclientexceptionA::httpclientexceptionA (void)
	throw ()
{
	m_dwLastError = HTTPCLIENT_ERR_NOT_SPECIFIED ;
	m_dwWin32LastError = NO_ERROR ;
}

/*!
 * This is a constructor with an initial error message and initial error codes.
 * If memory allocation failed for the error message, the error message will not be copied
 * and Internal error message will point to NULL.
 *
 * \param szErrMsg			[in] An initial error message.
 * \param dwLastError		[in] An error code.
 * \param dwWin32LastError	[in] A win32 error code.
 */
httpclientexceptionA::httpclientexceptionA (LPCSTR szErrMsg, DWORD dwLastError, DWORD dwWin32LastError)
	throw ()
: errmsg_exceptionA (szErrMsg)
{
	m_dwLastError = dwLastError ;
	m_dwWin32LastError = dwWin32LastError ;
}

/*!
 * This is a default constructor with no argument.
 */
httpclientexceptionW::httpclientexceptionW (void)
	throw ()
{
	m_dwLastError = HTTPCLIENT_ERR_NOT_SPECIFIED ;
	m_dwWin32LastError = NO_ERROR ;
}

/*!
 * This is a constructor with an initial error message and initial error codes.
 * If memory allocation failed for the error message, the error message will not be copied
 * and Internal error message will point to NULL.
 *
 * \param szErrMsg			[in] An initial error message.
 * \param dwLastError		[in] An error code.
 * \param dwWin32LastError	[in] A win32 error code.
 */
httpclientexceptionW::httpclientexceptionW (LPCWSTR szErrMsg, DWORD dwLastError, DWORD dwWin32LastError)
	throw ()
: errmsg_exceptionW (szErrMsg)
{
	m_dwLastError = dwLastError ;
	m_dwWin32LastError = dwWin32LastError ;
}
///////////////////////////////////////// httpclientexception /////////////////////////////////////////


///////////////////////////////////////// CHttpToolA /////////////////////////////////////////
inline
LPCSTR CHttpToolA::GetConstMessage (DWORD nIdx)
	throw ()
{
	// user-defined error message
	if ( nIdx >= 1000 )
		return g_UsrErrMsgA[0] ;

	// Win32 API error (which has a win32 error code)
	if ( nIdx >= 600 )
		return g_Win32MsgWin32A[nIdx % 100] ;

	// WinInet error (which has a win32 error code)
	if ( nIdx >= 400 )
		return g_WinInetMsgWin32A[nIdx % 100] ;

	// Normal error (which has a win32 error code)
	if ( nIdx >= 200 )
		return g_NormalMsgWin32A[nIdx % 100] ;

	// Normal error
	if ( nIdx >= 100 )
		return g_NormalMsgA[nIdx % 100] ;

	return g_NotSpecifiedA[nIdx] ;
}

void CHttpToolA::ThrowException (DWORD nErrMsgIdx)
	throw (Exception &)
{
	throw Exception (GetConstMessage (nErrMsgIdx), nErrMsgIdx) ;
}

void CHttpToolA::ThrowException (LPCSTR szErrMsg, DWORD nErrMsgIdx)
	throw (Exception &)
{
	throw Exception (szErrMsg, nErrMsgIdx) ;
}

void CHttpToolA::ThrowException (DWORD nErrMsgIdx, DWORD dwErrCode, LPCSTR szStrArg)
	throw (Exception &)
{
	if ( szStrArg == NULL )
		throw Exception (GetConstMessage (nErrMsgIdx), nErrMsgIdx, dwErrCode) ;

	CHAR		szErrMsg[512] ;
	LPCSTR		szFormat = GetConstMessage (nErrMsgIdx) ;
	int			cchWritten = SNPrintf (szErrMsg, 511, szFormat, szStrArg ? szStrArg : "NULL") ;

	// if an error occurs
	if ( cchWritten < -1 )
		throw Exception (szFormat, nErrMsgIdx, dwErrCode) ;

	if ( cchWritten == -1 )
		cchWritten = 511 ;

	szErrMsg[cchWritten] = NULL ;
	throw Exception (szErrMsg, nErrMsgIdx, dwErrCode) ;
}

void CHttpToolA::ThrowException (LPCWSTR szErrMsg, DWORD nErrMsgIdx, DWORD dwErrCode)
	throw (Exception &)
{
	LPSTR		szErrMsgA = NULL ;

	try {
		szErrMsgA = CHttpToolA::Unicode2Ansi (szErrMsg) ;
	} catch (Exception &) {
		szErrMsgA = NULL ;
	}

	Exception		objException (szErrMsgA, nErrMsgIdx, dwErrCode) ;
	SAFEFREE (szErrMsgA) ;
	throw objException ;
}

void CHttpToolA::ThrowException (CHttpToolW::Exception & e)
	throw (Exception &)
{
	ThrowException (e.errmsg (), e.LastError (), e.Win32LastError ()) ;
}

void CHttpToolA::ThrowException (::SafeIntException & e)
	throw (Exception &)
{
	switch ( e.m_code ) {
		case ERROR_ARITHMETIC_OVERFLOW:
			ThrowException (HTTPCLIENT_ERR_ARITHMETIC_OVERFLOW) ;
			break ;

		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			ThrowException (HTTPCLIENT_ERR_INT_DIVIDE_BY_ZERO) ;
			break ;

		default:
			ThrowException (HTTPCLIENT_ERR_UNEXPECTED_ARITHMETIC_ERROR) ;
			break ;
	}
}

// Conversion methods
// This method returns a converted ansi string from a unicode string.
// The returned string must be freed by using the ::free () function.
LPSTR CHttpToolA::Unicode2Ansi (LPCWSTR szStr, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPTOOL_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpToolA::Unicode2Ansi: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	if ( szStr == NULL )
		return NULL ;

	int		cchNeeded ;

	if ( 0 == (cchNeeded = ::WideCharToMultiByte (CodePage, 0, szStr, -1, NULL, 0, NULL, NULL)) )
		ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;

	PSTR		szAnsi = (PSTR) ::malloc (sizeof (CHAR) * cchNeeded) ;
	if ( szAnsi == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	if ( 0 == ::WideCharToMultiByte (CodePage, 0, szStr, -1, szAnsi, cchNeeded, NULL, NULL) ) {
		SAFEFREE (szAnsi) ;
		ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
	}

	return szAnsi ;
}

// This method returns a converted unicode string from a ansi string.
// The returned string must be freed by using the ::free () function.
LPWSTR CHttpToolA::Ansi2Unicode (LPCSTR szStr, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPTOOL_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpToolA::Ansi2Unicode: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	if ( szStr == NULL )
		return NULL ;

	int		cchNeeded ;
	if ( 0 == (cchNeeded = ::MultiByteToWideChar (CodePage, 0, szStr, -1, NULL, 0)) )
		ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

	PWSTR		szUni = (PWSTR) ::malloc (sizeof (WCHAR) * cchNeeded) ;
	if ( szUni == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	if ( 0 == ::MultiByteToWideChar (CodePage, 0, szStr, -1, szUni, cchNeeded) ) {
		SAFEFREE (szUni) ;
		ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;
	}

	return szUni ;
}

// comparison function (used by STL multimap)
bool CHttpToolA::operator () (LPCSTR szKey1, LPCSTR szKey2) const
	throw ()
{
	// return true if the two strings are null
	if ( szKey1 == NULL && szKey2 == NULL )
		return true ;

	if ( szKey1 == NULL )
		return true ;

	if ( szKey2 == NULL )
		return false ;

	// case insensitive
	return ::stricmp (szKey1, szKey2) < 0 ;
}

// Initializes a internet handle.
HINTERNET CHttpToolA::OpenInternet (LPCSTR szUserAgent, DWORD dwAccessType
									, LPCSTR szProxyName, LPCSTR szProxyBypass, DWORD dwFlags)
	throw (Exception &)
{
	HINTERNET		hInternet ;

	if ( NULL == (hInternet = ::InternetOpenA (
			szUserAgent				// user agent
			, dwAccessType			// use direct connection or proxy connection
			, szProxyName
			, szProxyBypass
			, dwFlags)
		) )
		ThrowException (HTTPCLIENT_ERR_INTERNETOPEN_FAILED, ::GetLastError ()) ;

	return hInternet ;
}

// Closes a internet handle
void CHttpToolA::CloseInternet (HINTERNET & hInternet)
	throw ()
{
	if ( hInternet == NULL )
		return ;

	// Ignore any errors while closing the handle
	::InternetCloseHandle (hInternet) ;
	hInternet = NULL ;
}

// Returns a connection handle
// The hInternet must be a valid internet handle.
HINTERNET CHttpToolA::OpenConnection (HINTERNET hInternet, LPCSTR szServerAddr, INTERNET_PORT nPort
											, LPCSTR szUsrName, LPCSTR szUsrPwd)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hInternet != NULL, "CHttpToolA::OpenConnection: hInternet can not be NULL.") ;
	HTTPTOOL_ASSERT (szServerAddr != NULL, "CHttpToolA::OpenConnection: szServerAddr can not be NULL.") ;
	HTTPTOOL_ASSERT (::strlen (szServerAddr) != 0, "CHttpToolA::OpenConnection: szServerAddr can not be an empty string.") ;

	HINTERNET			hConnection ;

	if ( NULL == (hConnection = ::InternetConnectA (
			hInternet
			, szServerAddr
			, nPort
			, szUsrName
			, szUsrPwd
			, INTERNET_SERVICE_HTTP
			, 0
			, NULL)
		) )
		ThrowException (HTTPCLIENT_ERR_INTERNETCONNECT_FAILED, ::GetLastError ()) ;

	return hConnection ;
}

// Closes a connection handle
void CHttpToolA::CloseConnection (HINTERNET & hConnection)
	throw ()
{
	if ( hConnection == NULL )
		return ;

	// Ignore any errors while closing the handle
	::InternetCloseHandle (hConnection) ;
	hConnection = NULL ;
}

// Returns a HTTP request handle
HINTERNET CHttpToolA::OpenRequest (HINTERNET hConnection, LPCSTR szMethod, LPCSTR szObjectName
										 , DWORD dwFlags, LPCSTR szReferer, UINT /* CodePage */)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hConnection != NULL, "CHttpToolA::OpenRequest: hConnection can not be NULL.") ;
	HTTPTOOL_ASSERT (szObjectName != NULL, "CHttpToolA::OpenRequest: szObjectName can not be NULL.") ;
	HTTPTOOL_ASSERT (::strlen (szObjectName) != 0, "CHttpToolA::OpenRequest: szObjectName can not be an empty string.") ;

	static LPCSTR			szAcceptedType[] = {
		"*/*"
		, NULL
	} ;

	HINTERNET		hRequest = NULL ;

	// Opens a HTTP request handle
	if ( NULL == (hRequest = ::HttpOpenRequestA (
			hConnection
			, szMethod				// HTTP Method
			, szObjectName			// Target object
			, "HTTP/1.0"			// HTTP/1.1
			, szReferer				// referer
			, szAcceptedType		// Accepts any type.
			, dwFlags				// Flags
			, NULL					// Doesn't use any context value.
			)
		) )
		ThrowException (HTTPCLIENT_ERR_HTTPOPENREQUEST_FAILED, ::GetLastError ()) ;

	return hRequest ;
}

// Closes a request handle
void CHttpToolA::CloseRequest (HINTERNET & hRequest)
	throw ()
{
	if ( hRequest == NULL )
		return ;

	// Ignore any errors while closing the handle
	::InternetCloseHandle (hRequest) ;
	hRequest = NULL ;
}

void CHttpToolA::AddHeader (HINTERNET hRequest, LPCSTR szName, LPCSTR szValue, UINT /* CodePage */)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolA::AddHeader: hRequest can not be NULL.") ;
	HTTPTOOL_ASSERT (szName != NULL, "CHttpToolA::AddHeader: szName can not be NULL.") ;

	::SafeInt<size_t>		cbHeader ;
	::SafeInt<DWORD>		cchHeader ;

	try {
		cbHeader = ::strlen (szName) ;
		cbHeader += szValue ? ::strlen (szValue) : 0 ;
		cbHeader += (4 + 1) ;	// for ": ", "\r\n", '\0'
		cchHeader = cbHeader - 1 ;
	} catch (::SafeIntException & e) {
		ThrowException (e) ;
	}

	PSTR		szHeader = (PSTR) ::malloc (sizeof (CHAR) * (cbHeader.Value ())) ;
	if ( szHeader == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	::strcpy (szHeader, szName) ;
	::strcat (szHeader, ": ") ;
	::strcat (szHeader, szValue ? szValue : "") ;
	::strcat (szHeader, "\r\n") ;

	// Adds a header
	if ( !::HttpAddRequestHeadersA (
			hRequest
			, szHeader								// headers to append to the request.
			, cchHeader.Value ()					// header length
			, HTTP_ADDREQ_FLAG_ADD					// flags
			)
		) {
		SAFEFREE (szHeader) ;
		ThrowException (HTTPCLIENT_ERR_HTTPADDREQUESTHEADERS_FAILED, ::GetLastError ()) ;
	}

	SAFEFREE (szHeader) ;
}

void CHttpToolA::SendRequest (HINTERNET hRequest, LPCSTR szPosted, UINT /* CodePage */)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolA::SendRequest: hRequest can not be NULL.") ;

	::SafeInt<DWORD>	cchPosted ;

	try {
		cchPosted = szPosted ? ::strlen (szPosted) : 0 ;
	} catch (::SafeIntException & e) {
		ThrowException (e) ;
	}

	if ( !::HttpSendRequestA (
			hRequest
			, NULL						// Additional header
			, 0							// The length of the additional header
			, (void *) szPosted			// A posted data
			, cchPosted.Value ()		// The length of the posted data
		) )
		ThrowException (HTTPCLIENT_ERR_HTTPSENDREQUEST_FAILED, ::GetLastError ()) ;
}

void CHttpToolA::SendRequestEx (HINTERNET hRequest, DWORD dwPostedSize)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolA::SendRequestEx: hRequest can not be NULL.") ;

	INTERNET_BUFFERSA BufferIn;

	BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS ); // Must be set or error will occur
    BufferIn.Next = NULL; 
    BufferIn.lpcszHeader = NULL;
    BufferIn.dwHeadersLength = 0;
    BufferIn.dwHeadersTotal = 0;
    BufferIn.lpvBuffer = NULL;                
    BufferIn.dwBufferLength = 0;
    BufferIn.dwBufferTotal = dwPostedSize; // This is the only member used other than dwStructSize
    BufferIn.dwOffsetLow = 0;
    BufferIn.dwOffsetHigh = 0;

	if ( !::HttpSendRequestExA (
			hRequest
			, &BufferIn
			, NULL
			, 0
			, 0
		) )
		ThrowException (HTTPCLIENT_ERR_HTTPSENDREQUESTEX_FAILED, ::GetLastError ()) ;
}

void CHttpToolA::InternetWriteFile (HINTERNET hRequest, const BYTE * pbyBuff, DWORD cbyBuff)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolA::InternetWriteFile: hRequest can not be NULL.") ;
	HTTPTOOL_ASSERT (pbyBuff != NULL, "CHttpToolA::InternetWriteFile: pbyBuff can not be NULL.") ;
	HTTPTOOL_ASSERT (cbyBuff != 0, "CHttpToolA::InternetWriteFile: cbyBuff can not be zero.") ;

	DWORD		dwTotalWritten = 0, dwWritten ;

	while ( dwTotalWritten < cbyBuff ) {
		if ( !::InternetWriteFile (
							hRequest
							, pbyBuff + dwTotalWritten
							, cbyBuff - dwTotalWritten
							, &dwWritten
				)
			)
			ThrowException (HTTPCLIENT_ERR_INTERNETWRITEFILE_FAILED, ::GetLastError ()) ;

		dwTotalWritten += dwWritten ;
	}
}

void CHttpToolA::EndRequest (HINTERNET hRequest)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolA::EndRequest: hRequest can not be NULL.") ;

	if ( !::HttpEndRequest (
						hRequest
						, NULL
						, 0
						, 0
			)
		)
		ThrowException (HTTPCLIENT_ERR_HTTPENDREQUEST_FAILED, ::GetLastError ()) ;
}

// Checks whether the file exists.
// (Does not throw an exception)
BOOL CHttpToolA::FileExists (LPCSTR szFilePath)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (szFilePath != NULL, "CHttpToolA::FileExists: szFilePath can not be NULL.") ;
	
	HANDLE		hFile = ::CreateFileA (
							szFilePath
							, 0
							, FILE_SHARE_READ
							, NULL
							, OPEN_EXISTING
							, FILE_ATTRIBUTE_NORMAL
							, NULL) ;

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle (hFile) ;
		return TRUE ;
	}

	return FALSE ;
}

// It returns a INVALID_HANDLE_VALUE if the specified file is not valid.
// (Does not throw an exception)
HANDLE CHttpToolA::OpenFile (LPCSTR szFilePath)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (szFilePath != NULL, "CHttpToolA::OpenFile: szFilePath can not be NULL.") ;

	return ::CreateFileA (
			szFilePath
			, GENERIC_READ
			, FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN
			, NULL) ;
}

// If it fails to create an file, it will return INVALID_HANDLE_VALUE.
// (Does not throw an exception)
HANDLE CHttpToolA::CreateFileAlwaysToWrite (LPCSTR szFilePath)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (szFilePath != NULL, "CHttpToolA::CreateFileAlwaysToWrite: szFilePath can not be NULL.") ;

	return ::CreateFileA (
			szFilePath
			, GENERIC_WRITE
			, 0
			, NULL
			, CREATE_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL) ;
}




DWORD CHttpToolA::GetFileSize (HANDLE hFile, LPCSTR szFilePath)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hFile != NULL, "CHttpToolA::GetFileSize: hFile can not be NULL.") ;
	HTTPTOOL_ASSERT (szFilePath != NULL, "CHttpToolA::GetFileSize: szFilePath can not be NULL.") ;

	DWORD		dwFileSize = ::GetFileSize (hFile, NULL) ;
	if ( dwFileSize == INVALID_FILE_SIZE )
		ThrowException (HTTPCLIENT_ERR_GETFILESIZE_FAILED, ::GetLastError (), szFilePath) ;

	return dwFileSize ;
}

// The file handle must point to BOF. If the file handle is INVALID_HANDLE_VALUE
// The default mime type is returned.
// The returned string must be freed by using the ::free () function.
LPSTR CHttpToolA::GetMimeType (HANDLE hFile, UINT CodePage)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hFile != NULL, "CHttpToolA::GetMimeType: hFile can not be NULL.") ;

	LPSTR		szMimeType = NULL ;
	try {
		szMimeType = CHttpToolW::GetMimeType (hFile, CodePage) ;
	} catch (CHttpToolW::Exception & e) {
		ThrowException (e) ;
	}

	return szMimeType ;
}

// Returns the HTTP status text from the HTTP request handle.
// The returned string must be freed by using the ::free () function.
LPSTR CHttpToolA::GetStatusText (HINTERNET hRequest)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolA::GetStatusText: hRequest can not be NULL.") ;

	DWORD			cbBuff = 0 ;
	LPSTR			szStatusText = NULL ;

	// Get required buffer size
	if ( !::HttpQueryInfoA (
			hRequest
			, HTTP_QUERY_STATUS_TEXT 		// Get the status text
			, static_cast<void *> (szStatusText)
			, &cbBuff						// Required buffer size (byte)
			, NULL							// Don't use a header index
			)
		)
		if ( ::GetLastError () != ERROR_INSUFFICIENT_BUFFER )
			ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;

	_ASSERTE ( cbBuff != 0 ) ;

	szStatusText = (LPSTR) ::malloc (cbBuff) ;
	if ( szStatusText == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	// Get the status text
	if ( !::HttpQueryInfoA (
			hRequest
			, HTTP_QUERY_STATUS_TEXT 		// Get the status text
			, static_cast<void *> (szStatusText)
			, &cbBuff						// Allocated buffer size (byte)
			, NULL							// Don't use a header index
			)
		) {
		::free (szStatusText) ;
		ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;
	}

	return szStatusText ;
}

// Get the HTTP header which has the name specified by szName from the HTTP request handle.
// If the header is not found, NULL is returned.
// The returned string must be freed by using the ::free () function.
// pnIdx exactly corresponds to the lpdwIndex parameter in the ::HttpQueryInfo function.
// For more information about this parameter, see microsoft's SDK documentation.
LPSTR CHttpToolA::GetHeader (HINTERNET hRequest, LPCSTR szName, DWORD * pnIdx)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolA::GetHeader: hRequest can not be NULL.") ;
	HTTPTOOL_ASSERT (szName != NULL, "CHttpToolA::GetHeader: szName can not be NULL.") ;

	DWORD			nOrigIdx = 0 ;
	if ( pnIdx )	nOrigIdx = *pnIdx ;

	// Copy the header name
	::SafeInt<DWORD>	cbBuff ;
	try {
		cbBuff = ::strlen (szName) ;
		cbBuff++ ;
		cbBuff *= sizeof (CHAR) ;
	} catch (::SafeIntException & e) {
		ThrowException (e) ;
	}

	PSTR		szHeader = (PSTR) ::malloc (cbBuff.Value ()) ;
	if ( szHeader == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
	::strcpy (szHeader, szName) ;

	if ( ::HttpQueryInfoA (
			hRequest
			, HTTP_QUERY_CUSTOM				// Get a custom header
			, static_cast<void *> (szHeader)
			, cbBuff.Ptr ()
			, pnIdx	
			)
		)
		return szHeader ;

	SAFEFREE (szHeader) ;
	if ( pnIdx )	*pnIdx = nOrigIdx ;

	// If the function failed
	if ( ::GetLastError () != ERROR_INSUFFICIENT_BUFFER ) {
		// If the header does not exist
		if ( ::GetLastError () == ERROR_HTTP_HEADER_NOT_FOUND )
			return NULL ;

		ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;
	}

	// Allocates required memory
	szHeader = (LPSTR) ::malloc (cbBuff.Value ()) ;
	if ( szHeader == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
	::strcpy (szHeader, szName) ;

	if ( !::HttpQueryInfoA (
			hRequest
			, HTTP_QUERY_CUSTOM				// Get a custom header
			, static_cast<void *> (szHeader)
			, cbBuff.Ptr ()
			, pnIdx	
			)
		) {
		SAFEFREE (szHeader) ;
		ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;
	}

	return szHeader ;
}

void CHttpToolA::InternetSetOption (HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength)
	throw (Exception &)
{
	if ( ::InternetSetOptionA (hInternet, dwOption, lpBuffer, dwBufferLength) )
		return ;

	ThrowException (HTTPCLIENT_ERR_INTERNETSETOPTION_FAILED, ::GetLastError ()) ;
}

// Generates a new upload boundary. If an error occurs, NULL is returned
// The returned string must be freed by using the ::free () function.
LPSTR CHttpToolA::CreateUploadBoundary (void)
	throw ()
{
	GUID			guid ;

	if ( FAILED ( ::CoCreateGuid (&guid)) )
		return NULL ;

	PSTR		szBoundary = (PSTR) ::malloc (sizeof (CHAR) * 44) ;

	if ( szBoundary == NULL )
		return NULL ;

	::sprintf (szBoundary, "--------%.08x%.04x%.04x%.02x%.02x%.02x%.02x%.02x%.02x%.02x%.02x"
		, guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1]
		, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return szBoundary ;
}
///////////////////////////////////////// CHttpToolA /////////////////////////////////////////


///////////////////////////////////////// CHttpToolW /////////////////////////////////////////
inline
LPCWSTR CHttpToolW::GetConstMessage (int nIdx)
	throw ()
{
	// user-defined error message
	if ( nIdx >= 1000 )
		return g_UsrErrMsgW[0] ;

	// Win32 API error (which has a win32 error code)
	if ( nIdx >= 600 )
		return g_Win32MsgWin32W[nIdx % 100] ;

	// WinInet error (which has a win32 error code)
	if ( nIdx >= 400 )
		return g_WinInetMsgWin32W[nIdx % 100] ;

	// Normal error (which has a win32 error code)
	if ( nIdx >= 200 )
		return g_NormalMsgWin32W[nIdx % 100] ;

	// Normal error
	if ( nIdx >= 100 )
		return g_NormalMsgW[nIdx % 100] ;

	return g_NotSpecifiedW[nIdx] ;
}

void CHttpToolW::ThrowException (DWORD nErrMsgIdx)
	throw (Exception &)
{
	throw Exception (GetConstMessage (nErrMsgIdx), nErrMsgIdx) ;
}

void CHttpToolW::ThrowException (LPCWSTR szErrMsg, DWORD nErrMsgIdx)
	throw (Exception &)
{
	throw Exception (szErrMsg, nErrMsgIdx) ;
}

void CHttpToolW::ThrowException (DWORD nErrMsgIdx, DWORD dwErrCode, LPCWSTR szStrArg)
	throw (Exception &)
{
	if ( szStrArg == NULL )
		throw Exception (GetConstMessage (nErrMsgIdx), nErrMsgIdx, dwErrCode) ;

	WCHAR		szErrMsg[512] ;
	LPCWSTR		szFormat = GetConstMessage (nErrMsgIdx) ;
	int			cchWritten = SNPrintf (szErrMsg, 511, szFormat, szStrArg ? szStrArg : L"NULL") ;

	// if an error occurs
	if ( cchWritten < -1 )
		throw Exception (szFormat, nErrMsgIdx, dwErrCode) ;

	if ( cchWritten == -1 )
		cchWritten = 511 ;

	szErrMsg[cchWritten] = NULL ;
	throw Exception (szErrMsg, nErrMsgIdx, dwErrCode) ;
}

void CHttpToolW::ThrowException (LPCSTR szErrMsg, DWORD nErrMsgIdx, DWORD dwErrCode)
	throw (Exception &)
{
	LPWSTR		szErrMsgW = NULL ;

	try {
		szErrMsgW = Ansi2Unicode (szErrMsg) ;
	} catch (Exception &) {
		;
	}

	Exception		objException (szErrMsgW, nErrMsgIdx, dwErrCode) ;
	SAFEFREE (szErrMsgW) ;
	throw objException ;
}

void CHttpToolW::ThrowException (httpclientexceptionA & e)
	throw (Exception &)
{
	ThrowException (e.errmsg (), e.LastError (), e.Win32LastError ()) ;
}

void CHttpToolW::ThrowException (::SafeIntException & e)
	throw (Exception &)
{
	switch ( e.m_code ) {
		case ERROR_ARITHMETIC_OVERFLOW:
			ThrowException (HTTPCLIENT_ERR_ARITHMETIC_OVERFLOW) ;
			break ;

		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			ThrowException (HTTPCLIENT_ERR_INT_DIVIDE_BY_ZERO) ;
			break ;

		default:
			ThrowException (HTTPCLIENT_ERR_UNEXPECTED_ARITHMETIC_ERROR) ;
			break ;
	}
}


// Conversion methods
// This function returns a converted ansi string from a unicode string.
// The returned string must be freed by using the ::free () function.
LPSTR CHttpToolW::Unicode2Ansi (LPCWSTR szStr, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPTOOL_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpToolW::Unicode2Ansi: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	if ( szStr == NULL )
		return NULL ;

	int		cchNeeded ;

	if ( 0 == (cchNeeded = ::WideCharToMultiByte (CodePage, 0, szStr, -1, NULL, 0, NULL, NULL)) )
		ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;

	PSTR		szAnsi = (PSTR) ::malloc (sizeof (CHAR) * cchNeeded) ;
	if ( szAnsi == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	if ( 0 == ::WideCharToMultiByte (CodePage, 0, szStr, -1, szAnsi, cchNeeded, NULL, NULL) ) {
		SAFEFREE (szAnsi) ;
		ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
	}

	return szAnsi ;
}

// This method returns a converted unicode string from a ansi string.
// The returned string must be freed by using the ::free () function.
LPWSTR CHttpToolW::Ansi2Unicode (LPCSTR szStr, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPTOOL_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpToolW::Ansi2Unicode: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	if ( szStr == NULL )
		return NULL ;

	int		cchNeeded ;
	if ( 0 == (cchNeeded = ::MultiByteToWideChar (CodePage, 0, szStr, -1, NULL, 0)) )
		ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

	PWSTR		szUni = (PWSTR) ::malloc (sizeof (WCHAR) * cchNeeded) ;
	if ( szUni == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	if ( 0 == ::MultiByteToWideChar (CodePage, 0, szStr, -1, szUni, cchNeeded) ) {
		SAFEFREE (szUni) ;
		ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;
	}

	return szUni ;
}


// comparison function (used by STL multimap)
bool CHttpToolW::operator () (LPCWSTR szKey1, LPCWSTR szKey2) const
	throw ()
{
	// return true if the two strings are null
	if ( szKey1 == NULL && szKey2 == NULL )
		return true ;

	if ( szKey1 == NULL )
		return true ;

	if ( szKey2 == NULL )
		return false ;

	// case insensitive
	return ::wcsicmp (szKey1, szKey2) < 0 ;
}

// Initializes a internet handle.
HINTERNET CHttpToolW::OpenInternet (LPCWSTR szUserAgent, DWORD dwAccessType
									, LPCWSTR szProxyName, LPCWSTR szProxyBypass, DWORD dwFlags)
	throw (Exception &)
{
	HINTERNET		hInternet ;

	if ( NULL == (hInternet = InternetOpenW (
			szUserAgent				// user agent
			, dwAccessType			// use direct connection or proxy connection
			, szProxyName
			, szProxyBypass
			, dwFlags)
		) )
		ThrowException (HTTPCLIENT_ERR_INTERNETOPEN_FAILED, ::GetLastError ()) ;

	return hInternet ;
}

// Closes a internet handle
void CHttpToolW::CloseInternet (HINTERNET & hInternet)
	throw ()
{
	CHttpToolA::CloseInternet (hInternet) ;
}

// Returns a connection handle
// The hInternet must be a valid internet handle.
HINTERNET CHttpToolW::OpenConnection (HINTERNET hInternet, LPCWSTR szServerAddr, INTERNET_PORT nPort
											, LPCWSTR szUsrName, LPCWSTR szUsrPwd)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hInternet != NULL, "CHttpToolW::OpenConnection: hInternet can not be NULL.") ;
	HTTPTOOL_ASSERT (szServerAddr != NULL, "CHttpToolW::OpenConnection: szServerAddr can not be NULL.") ;
	HTTPTOOL_ASSERT (::wcslen (szServerAddr) != 0, "CHttpToolW::OpenConnection: szServerAddr can not be an empty string.") ;

	HINTERNET			hConnection ;

	if ( NULL == (hConnection = ::InternetConnectW (
			hInternet
			, szServerAddr
			, nPort
			, szUsrName
			, szUsrPwd
			, INTERNET_SERVICE_HTTP
			, 0
			, NULL)
		) )
		ThrowException (HTTPCLIENT_ERR_INTERNETCONNECT_FAILED, ::GetLastError ()) ;

	return hConnection ;
}

// Closes a connection handle
void CHttpToolW::CloseConnection (HINTERNET & hConnection)
	throw ()
{
	CHttpToolA::CloseConnection (hConnection) ;
}

// Returns a HTTP request handle
HINTERNET CHttpToolW::OpenRequest (HINTERNET hConnection, LPCWSTR szMethod, LPCWSTR szObjectName
										 , DWORD dwFlags, LPCWSTR szReferer, UINT CodePage)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hConnection != NULL, "CHttpToolW::OpenRequest: hConnection can not be NULL.") ;
	HTTPTOOL_ASSERT (szObjectName != NULL, "CHttpToolW::OpenRequest: szObjectName can not be NULL.") ;
	HTTPTOOL_ASSERT (::wcslen (szObjectName) != 0, "CHttpToolW::OpenRequest: szObjectName can not be an empty string.") ;

	// The unicode encodings are not allowed
	HTTPTOOL_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpToolW::OpenRequest: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	LPSTR		szMethodA = NULL ;
	LPSTR		szObjectNameA = NULL ;
	LPSTR		szRefererA = NULL ;

	HINTERNET		hRequest = NULL ;

	try {
		szMethodA = Unicode2Ansi (szMethod, CodePage) ;
		szObjectNameA = Unicode2Ansi (szObjectName, CodePage) ;
		szRefererA = Unicode2Ansi (szReferer, CodePage) ;

		hRequest = CHttpToolA::OpenRequest (hConnection, szMethodA, szObjectNameA
													, dwFlags, szRefererA) ;
	}  catch (Exception &) {
		SAFEFREE (szMethodA) ;
		SAFEFREE (szObjectNameA) ;
		SAFEFREE (szRefererA) ;
		throw ;
	} catch (CHttpToolA::Exception & e) {
		SAFEFREE (szMethodA) ;
		SAFEFREE (szObjectNameA) ;
		SAFEFREE (szRefererA) ;
		ThrowException (e) ;
	}

	SAFEFREE (szMethodA) ;
	SAFEFREE (szObjectNameA) ;
	SAFEFREE (szRefererA) ;

	return hRequest ;
}

// Closes a request handle
void CHttpToolW::CloseRequest (HINTERNET & hRequest)
	throw ()
{
	CHttpToolA::CloseRequest (hRequest) ;
}

void CHttpToolW::AddHeader (HINTERNET hRequest, LPCWSTR szName, LPCWSTR szValue, UINT CodePage)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolW::AddHeader: hRequest can not be NULL.") ;
	HTTPTOOL_ASSERT (szName != NULL, "CHttpToolW::AddHeader: szName can not be NULL.") ;

	// The unicode encodings are not allowed
	HTTPTOOL_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpToolW::AddHeader: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	LPSTR		szNameA = NULL ;
	LPSTR		szValueA = NULL ;

	try {
		szNameA = Unicode2Ansi (szName, CodePage) ;
		szValueA = Unicode2Ansi (szValue, CodePage) ;

		CHttpToolA::AddHeader (hRequest, szNameA, szValueA) ;
	}  catch (Exception &) {
		SAFEFREE (szNameA) ;
		SAFEFREE (szValueA) ;
		throw ;
	} catch (CHttpToolA::Exception & e) {
		SAFEFREE (szNameA) ;
		SAFEFREE (szValueA) ;
		ThrowException (e) ;
	}
	SAFEFREE (szNameA) ;
	SAFEFREE (szValueA) ;
}

void CHttpToolW::SendRequest (HINTERNET hRequest, LPCWSTR szPosted, UINT CodePage)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolW::SendRequest: hRequest can not be NULL.") ;

	// The unicode encodings are not allowed
	HTTPTOOL_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpToolW::SendRequest: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	LPSTR		szPostedA = Unicode2Ansi (szPosted, CodePage) ;

	::SafeInt<DWORD>	cchPosted ;
	try {
		cchPosted = szPostedA ? ::strlen (szPostedA) : 0 ;
	} catch (::SafeIntException & e) {
		SAFEFREE (szPostedA) ;
		ThrowException (e) ;
	}

	if ( !::HttpSendRequestW (
			hRequest
			, NULL						// Additional header
			, 0							// The length of the additional header
			, (void *) szPostedA		// A posted data
			, cchPosted.Value ()		// The length of the posted data
		) ) {
		SAFEFREE (szPostedA) ;
		ThrowException (HTTPCLIENT_ERR_HTTPSENDREQUEST_FAILED, ::GetLastError ()) ;
	}
	SAFEFREE (szPostedA) ;
}

void CHttpToolW::SendRequestEx (HINTERNET hRequest, DWORD dwPostedSize)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolW::SendRequestEx: hRequest can not be NULL.") ;

	INTERNET_BUFFERSW BufferIn;

	BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS ); // Must be set or error will occur
    BufferIn.Next = NULL; 
    BufferIn.lpcszHeader = NULL;
    BufferIn.dwHeadersLength = 0;
    BufferIn.dwHeadersTotal = 0;
    BufferIn.lpvBuffer = NULL;                
    BufferIn.dwBufferLength = 0;
    BufferIn.dwBufferTotal = dwPostedSize; // This is the only member used other than dwStructSize
    BufferIn.dwOffsetLow = 0;
    BufferIn.dwOffsetHigh = 0;

	if ( !::HttpSendRequestExW (
			hRequest
			, &BufferIn
			, NULL
			, 0
			, 0
		) )
		ThrowException (HTTPCLIENT_ERR_HTTPSENDREQUESTEX_FAILED, ::GetLastError ()) ;
}

void CHttpToolW::InternetWriteFile (HINTERNET hRequest, const BYTE * pbyBuff, DWORD cbyBuff)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolW::InternetWriteFile: hRequest can not be NULL.") ;
	HTTPTOOL_ASSERT (pbyBuff != NULL, "CHttpToolW::InternetWriteFile: pbyBuff can not be NULL.") ;
	HTTPTOOL_ASSERT (cbyBuff != 0, "CHttpToolW::InternetWriteFile: cbyBuff can not be zero.") ;

	DWORD		dwTotalWritten = 0, dwWritten ;

	while ( dwTotalWritten < cbyBuff ) {
		if ( !::InternetWriteFile (
							hRequest
							, pbyBuff + dwTotalWritten
							, cbyBuff - dwTotalWritten
							, &dwWritten
				)
			)
			ThrowException (HTTPCLIENT_ERR_INTERNETWRITEFILE_FAILED, ::GetLastError ()) ;

		dwTotalWritten += dwWritten ;
	}
}

void CHttpToolW::EndRequest (HINTERNET hRequest)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolW::EndRequest: hRequest can not be NULL.") ;

	if ( !::HttpEndRequest (
						hRequest
						, NULL
						, 0
						, 0
			)
		)
		ThrowException (HTTPCLIENT_ERR_HTTPENDREQUEST_FAILED, ::GetLastError ()) ;
}

// Checks whether the file exists.
// (Does not throw an exception)
BOOL CHttpToolW::FileExists (LPCWSTR szFilePath)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (szFilePath != NULL, "CHttpToolW::FileExists: szFilePath can not be NULL.") ;
	
	HANDLE		hFile = ::CreateFileW (
							szFilePath
							, 0
							, FILE_SHARE_READ
							, NULL
							, OPEN_EXISTING
							, FILE_ATTRIBUTE_NORMAL
							, NULL) ;

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle (hFile) ;
		return TRUE ;
	}

	return FALSE ;
}

// It returns a INVALID_HANDLE_VALUE if the specified file is not valid.
// (Does not throw an exception)
HANDLE CHttpToolW::OpenFile (LPCWSTR szFilePath)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (szFilePath != NULL, "CHttpToolW::OpenFile: szFilePath can not be NULL.") ;

	return ::CreateFileW (
			szFilePath
			, GENERIC_READ
			, FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN
			, NULL) ;
}

// If it fails to create an file, it will return INVALID_HANDLE_VALUE.
// (Does not throw an exception)
HANDLE CHttpToolW::CreateFileAlwaysToWrite (LPCWSTR szFilePath)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (szFilePath != NULL, "CHttpToolW::CreateFileAlwaysToWrite: szFilePath can not be NULL.") ;

	return ::CreateFileW (
			szFilePath
			, GENERIC_WRITE
			, 0
			, NULL
			, CREATE_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL) ;
}

DWORD CHttpToolW::GetFileSize (HANDLE hFile, LPCWSTR szFilePath)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hFile != NULL, "CHttpToolW::GetFileSize: hFile can not be NULL.") ;
	HTTPTOOL_ASSERT (szFilePath != NULL, "CHttpToolW::GetFileSize: szFilePath can not be NULL.") ;

	DWORD		dwFileSize = ::GetFileSize (hFile, NULL) ;
	if ( dwFileSize == INVALID_FILE_SIZE )
		ThrowException (HTTPCLIENT_ERR_GETFILESIZE_FAILED, ::GetLastError (), szFilePath) ;

	return dwFileSize ;
}

// The file handle must point to BOF. If the file handle is INVALID_HANDLE_VALUE
// The default mime type is returned.
// The returned string must be freed by using the ::free () function.
LPSTR CHttpToolW::GetMimeType (HANDLE hFile, UINT CodePage)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hFile != NULL, "CHttpToolW::GetMimeType: hFile can not be NULL.") ;

	// The unicode encodings are not allowed
	HTTPTOOL_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpToolW::GetMimeType: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	// If the file handle is not valid, just returns the default MimeType.
	if ( hFile == INVALID_HANDLE_VALUE ) {
		PSTR		szMimeA = NULL ;
		szMimeA = (PSTR) ::malloc (sizeof (CHAR) * (::strlen (HTTPCLIENT_DEF_MIMETYPE) + 1)) ;
		if ( szMimeA == NULL )
			ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
		::strcpy (szMimeA, HTTPCLIENT_DEF_MIMETYPE) ;
		return szMimeA ;
	}

	BYTE		byBuff[256] ;
	DWORD		dwRead ;

	if ( 0 == ::ReadFile (hFile
				, byBuff
				, 256
				, &dwRead
				, NULL) )
		ThrowException (HTTPCLIENT_ERR_READFILE_FAILED, ::GetLastError ()) ;

	// Moves the file pointer to the beginning of the file.
	if ( INVALID_SET_FILE_POINTER == ::SetFilePointer (hFile, 0, NULL, FILE_BEGIN) )
		ThrowException (HTTPCLIENT_ERR_SETFILEPOINTER_FAILED, ::GetLastError ()) ;

	PWSTR		szMimeW = NULL ;
	PSTR		szMimeA = NULL ;
	HRESULT		hResult ;

	// If the ::FindMimeFromData function failed to get a appropriate MimeType,
	// just returns the default MimeType.
	if ( NOERROR != (hResult = ::FindMimeFromData (
						NULL
						, NULL
						, byBuff
						, dwRead
						, NULL
						, 0
						, &szMimeW
						, 0)) ) {
		szMimeA = (PSTR) ::malloc (sizeof (CHAR) * (::strlen (HTTPCLIENT_DEF_MIMETYPE) + 1)) ;
		if ( szMimeA == NULL )
			ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
		::strcpy (szMimeA, HTTPCLIENT_DEF_MIMETYPE) ;
		return szMimeA ;
	}

	try {
		szMimeA = Unicode2Ansi (szMimeW, CodePage) ;
	} catch (Exception &) {
		::CoTaskMemFree (szMimeW) ;
		throw ;
	}

	::CoTaskMemFree (szMimeW) ;
	if(!lstrcmpA(szMimeA,"image/x-png")) lstrcpyA(szMimeA, "image/png");
	return szMimeA ;
}

// Returns the HTTP status text from the HTTP request handle.
// The returned string must be freed by using the ::free () function.
LPWSTR CHttpToolW::GetStatusText (HINTERNET hRequest)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolW::GetStatusText: hRequest can not be NULL.") ;

	DWORD			cbBuff = 0 ;
	PWSTR			szStatusText = NULL ;

	// Get required buffer size
	if ( !::HttpQueryInfoW (
			hRequest
			, HTTP_QUERY_STATUS_TEXT 		// Get the status text
			, static_cast<void *> (szStatusText)
			, &cbBuff						// Required buffer size (byte)
			, NULL							// Don't use a header index
			)
		)
		if ( ::GetLastError () != ERROR_INSUFFICIENT_BUFFER )
			ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;

	_ASSERTE ( cbBuff != 0 ) ;

	szStatusText = (PWSTR) ::malloc (cbBuff) ;
	if ( szStatusText == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	// Get the status text
	if ( !::HttpQueryInfoW (
			hRequest
			, HTTP_QUERY_STATUS_TEXT 		// Get the status text
			, static_cast<void *> (szStatusText)
			, &cbBuff						// Required buffer size (byte)
			, NULL							// Don't use a header index
			)
		) {
		::free (szStatusText) ;
		ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;
	}

	return szStatusText ;
}

// Get the HTTP header which has the name specified by szName from the HTTP request handle.
// If the header is not found, NULL is returned.
// The returned string must be freed by using the ::free () function.
// pnIdx exactly corresponds to the lpdwIndex parameter in the ::HttpQueryInfo function.
// For more information about this parameter, see microsoft's SDK documentation.
LPWSTR CHttpToolW::GetHeader (HINTERNET hRequest, LPCWSTR szName, DWORD * pnIdx)
	throw (Exception &)
{
	HTTPTOOL_ASSERT (hRequest != NULL, "CHttpToolW::GetHeader: hRequest can not be NULL.") ;
	HTTPTOOL_ASSERT (szName != NULL, "CHttpToolW::GetHeader: szName can not be NULL.") ;

	DWORD			nOrigIdx = 0 ;
	if ( pnIdx )	nOrigIdx = *pnIdx ;


	// Copy the header name
	::SafeInt<DWORD>		cbBuff ;
	try {
		cbBuff = ::wcslen (szName) ;
		cbBuff++ ;		// for '\0' character
		cbBuff *= sizeof (WCHAR) ;
	} catch (::SafeIntException & e) {
		ThrowException (e) ;
	}

	PWSTR		szHeader = (PWSTR) ::malloc (cbBuff.Value ()) ;
	if ( szHeader == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
	::wcscpy (szHeader, szName) ;

	if ( ::HttpQueryInfoW (
			hRequest
			, HTTP_QUERY_CUSTOM				// Get a custom header
			, static_cast<void *> (szHeader)
			, cbBuff.Ptr ()
			, pnIdx	
			)
		)
		return szHeader ;

	SAFEFREE (szHeader) ;
	if ( pnIdx )	*pnIdx = nOrigIdx ;

	// If the function failed
	if ( ::GetLastError () != ERROR_INSUFFICIENT_BUFFER ) {
		// If the header does not exist
		if ( ::GetLastError () == ERROR_HTTP_HEADER_NOT_FOUND )
			return NULL ;

		ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;
	}

	// Allocates required memory
	szHeader = (PWSTR) ::malloc (cbBuff.Value ()) ;
	if ( szHeader == NULL )
		ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
	::wcscpy (szHeader, szName) ;

	if ( !::HttpQueryInfoW (
			hRequest
			, HTTP_QUERY_CUSTOM				// Get a custom header
			, static_cast<void *> (szHeader)
			, cbBuff.Ptr ()
			, pnIdx
			)
		) {
		SAFEFREE (szHeader) ;
		ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;
	}

	return szHeader ;
}

void CHttpToolW::InternetSetOption (HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength)
	throw (Exception &)
{
	if ( ::InternetSetOptionW (hInternet, dwOption, lpBuffer, dwBufferLength) )
		return ;

	ThrowException (HTTPCLIENT_ERR_INTERNETSETOPTION_FAILED, ::GetLastError ()) ;
}

// Generates a new upload boundary. If an error occurs, NULL is returned
// The returned string must be freed by using the ::free () function.
LPWSTR CHttpToolW::CreateUploadBoundary (void)
	throw ()
{
	GUID			guid ;

	if ( FAILED ( ::CoCreateGuid (&guid)) )
		return NULL ;

	PWSTR			szBoundary = NULL ;
	szBoundary = (PWSTR) ::malloc (sizeof (WCHAR) * 44) ;

	if ( szBoundary == NULL )
		return NULL ;

	::swprintf (szBoundary, L"----------071009092704718"/*L"----LYOUL-%.08x%.04x%.04x%.02x%.02x%.02x%.02x%.02x%.02x%.02x%.02x-"*/
		, guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1]
		, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return szBoundary ;
}
///////////////////////////////////////// CHttpToolW /////////////////////////////////////////


///////////////////////////////////////// CHttpClientMapT /////////////////////////////////////////
template <typename HttpTool>
CHttpClientMapT<HttpTool>::CHttpClientMapT (void)
	throw ()
{
	;	// do nothing yet
}

template <typename HttpTool>
CHttpClientMapT<HttpTool>::~CHttpClientMapT (void)
	throw ()
{
	Clear () ;
}

// If some data cleared, it will return TRUE, otherwise return FALSE.
// If memory is exhausted, it can leak memory.
template <typename HttpTool>
BOOL CHttpClientMapT<HttpTool>::Clear (void)
	throw ()
{
	if ( m_map.empty () )
		return FALSE ;

	// Allocates memory to save pointers for key name.
	// If an overflow exception or a memory allocation failure is occurs,
	// The memory pointed by the key name pointer are leaked.
	::SafeInt<MapSizeType>	cKeys = m_map.size () ;
	::SafeInt<size_t>		cbRequired = 0 ;
	PCSZ *					arrKeys = NULL ;

	try {
		cbRequired = cKeys ;
		cbRequired *= sizeof (PCSZ) ;
		arrKeys = (PCSZ *) ::malloc (cbRequired.Value ()) ;
	} catch (::SafeIntException &) {
		arrKeys = NULL ;
	}

	MapSizeType		nIdx = 0 ;
	for (MapIter iter = m_map.begin (); iter != m_map.end (); ++iter) {
		(iter->second).Delete () ;

		// Saves the key name pointer
		if ( arrKeys )
			arrKeys[nIdx++] = iter->first ;
	}

	m_map.clear () ;

	if ( arrKeys == NULL )
		return TRUE ;

	for (nIdx = 0; nIdx < cKeys; nIdx++)
		SAFEFREE ( arrKeys[nIdx] ) ;
	SAFEFREE ( arrKeys ) ;

	return TRUE ;
}

// This function deletes an element in position nIdx from the map.
// If the element deleted, it will return TRUE, otherwise return FALSE.
template <typename HttpTool>
BOOL CHttpClientMapT<HttpTool>::Remove (DWORD nIdx)
	throw ()
{
	if ( m_map.empty () )
		return FALSE ;

	// Find the item to delete
	MapIter	 iter = m_map.begin () ;
	for (DWORD i = 0; i < nIdx; i++) {
		++iter ;

		// If the nIdx is out of range.
		if ( iter == m_map.end () )
			return FALSE ;
	}

	(iter->second).Delete () ;
	PCSZ		szName = iter->first ;
	m_map.erase (iter) ;
	SAFEFREE (szName) ;
	return TRUE ;
}

// This function erases all elements of which key name equals to szName.
// If some data cleared, it will return TRUE, otherwise return FALSE.
// If memory is exhausted, memory will be leaked.
template <typename HttpTool>
BOOL CHttpClientMapT<HttpTool>::RemoveAll (PCSZ szName)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientMapT::RemoveAll: szName can not be NULL.") ;

	::SafeInt<MapSizeType>	cKeys = m_map.count (szName) ;

	// If the specified key name does not exist in the map
	if ( cKeys.Value () == 0 )
		return FALSE ;

	// Allocates memory to save pointers to key name.
	// If the allocation failed, the memory pointed by the key name pointer are leaked.
	::SafeInt<size_t>	cbRequired ;
	PCSZ *				arrKeys = NULL ;
	try {
		cbRequired = cKeys ;
		cbRequired *= sizeof (PCSZ) ;
		arrKeys = (PCSZ *) ::malloc (cbRequired.Value ()) ;
	} catch (::SafeIntException &) {
		arrKeys = NULL ;
	}

	std::pair<MapIter, MapIter>		pairIter = m_map.equal_range (szName) ;

	MapSizeType		nIdx = 0 ;
	for (MapIter iter = pairIter.first; iter != pairIter.second; ++iter) {
		(iter->second).Delete () ;

		// Saves the key name pointer
		if ( arrKeys )
			arrKeys[nIdx++] = iter->first ;
	}

	m_map.erase (pairIter.first, pairIter.second) ;

	if ( arrKeys == NULL )
		return TRUE ;

	for (nIdx = 0; nIdx < cKeys; nIdx++)
		SAFEFREE ( arrKeys[nIdx] ) ;
	SAFEFREE ( arrKeys ) ;

	return TRUE ;
}

// This function deletes an element of which the key name equals to szName and the index is nIdx.
// If some data cleared, it will return TRUE, otherwise return FALSE.
template <typename HttpTool>
BOOL CHttpClientMapT<HttpTool>::Remove (PCSZ szName, DWORD nIdx)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientMapT::Remove: szName can not be NULL.") ;

	std::pair<MapIter, MapIter>		pairIter = m_map.equal_range (szName) ;

	// If the specified key name does not exist in the map
	if ( pairIter.first == pairIter.second )
		return FALSE ;

	MapIter iter = pairIter.first ;
	for (DWORD i = 0; i < nIdx; i++) {
		++iter ;

		// If the nIdx is out of range.
		if ( iter == pairIter.second )
			return FALSE ;
	}

	// Deletes the element
	(iter->second).Delete () ;
	PCSZ	szKeyName = iter->first ;
	m_map.erase (iter) ;
	SAFEFREE (szKeyName) ;

	return TRUE ;
}

template <typename HttpTool>
BOOL CHttpClientMapT<HttpTool>::Exists (PCSZ szName, DWORD nIdx)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientMapT::Exists: szName can not be NULL.") ;

	::SafeInt<MapSizeType>		cKeys = m_map.count (szName) ;
	return (cKeys > nIdx) ;
}

// If the szName is NULL, it will return the count of elements.
template <typename HttpTool>
DWORD CHttpClientMapT<HttpTool>::Count (PCSZ szName)
	throw ()
{
	::SafeInt<DWORD>		cKeys ;
	try {
		cKeys = szName ? m_map.count (szName) : m_map.size () ;
	} catch (::SafeIntException &) {
		cKeys = cKeys.MaxInt () ;	// This statement is never executed.
	}
	return cKeys.Value () ;
}

template <typename HttpTool>
BOOL CHttpClientMapT<HttpTool>::Empty (void) const
	throw ()
{
	return m_map.empty () ? TRUE : FALSE ;
}

// If the nIdx is not valid, it will return NULL.
template <typename HttpTool>
typename CHttpClientMapT<HttpTool>::PCSZ
CHttpClientMapT<HttpTool>::GetKey (DWORD nIdx)
	throw ()
{
	if ( m_map.empty () )
		return NULL ;

	MapIter		iter = m_map.begin () ;
	for (DWORD i = 0; i < nIdx; i++) {
		++iter ;

		// If the nIdx is not valid.
		if ( iter == m_map.end () )
			return NULL ;
	}
	return iter->first ;
}

// If the specified element is not found in position nIdx,
// returns MapValue of which szValue is NULL.
template <typename HttpTool>
typename CHttpClientMapT<HttpTool>::MapValue
CHttpClientMapT<HttpTool>::Get (DWORD nIdx)
	throw ()
{
	MapValue	mapValue = { NULL, 0 } ;

	if ( m_map.empty () )
		return mapValue ;

	MapIter		iter = m_map.begin () ;
	for (DWORD i = 0; i < nIdx; i++) {
		++iter ;

		// If the nIdx is not valid.
		if ( iter == m_map.end () )
			return mapValue ;
	}

	mapValue = iter->second ;
	if ( mapValue.szValue == NULL )
		mapValue.szValue = HttpTool::szEmptyString ;

	return mapValue ;
}

// If the specified element is not found, it will return NULL.
template <typename HttpTool>
typename CHttpClientMapT<HttpTool>::PCSZ
CHttpClientMapT<HttpTool>::GetValue (DWORD nIdx)
	throw ()
{
	return Get (nIdx).szValue ;
}

// Returns 0 if the specified element is not found.
// (If the dwFlag in MapValue is 0, it also returns 0)
template <typename HttpTool>
DWORD CHttpClientMapT<HttpTool>::GetFlag (DWORD nIdx)
	throw ()
{
	return Get (nIdx).dwFlag ;
}

// If the specified element is not found, returns MapValue of which szValue is NULL.
template <typename HttpTool>
typename CHttpClientMapT<HttpTool>::MapValue
CHttpClientMapT<HttpTool>::Get (PCSZ szName, DWORD nIdx)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientMapT::Get: szName can not be NULL.") ;

	MapValue	mapValue = { NULL, 0 } ;
	std::pair<MapIter, MapIter>	pairIter ;
	pairIter = m_map.equal_range (szName) ;

	// If the key is not found.
	if ( pairIter.first == pairIter.second )
		return mapValue ;

	MapIter iter = pairIter.first ;
	for (DWORD i = 0; i < nIdx; i++) {
		++iter ;

		// If the nIdx is not valid
		if ( iter == pairIter.second )
			return mapValue ;
	}

	mapValue = iter->second ;
	if ( mapValue.szValue == NULL )
		mapValue.szValue = HttpTool::szEmptyString ;

	return mapValue ;
}

// Returns NULL if the specified element is not found.
template <typename HttpTool>
typename CHttpClientMapT<HttpTool>::PCSZ
CHttpClientMapT<HttpTool>::GetValue (PCSZ szName, DWORD nIdx)
	throw ()
{
	return Get (szName, nIdx).szValue ;
}

// Returns 0 if the specified element is not found.
// (If the dwFlag in MapValue is 0, it also returns 0)
template <typename HttpTool>
DWORD CHttpClientMapT<HttpTool>::GetFlag (PCSZ szName, DWORD nIdx)
	throw ()
{
	return Get (szName, nIdx).dwFlag ;
}

// Adds a new MapValue. It also receives the ownership of memory pointed by szName and szValue.
// The szName and szValue must be allocated by using ::malloc.
// The szValue could be NULL.
template <typename HttpTool>
void CHttpClientMapT<HttpTool>::AddPointerDirectly (PSZ szName, PSZ szValue, BOOL dwFlag)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientMapT::AddPointerDirectly: szName can not be NULL.") ;

	try {
		// Checks the arithmetic overflow exception
		::SafeInt<MapSizeType>	cKeys = Count () ;
		::SafeInt<DWORD>		cdwKeys = cKeys ;
		cKeys++ ;
		cdwKeys++ ;

		MapValue	newValue = { (PCSZ) szValue, dwFlag } ;
		m_map.insert (MapItem ((PCSZ) szName, newValue)) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	} catch (std::exception & e) {
		HttpTool::ThrowException (e.what (), HTTPCLIENT_ERR_STD_EXCEPTION) ;
	} catch (...) {
		HttpTool::ThrowException (HTTPCLIENT_ERR_UNEXPECTED_ERROR) ;
	}
}

// The szValue could be NULL.
template <typename HttpTool>
void CHttpClientMapT<HttpTool>::Add (PCSZ szName, PCSZ szValue, BOOL dwFlag)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientMapT::Add: szName can not be NULL.") ;

	PSZ					szNewName = NULL ;
	PSZ					szNewValue = NULL ;
	::SafeInt<size_t>	cbName, cbValue ;

	try {
		cbName = HttpTool::StringLen (szName) ;
		cbName++ ;
		cbName *= sizeof (CharType) ;

		szNewName = (PSZ) ::malloc (cbName.Value ()) ;
		if ( szNewName == NULL )
			HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
		HttpTool::StringCopy (szNewName, szName) ;

		if ( szValue != NULL ) {
			cbValue = HttpTool::StringLen (szValue) ;
			cbValue++ ;
			cbValue *= sizeof (CharType) ;

			szNewValue = (PSZ) ::malloc (cbValue.Value ()) ;
			if ( szNewValue == NULL )
				HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
			HttpTool::StringCopy (szNewValue, szValue) ;
		}
		AddPointerDirectly (szNewName, szNewValue, dwFlag) ;

	} catch (::SafeIntException & e) {
		SAFEFREE (szNewName) ;
		SAFEFREE (szNewValue) ;
		HttpTool::ThrowException (e) ;
	} catch (Exception &) {
		SAFEFREE (szNewName) ;
		SAFEFREE (szNewValue) ;
		throw ;
	}
}

// If the specified element is not found, it will add a new value.
// otherwise it will modifiy the existing value.
// The szValue could be NULL.
template <typename HttpTool>
void CHttpClientMapT<HttpTool>::Set (PCSZ szName, PCSZ szValue, BOOL dwFlag, DWORD nIdx)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientMapT::Set: szName can not be NULL.") ;

	std::pair<MapIter, MapIter>	pairIter ;
	pairIter = m_map.equal_range (szName) ;

	// Adds a new value if the specified element is not found.
	if ( pairIter.first == pairIter.second ) {
		// The nIdx must be 0
		_ASSERTE ( nIdx == 0 ) ;
		Add (szName, szValue, dwFlag) ;
		return ;
	}

	MapIter iter = pairIter.first ;
	for (DWORD i = 0; i < nIdx; i++) {
		++iter ;

		// The nIdx must be valid
		_ASSERTE ( iter != pairIter.second ) ;
	}

	MapValue		newValue = { NULL, dwFlag } ;
	if ( szValue ) {
		::SafeInt<size_t>		cbValue ;

		try {
			cbValue = HttpTool::StringLen (szValue) ;
			cbValue++ ;
			cbValue *= sizeof (CharType) ;
		} catch (::SafeIntException & e) {
			HttpTool::ThrowException (e) ;
		}

		newValue.szValue = (PCSZ) ::malloc (cbValue.Value ()) ;
		if ( newValue.szValue == NULL )
			HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

		HttpTool::StringCopy ((PSZ) newValue.szValue, szValue) ;
	}

	MapValue		oldValue = iter->second ;
	iter->second = newValue ;
	oldValue.Delete () ;
}

template <typename HttpTool>
typename CHttpClientMapT<HttpTool>::ConstMapIter
CHttpClientMapT<HttpTool>::Begin (void) const
	throw ()
{
	return m_map.begin () ;
}

template <typename HttpTool>
typename CHttpClientMapT<HttpTool>::ConstMapIter
CHttpClientMapT<HttpTool>::End (void) const
	throw ()
{
	return m_map.end () ;
}
///////////////////////////////////////// CHttpClientMapT /////////////////////////////////////////


///////////////////////////////////////// CHttpEncoder /////////////////////////////////////////
/*!
 * This method has no meaning because the input string is an Ansi string.
 * It just returns the length of the input string.
 *
 * \param szStr			[in] A string which is encoded.
 * \param CodePage		[in] Ignored.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderA::AnsiEncodeLen (PCSTR szStr, UINT /* CodePage */)
	throw (Exception &)
{
	if ( szStr == NULL || szStr[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	dwLen ;
	try {
		dwLen = ::strlen (szStr) ;
	} catch (::SafeIntException & e) {
		CHttpToolA::ThrowException (e) ;
	}
	return dwLen.Value () ;
}

/*!
 * This method has no meaning because the input string is an Ansi string.
 * It just returns the copy of the input string.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szStr			[in] A string which is encoded.
 * \param CodePage		[in] Ignored.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderA::AnsiEncode (PSTR szBuff, PCSTR szStr, UINT /* CodePage */)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szBuff != NULL, "CHttpEncoderA::AnsiEncode: szBuff can not be NULL.") ;

	if ( (szStr == NULL) || (szStr[0] == '\0') ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}
	return ::strcpy (szBuff, szStr) ;
}

/*!
 * This method has no meaning because the decoded string is an Ansi string.
 * It just returns the length of the input string.
 *
 * \param szEncoded		[in] A string to decode.
 * \param CodePage		[in] Ignored.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderA::AnsiDecodeLen (PCSTR szEncoded, UINT /* CodePage */)
	throw (Exception &)
{
	if ( szEncoded == NULL || szEncoded[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	dwLen ;
	try {
		dwLen = ::strlen (szEncoded) ;
	} catch (::SafeIntException & e) {
		CHttpToolA::ThrowException (e) ;
	}
	return dwLen.Value () ;
}

/*!
 * This method has no meaning because the decoded string is an Ansi string.
 * It just returns the copy of the input string.
 *
 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
 * \param szEncoded		[in] A string to decode.
 * \param CodePage		[in] Ignored.
 * \return				A decoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderA::AnsiDecode (PSTR szBuff, PCSTR szEncoded, UINT /* CodePage */)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szBuff != NULL, "CHttpEncoderA::AnsiDecode: szBuff can not be NULL.") ;

	if ( (szEncoded == NULL) || (szEncoded[0] == '\0') ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}
	return ::strcpy (szBuff, szEncoded) ;
}

// Checks whether the character is a valid hexadecimal.
template <typename CharType>
static BOOL _HexIsValid(CharType chHex)
{
	if ( chHex >= '0' && chHex <= '9' )
		return TRUE ;

	if ( chHex >= 'A' && chHex <= 'F' )
		return TRUE ;

	if ( chHex >= 'a' && chHex <= 'f' )
		return TRUE ;

	return FALSE ;
}

template <typename CharType>
static CharType _HexToNum(CharType chHex)
{
	if ( chHex >= '0' && chHex <= '9' )
		return chHex - '0' ;

	if ( chHex >= 'A' && chHex <= 'F' )
		return chHex - 'A' + 10 ;

	if ( chHex >= 'a' && chHex <= 'f' )
		return chHex - 'a' + 10 ;

	return 0 ;
}

template <typename CharType>
static BOOL _IsAlNum (CharType chChr)
{
	if (chChr >= 'A' && chChr <= 'Z')
		return TRUE ;
	
	if (chChr >= 'a' && chChr <= 'z')
		return TRUE ;
		
	if (chChr >= '0' && chChr <= '9')
		return TRUE ;

	return FALSE ;
}

/*!
 * \internal
 * This method returns the number of characters required to make a URL-encoded string.
 *
 * \param szEncoded		[in] An Ansi string or a UTF-8 string. The string can not be NULL.
 * \return				The number of characters required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
static DWORD _UrlEncodeLen (PCSTR szStr)
	throw (typename HttpTool::Exception &)
{
	HTTPCLIENT_ASSERT (szStr != NULL, "_UrlEncodeLen: szStr can not be NULL.") ;

	::SafeInt<DWORD>	cchEncoded = 0 ;
	PCSTR				pchStr = szStr ;
	try {
		while ( *pchStr ) {
			cchEncoded++ ;
			if ( !_IsAlNum (*pchStr) && *pchStr != ' ' )
				cchEncoded += 2 ;
			pchStr++ ;
		}
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}
	return cchEncoded.Value () ;
}

template <typename CharType>
static CharType _ToHex (CharType chChr)
{
	_ASSERTE ( chChr < 16 ) ;
	return chChr > 9 ? 'A' + (chChr - 10) : '0' + chChr ;
}

/*!
 * \internal
 * This method makes a URL-encoded string.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szEncoded		[in] An Ansi string or a UTF-8 string. The string can not be NULL.
 * \return				An URL-encoded string.
 */
template <typename StringType>
static StringType _UrlEncode (StringType szBuff, PCSTR szStr)
{
	_ASSERTE ( szBuff != NULL && szStr != NULL ) ;

	const CHAR *	pchStr = szStr ;
	StringType		pchOut = szBuff ;

	// do encoding
	for (; *pchStr != NULL; pchStr++) {
		if ( _IsAlNum (*pchStr) ) {
			*pchOut++ = *pchStr ;
			continue ;
		}

		if ( *pchStr == ' ' ) {
			*pchOut++ = '+' ;
			continue ;
		}

		*pchOut++ = '%';
		*pchOut++ = _ToHex (((*pchStr) & 0xF0) >> 4);
		*pchOut++ = _ToHex ((*pchStr) & 0x0F);
	}
	*pchOut = '\0' ;
	return szBuff ;
}

/*!
 * \internal
 * This method converts an Ansi character into an UTF-8 character.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szUtf8Char	[out] A buffer to save the converted UTF-8 character. The buffer can not be NULL.
 *						      The buffer must be allocated at least 7 characters.
 * \param szAnsiChar	[in] A string which contains an Ansi character. The string can not be NULL.
 * \param CodePage		[in] A code page of the Ansi character.
 * \throw				Throws a httpclientexception if an error occurs.
 */
void CHttpEncoderA::_AnsiCharToUtf8Char (PSTR szUtf8Char, PCSTR szAnsiChar, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szUtf8Char != NULL && szAnsiChar != NULL
		, "CHttpEncoderA::_AnsiCharToUtf8Char: szUtf8Char and szAnsiChar can not be NULL.") ;

	WCHAR		wszUnicode[2] ;

	// Get a unicode character
	if ( 0 == ::MultiByteToWideChar (CodePage, 0, szAnsiChar, -1, wszUnicode, 2) )
		CHttpToolA::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

	// Get a UTF-8 character sequence
	if ( 0 == ::WideCharToMultiByte (CP_UTF8, 0, wszUnicode, 2, szUtf8Char, 7, NULL, NULL) )
		CHttpToolA::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
}

/*!
 * \internal
 * This method converts an UTF-8 character into an Ansi character.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szAnsiChar	[out] A buffer to save the converted Ansi character. The buffer can not be NULL.
 *						      The buffer must be allocated at least 3 characters.
 * \param szUtf8Char	[in] A string which contains an UTF-8 character. The string can not be NULL.
 * \param CodePage		[in] A code page of the converted Ansi character.
 * \throw				Throws a httpclientexception if an error occurs.
 */
void CHttpEncoderA::_Utf8CharToAnsiChar (PSTR szAnsiChar, PCSTR szUtf8Char, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szAnsiChar != NULL && szUtf8Char != NULL
		, "CHttpEncoderA::_Utf8CharToAnsiChar: szAnsiChar and szUtf8Char can not be NULL.") ;

	WCHAR		wszUnicode[2] ;

	// Get a unicode character
	if ( 0 == ::MultiByteToWideChar (CP_UTF8, 0, szUtf8Char, -1, wszUnicode, 2) )
		CHttpToolA::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

	// Get a Ansi character sequence
	if ( 0 == ::WideCharToMultiByte (CodePage, 0, wszUnicode, 2, szAnsiChar, 3, NULL, NULL) )
		CHttpToolA::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
}

/*!
 * This method returns the number of bytes required to encode an Ansi string using UTF-8 encoding.
 * The returned value does not include the terminating NULL character.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szStr			[in] A string which is encoded.
 * \param CodePage		[in] A code page of the szStr parameter.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderA::Utf8EncodeLen (PCSTR szStr, UINT CodePage)
	throw (Exception &)
{
	if ( szStr == NULL || szStr[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchEncoded = 0 ;
	PCSTR				pchStr = szStr ;
	CHAR				szAnsi[3], szUtf8[7] ;
	PSTR				pchUtf8 ;

	try {
		while ( *pchStr ) {
			szAnsi[0] = *pchStr ;
			if ( ::IsDBCSLeadByteEx (CodePage, *pchStr) ) {
				szAnsi[1] = *(++pchStr) ;
				szAnsi[2] = '\0' ;
			} else
				szAnsi[1] = '\0' ;

			_AnsiCharToUtf8Char (szUtf8, szAnsi, CodePage) ;
			for (pchUtf8 = szUtf8; *pchUtf8 != '\0'; pchUtf8++, cchEncoded++) ;
			pchStr++ ;
		}
	} catch (::SafeIntException & e) {
		CHttpToolA::ThrowException (e) ;
	}
	return cchEncoded.Value () ;
}

/*!
 * This method encodes an Ansi string using UTF-8 encoding.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szStr			[in] A string which is encoded.
 * \param CodePage		[in] A code page of the szStr parameter.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderA::Utf8Encode (PSTR szBuff, PCSTR szStr, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szBuff != NULL, "CHttpEncoderA::Utf8Encode: szBuff can not be NULL.") ;

	if ( szStr == NULL || szStr[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	CHAR		szAnsi[3], szUtf8[7] ;
	PSTR		pchBuff = szBuff ;
	PCSTR		pchStr = szStr ;

	while ( *pchStr ) {
		szAnsi[0] = *pchStr ;
		if ( ::IsDBCSLeadByteEx (CodePage, *pchStr) ) {
			szAnsi[1] = *(++pchStr) ;
			szAnsi[2] = '\0' ;
		} else
			szAnsi[1] = '\0' ;

		_AnsiCharToUtf8Char (szUtf8, szAnsi, CodePage) ;
		::strcpy (pchBuff, szUtf8) ;
		for (; *pchBuff != NULL; pchBuff++) ;
		pchStr++ ;
	}

	return szBuff ;
}

/*!
 * \internal
 * This method saves an UTF-8 character into the buffer.
 * The pszSrc parameter is modified to point to the next UTF-8 character.
 *
 * \param szUtf8Char	[out] A buffer to save an UTF-8 character. The buffer can not be NULL.
 *						      The buffer must be allocated at least 7 characters.
 * \param pszSrc		[in] A pointer to a UTF-8 string. The pointer can not be NULL.
 * \return				Returns FALSE if the UTF-8 string contains an invalid byte.
 * \throw				Throws a httpclientexception if an error occurs.
 */
static BOOL _GetNextUtf8Char (PSTR szUtf8Char, PCSTR * pszSrc)
{
	_ASSERTE ( szUtf8Char != NULL ) ;
	_ASSERTE ( pszSrc != NULL && *pszSrc != NULL ) ;

	if ( '\0' == (szUtf8Char[0] = **pszSrc) )
		return TRUE ;

	(*pszSrc)++ ;

	// An ASCII character
	if ( szUtf8Char[0] >= 0 && szUtf8Char[0] <= 0x7F ) {
		szUtf8Char[1] = '\0' ;
		return TRUE ;
	}

	BYTE		cchUtf8, byChar ;

	byChar = static_cast<BYTE> (szUtf8Char[0]) ;

	// Following values are not allowed in UTF-8 encoding
	if ( byChar == 0xFE || byChar == 0xFF )
		return FALSE ;

	// It must be the first byte of the UTF-8 character sequence
	if ( !(byChar >= 0xC0 && byChar <= 0xFD) )
		return FALSE ;

	// Counts the number of bytes of the UTF-8 character sequence
	if ( (byChar & 0xFE) == 0xFC )					cchUtf8 = 6 ;
	else if ( (byChar & 0xFC) == 0xF8 )				cchUtf8 = 5 ;
	else if ( (byChar & 0xF8) == 0xF0 )				cchUtf8 = 4 ;
	else if ( (byChar & 0xF0) == 0xE0 )				cchUtf8 = 3 ;
	else if ( (byChar & 0xE0) == 0xC0 )				cchUtf8 = 2 ;
	else	return FALSE ;

	for (BYTE i = 1; i < cchUtf8; i++) {
		szUtf8Char[i] = *((*pszSrc)++) ;
		byChar = static_cast<BYTE> (szUtf8Char[i]) ;

		// It must be a valid byte of the UTF-8 multibyte sequence.
		if ( !(byChar >=0x80 && byChar <= 0xBF) )
			return FALSE ;
	}
	szUtf8Char[cchUtf8] = '\0' ;
	return TRUE ;
}

/*!
 * This method returns the number of bytes required to decode an UTF-8 string in ANSI.
 * The returned value does not include the terminating NULL character.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szEncoded		[in] A string to decode.
 * \param CodePage		[in] A code page of the returned Ansi string.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderA::Utf8DecodeLen (PCSTR szEncoded, UINT CodePage)
	throw (Exception &)
{
	if ( szEncoded == NULL || szEncoded[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchDecoded = 0 ;
	PCSTR				pchEncoded = szEncoded ;
	CHAR				szAnsi[3], szUtf8[7] ;
	PSTR				pchAnsi ;

	try {
		while ( *pchEncoded ) {
			if ( !_GetNextUtf8Char (szUtf8, &pchEncoded) )
				CHttpToolA::ThrowException (HTTPCLIENT_ERR_INVALID_UTF8_CHARACTER) ;

			_Utf8CharToAnsiChar (szAnsi, szUtf8, CodePage) ;
			for (pchAnsi = szAnsi; *pchAnsi != '\0'; pchAnsi++, cchDecoded++) ;
		}
	} catch (::SafeIntException & e) {
		CHttpToolA::ThrowException (e) ;
	}
	return cchDecoded.Value () ;
}

/*!
 * This method decodes an UTF-8 string using Ansi encoding.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
 * \param szEncoded		[in] A string to decode.
 * \param CodePage		[in] A code page of the returned Ansi string.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderA::Utf8Decode (PSTR szBuff, PCSTR szEncoded, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szBuff != NULL, "CHttpEncoderA::Utf8Decode: szBuff can not be NULL.") ;

	if ( szEncoded == NULL || szEncoded[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	PCSTR		pchEncoded = szEncoded ;
	CHAR		szUtf8[7] ;
	PSTR		pchBuff = szBuff ;

	while ( *pchEncoded ) {
		if ( !_GetNextUtf8Char (szUtf8, &pchEncoded) )
			CHttpToolA::ThrowException (HTTPCLIENT_ERR_INVALID_UTF8_CHARACTER) ;

		_Utf8CharToAnsiChar (pchBuff, szUtf8, CodePage) ;
		for (; *pchBuff != '\0'; pchBuff++) ;
	}

	return szBuff ;
}

/*!
 * This method returns the number of bytes required to make a URL-encoded string
 * (in Ansi character set) from an Ansi string.
 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
 * The returned value does not include the terminating NULL character.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szStr			[in] A string which is encoded.
 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
 *						     and that string is used to make a URL-encoded string.
 * \param CodePage		[in] A code page of the szStr parameter.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderA::UrlEncodeLenA (PCSTR szStr, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	if ( szStr == NULL || szStr[0] == '\0' )
		return 0 ;

	if ( !bUtf8Encoding )
		return _UrlEncodeLen<CHttpToolA> (szStr) ;

	::SafeInt<DWORD>	cchEncoded = 0 ;
	PCSTR				pchStr = szStr ;
	CHAR				szAnsi[3], szUtf8[7] ;

	try {
		while ( *pchStr ) {
			szAnsi[0] = *pchStr ;
			if ( ::IsDBCSLeadByteEx (CodePage, *pchStr) ) {
				szAnsi[1] = *(++pchStr) ;
				szAnsi[2] = '\0' ;
			} else
				szAnsi[1] = '\0' ;

			_AnsiCharToUtf8Char (szUtf8, szAnsi, CodePage) ;
			cchEncoded += _UrlEncodeLen<CHttpToolA> (szUtf8) ;
			pchStr++ ;
		}
	} catch (::SafeIntException & e) {
		CHttpToolA::ThrowException (e) ;
	}
	return cchEncoded.Value () ;
}

/*!
 * This method encodes a Ansi string to make a URL-encoded string (in Ansi character set).
 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szStr			[in] A string which is encoded.
 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
 *						     and that string is used to make a URL-encoded string.
 * \param CodePage		[in] A code page of the szStr parameter.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderA::UrlEncodeA (PSTR szBuff, PCSTR szStr, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szBuff != NULL, "CHttpEncoderA::UrlEncodeA: szBuff can not be NULL.") ;

	if ( szStr == NULL || szStr[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	if ( !bUtf8Encoding )
		return _UrlEncode (szBuff, szStr) ;

	CHAR		szAnsi[3], szUtf8[7] ;
	PSTR		pchBuff = szBuff ;
	PCSTR		pchStr = szStr ;

	while ( *pchStr ) {
		szAnsi[0] = *pchStr ;
		if ( ::IsDBCSLeadByteEx (CodePage, *pchStr) ) {
			szAnsi[1] = *(++pchStr) ;
			szAnsi[2] = '\0' ;
		} else
			szAnsi[1] = '\0' ;

		_AnsiCharToUtf8Char (szUtf8, szAnsi, CodePage) ;
		_UrlEncode (pchBuff, szUtf8) ;
		for (; *pchBuff != NULL; pchBuff++) ;
		pchStr++ ;
	}

	return szBuff ;
}

/*!
 * This method encodes a Ansi string to make a URL-encoded string (in Unicode character set).
 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szStr			[in] A string which is encoded.
 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
 *						     and that string is used to make a URL-encoded string.
 * \param CodePage		[in] A code page of the szStr parameter.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PWSTR CHttpEncoderA::UrlEncodeW (PWSTR szBuff, PCSTR szStr, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szBuff != NULL, "CHttpEncoderA::UrlEncodeW: szBuff can not be NULL.") ;

	if ( szStr == NULL || szStr[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	if ( !bUtf8Encoding )
		return _UrlEncode (szBuff, szStr) ;

	CHAR		szAnsi[3], szUtf8[7] ;
	PWSTR		pchBuff = szBuff ;
	PCSTR		pchStr = szStr ;

	while ( *pchStr ) {
		szAnsi[0] = *pchStr ;
		if ( ::IsDBCSLeadByteEx (CodePage, *pchStr) ) {
			szAnsi[1] = *(++pchStr) ;
			szAnsi[2] = '\0' ;
		} else
			szAnsi[1] = '\0' ;

		_AnsiCharToUtf8Char (szUtf8, szAnsi, CodePage) ;
		_UrlEncode (pchBuff, szUtf8) ;
		for (; *pchBuff != NULL; pchBuff++) ;
		pchStr++ ;
	}

	return szBuff ;
}

// [out] pchDecoded		A pointer to a character to save the next URL decoded character.
// [in] pszSrc			A pointer to a URL Encoded string.
// Returns TRUE if the operation success
template <typename StringType>
static BOOL _GetNextUrlDecodedChar (CHAR * pchDecoded, StringType * pszSrc)
{
	_ASSERTE ( pchDecoded != NULL && pszSrc != NULL && *pszSrc != NULL ) ;

	if ( **pszSrc == NULL ) {
		*pchDecoded = '\0' ;
		return TRUE ;
	}

	if ( **pszSrc == '+' ) {
		*pchDecoded = ' ' ;
		(*pszSrc)++ ;
		return TRUE ;
	}

	if ( **pszSrc != '%' ) {
		*pchDecoded = static_cast<CHAR> (*((*pszSrc)++)) ;
		return FALSE ;
	}

	(*pszSrc)++ ;

	if ( !_HexIsValid (**pszSrc) )
		return FALSE ;

	*pchDecoded = _HexToNum (**pszSrc) * 16 ;
	(*pszSrc)++ ;

	if ( !_HexIsValid (**pszSrc) )
		return FALSE ;

	*pchDecoded += _HexToNum (**pszSrc) ;
	(*pszSrc)++ ;

	return TRUE ;
}

// [out] szAnsiChar		a buffer to save the next URL decoded Ansi character sequence.
//						The buffer must be allocated at least 3 characters.
// [in] pszSrc			A pointer to a URL Encoded string.
// [in] CodePage		codepage
// Returns TRUE if the operation success
template <typename StringType>
static BOOL _GetNextUrlDecodedAnsiChar (PSTR szAnsiChar, StringType * pszSrc, UINT CodePage = CP_ACP)
{
	_ASSERTE ( szAnsiChar != NULL ) ;
	_ASSERTE ( pszSrc != NULL && *pszSrc != NULL ) ;

	if ( !_GetNextUrlDecodedChar (szAnsiChar, pszSrc) )
		return FALSE ;

	if ( szAnsiChar[0] == '\0' )
		return TRUE ;

	if ( !::IsDBCSLeadByteEx (CodePage, szAnsiChar[0]) ) {
		szAnsiChar[1] = '\0' ;
		return TRUE ;
	}

	if ( !_GetNextUrlDecodedChar (szAnsiChar + 1, pszSrc) )
		return FALSE ;

	// It can not be '\0'
	if ( szAnsiChar[1] == '\0' )
		return FALSE ;

	szAnsiChar[2] = '\0' ;
	return TRUE ;
}

// [out] szUtf8Char		a buffer to save a returned UTF-8 character sequence.
//						The buffer must be allocated at least 7 characters.
// [in] pszSrc			A pointer to a URL Encoded string.
// Returns TRUE if the operation success
template <typename StringType>
static BOOL _GetNextUrlDecodedUtf8Char (PSTR szUtf8Char, StringType * pszSrc)
{
	_ASSERTE ( szUtf8Char != NULL ) ;
	_ASSERTE ( pszSrc != NULL && *pszSrc != NULL ) ;

	if ( !_GetNextUrlDecodedChar (szUtf8Char, pszSrc) )
		return FALSE ;

	if ( szUtf8Char[0] == '\0' )
		return TRUE ;

	// An ASCII character
	if ( szUtf8Char[0] >= 0 && szUtf8Char[0] <= 0x7F ) {
		szUtf8Char[1] = '\0' ;
		return TRUE ;
	}

	BYTE		cchUtf8, byChar ;

	byChar = static_cast<BYTE> (szUtf8Char[0]) ;

	// following values are not allowed in UTF-8 encoding
	if ( byChar == 0xFE || byChar == 0xFF )
		return FALSE ;				// Invalid UTF-8 byte

	// It must be the first character of a UTF-8 character sequence
	if ( !(byChar >= 0xC0 && byChar <= 0xFD) )
		return FALSE ;

	// Counts the number of bytes of the UTF-8 character sequence
	if ( (byChar & 0xFE) == 0xFC )					cchUtf8 = 6 ;
	else if ( (byChar & 0xFC) == 0xF8 )				cchUtf8 = 5 ;
	else if ( (byChar & 0xF8) == 0xF0 )				cchUtf8 = 4 ;
	else if ( (byChar & 0xF0) == 0xE0 )				cchUtf8 = 3 ;
	else if ( (byChar & 0xE0) == 0xC0 )				cchUtf8 = 2 ;
	else	return FALSE ;

	for (BYTE i = 1; i < cchUtf8; i++) {
		if ( !_GetNextUrlDecodedChar (szUtf8Char + i, pszSrc) )
			return FALSE ;

		byChar = static_cast<BYTE> (szUtf8Char[i]) ;

		// It must be a byte of the UTF-8 multibyte sequence.
		if ( !(byChar >=0x80 && byChar <= 0xBF) )
			return FALSE ;
	}
	szUtf8Char[cchUtf8] = '\0' ;
	return TRUE ;
}

/*!
 * This method returns the number of bytes required to decode an URL-encoded string. (Ansi version)
 * The returned value does not include the terminating NULL character.
 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szEncoded		[in] A string to decode.
 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
 *						     So the decoded string is converted into an Ansi string.
 * \param CodePage		[in] A code page of the decoded string.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderA::UrlDecodeLenA (PCSTR szEncoded, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	if ( szEncoded == NULL || szEncoded[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchDecoded = 0 ;
	PCSTR				pchEncoded = szEncoded ;
	CHAR				szAnsi[3], szUtf8[7] ;
	CHAR *				pchAnsi ;

	try {
		do {
			if ( bUtf8Encoding ) {
				if ( !_GetNextUrlDecodedUtf8Char (szUtf8, &pchEncoded) )
					CHttpToolA::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

				// Converts into Ansi
				_Utf8CharToAnsiChar (szAnsi, szUtf8, CodePage) ;
			} else {
				if ( !_GetNextUrlDecodedAnsiChar (szAnsi, &pchEncoded, CodePage) )
					CHttpToolA::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;
			}

			for (pchAnsi = szAnsi; *pchAnsi != NULL; pchAnsi++, cchDecoded++) ;

		} while ( szAnsi[0] != '\0' ) ;
	} catch (::SafeIntException & e) {
		CHttpToolA::ThrowException (e) ;
	}
	return cchDecoded.Value () ;
}

/*!
 * This method returns the number of unicode characters required to decode an URL-encoded string.
 * The returned value does not include the terminating NULL character.
 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szEncoded		[in] A string to decode.
 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
 *						     So the decoded string is converted into an Unicode string.
 * \param CodePage		[in] A code page of the decoded string.
 * \return				The number of unicode characters required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderA::UrlDecodeLenW (PCSTR szEncoded, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	if ( szEncoded == NULL || szEncoded[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchDecoded = 0 ;
	PCSTR				pchEncoded = szEncoded ;

	try {
		if ( bUtf8Encoding ) {
			CHAR		szUtf8[7] ;

			while ( true ) {
				if ( !_GetNextUrlDecodedUtf8Char (szUtf8, &pchEncoded) )
					CHttpToolA::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

				if ( szUtf8[0] == '\0' )
					break ;

				cchDecoded++ ;
			}
			return cchDecoded.Value () ;
		}

		CHAR		szAnsi[3] ;
		while ( true ) {
			if ( !_GetNextUrlDecodedAnsiChar (szAnsi, &pchEncoded, CodePage) )
				CHttpToolA::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

			if ( szAnsi[0] == '\0' )
				break ;

			cchDecoded++ ;
		}
	} catch (::SafeIntException & e) {
		CHttpToolA::ThrowException (e) ;
	}
	return cchDecoded.Value () ;
}

/*!
 * This method decodes an URL-encoded string.
 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
 * \param szEncoded		[in] A string to decode.
 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
 *						     So the decoded string is converted into an Ansi string.
 * \param CodePage		[in] A code page of the decoded string.
 * \return				A decoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderA::UrlDecodeA (PSTR szBuff, PCSTR szEncoded, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szBuff != NULL, "CHttpEncoderA::UrlDecodeA: szBuff can not be NULL.") ;

	if ( szEncoded == NULL || szEncoded[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	CHAR		szAnsi[3], szUtf8[7] ;
	PCSTR		pchEncoded = szEncoded ;
	PSTR		pchBuff = szBuff ;
	PSTR		pchAnsi ;

	do {
		if ( bUtf8Encoding ) {
			if ( !_GetNextUrlDecodedUtf8Char (szUtf8, &pchEncoded) )
				CHttpToolA::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

			// Converts into a Ansi character
			_Utf8CharToAnsiChar (szAnsi, szUtf8, CodePage) ;
		} else {
			if ( !_GetNextUrlDecodedAnsiChar (szAnsi, &pchEncoded, CodePage) )
				CHttpToolA::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;
		}

		for (pchAnsi = szAnsi; *pchAnsi != '\0'; *(pchBuff++) = *pchAnsi, pchAnsi++) ;

	} while ( szAnsi[0] != '\0' ) ;

	*pchBuff = '\0' ;
	return szBuff ;
}

/*!
 * This method decodes an URL-encoded string.
 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
 * \param szEncoded		[in] A string to decode.
 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
 *						     So the decoded string is converted into an Unicode string.
 * \param CodePage		[in] A code page of the decoded string.
 * \return				A decoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PWSTR CHttpEncoderA::UrlDecodeW (PWSTR szBuff, PCSTR szEncoded, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTA (szBuff != NULL, "CHttpEncoderA::UrlDecodeW: szBuff can not be NULL.") ;

	if ( szEncoded == NULL || szEncoded[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	PCSTR		pchEncoded = szEncoded ;
	PWSTR		pchBuff = szBuff ;

	if ( bUtf8Encoding ) {
		CHAR		szUtf8[7] ;

		while ( true ) {
			if ( !_GetNextUrlDecodedUtf8Char (szUtf8, &pchEncoded) )
				CHttpToolA::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

			if ( szUtf8[0] == '\0' )
				break ;

			// Converts into a unicode character
			if ( 0 == ::MultiByteToWideChar (CP_UTF8, 0, szUtf8, -1, pchBuff, 2) )
				CHttpToolA::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

			pchBuff++ ;
		}
	} else {
		CHAR		szAnsi[3] ;

		while ( true ) {
			if ( !_GetNextUrlDecodedAnsiChar (szAnsi, &pchEncoded, CodePage) )
				CHttpToolA::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

			if ( szAnsi[0] == '\0' )
				break ;

			// Converts into a unicode character
			if ( 0 == ::MultiByteToWideChar (CodePage, 0, szAnsi, -1, pchBuff, 2) )
				CHttpToolA::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

			pchBuff++ ;
		}
	}

	*pchBuff = '\0' ;
	return szBuff ;
}


/*!
 * This method returns the number of bytes required to convert an Unicode string into an Ansi string.
 * The returned value does not include the terminating NULL character.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szStr			[in] A string which is encoded.
 * \param CodePage		[in] A code page of the encoded string.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderW::AnsiEncodeLen (PCWSTR szStr, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPCLIENT_ASSERTW ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpEncoderW::AnsiEncodeLen: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	if ( szStr == NULL || szStr[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchAnsiLen ;
	try {
		if ( 0 == (cchAnsiLen = ::WideCharToMultiByte (CodePage, 0, szStr, -1, NULL, 0, NULL, NULL)) )
			CHttpToolW::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
		cchAnsiLen-- ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}
	return cchAnsiLen.Value () ;
}

/*!
 * This method converts an Unicode string into an Ansi string.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szStr			[in] A string which is encoded.
 * \param CodePage		[in] A code page of the encoded string.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderW::AnsiEncode (PSTR szBuff, PCWSTR szStr, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPCLIENT_ASSERTW ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpEncoderW::AnsiEncode: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;
	HTTPCLIENT_ASSERTW (szBuff != NULL, "CHttpEncoderW::AnsiEncode: szBuff can not be NULL.") ;

	if ( (szStr == NULL) || (szStr[0] == '\0') ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	::SafeInt<int>		cbAnsiLen ;
	try {
		cbAnsiLen = AnsiEncodeLen (szStr, CodePage) ;
		cbAnsiLen++ ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}

	if ( 0 == ::WideCharToMultiByte (CodePage, 0, szStr, -1, szBuff, cbAnsiLen.Value (), NULL, NULL) )
		CHttpToolW::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;

	return szBuff ;
}

/*!
 * This method returns the number of unicode characters required to convert an Ansi string into an Unicode string.
 * The returned value does not include the terminating NULL character.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szEncoded		[in] A string to decode.
 * \param CodePage		[in] A code page of the szEncoded parameter.
 * \return				The number of unicode characters required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderW::AnsiDecodeLen (PCSTR szEncoded, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPCLIENT_ASSERTW ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpEncoderW::AnsiDecodeLen: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	if ( szEncoded == NULL || szEncoded[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchUnicodeLen ;
	try {
		if ( 0 == (cchUnicodeLen = ::MultiByteToWideChar (CodePage, 0, szEncoded, -1, NULL, 0)) )
			CHttpToolW::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;
		cchUnicodeLen-- ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}
	return cchUnicodeLen.Value () ;
}

/*!
 * This method converts an Ansi string into an Unicode string.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
 * \param szEncoded		[in] A string to decode.
 * \param CodePage		[in] A code page of the szEncoded parameter.
 * \return				A decoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PWSTR CHttpEncoderW::AnsiDecode (PWSTR szBuff, PCSTR szEncoded, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPCLIENT_ASSERTW ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpEncoderW::AnsiDecode: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;
	HTTPCLIENT_ASSERTW (szBuff != NULL, "CHttpEncoderW::AnsiDecode: szBuff can not be NULL.") ;

	if ( (szEncoded == NULL) || (szEncoded[0] == '\0') ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	::SafeInt<int>		cchUnicode ;
	try {
		cchUnicode = AnsiDecodeLen (szEncoded, CodePage) ;
		cchUnicode++ ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}
	if ( 0 == ::MultiByteToWideChar (CodePage, 0, szEncoded, -1, szBuff, cchUnicode.Value ()) )
		CHttpToolW::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

	return szBuff ;
}

/*!
 * This method returns the number of characters required to encode an Unicode string using UTF-8 encoding.
 * The returned value does not include the terminating NULL character.
 *
 * \param szStr			[in] A string which is encoded.
 * \param CodePage		[in] Ignored.
 * \return				The number of characters required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderW::Utf8EncodeLen (PCWSTR szStr, UINT /* CodePage */)
	throw (Exception &)
{
	if ( szStr == NULL || szStr[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchRequired ;
	try  {
		if ( 0 == (cchRequired = ::WideCharToMultiByte (CP_UTF8, 0, szStr, -1, NULL, 0, NULL, NULL)) )
			CHttpToolW::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
		cchRequired-- ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}
	return cchRequired.Value () ;
}

/*!
 * This method encodes an Unicode string using UTF-8 encoding.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szStr			[in] A string which is encoded.
 * \param CodePage		[in] Ignored.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderW::Utf8Encode (PSTR szBuff, PCWSTR szStr, UINT /* CodePage */)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTW (szBuff != NULL, "CHttpEncoderW::Utf8Encode: szBuff can not be NULL.") ;

	if ( szStr == NULL || szStr[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	::SafeInt<int>		cchRequired ;
	try  {
		cchRequired = Utf8EncodeLen (szStr) ;
		cchRequired++ ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}
	if ( 0 == ::WideCharToMultiByte (CP_UTF8, 0, szStr, -1, szBuff, cchRequired.Value (), NULL, NULL) )
		CHttpToolW::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;

	return szBuff ;
}

/*!
 * This method returns the number of unicode characters required to decode an UTF-8 string in unicode.
 * The returned value does not include the terminating NULL character.
 *
 * \param szEncoded		[in] A string to decode.
 * \param CodePage		[in] Ignored.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderW::Utf8DecodeLen (PCSTR szEncoded, UINT /* CodePage */)
	throw (Exception &)
{
	if ( szEncoded == NULL || szEncoded[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchRequired ;
	try {
		if ( 0 == (cchRequired = ::MultiByteToWideChar (CP_UTF8, 0, szEncoded, -1, NULL, 0)) )
			CHttpToolW::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;	
		cchRequired-- ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}
	return cchRequired.Value () ;
}

/*!
 * This method decodes an UTF-8 string in unicode.
 *
 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
 * \param szEncoded		[in] A string to decode.
 * \param CodePage		[in] Ignored.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PWSTR CHttpEncoderW::Utf8Decode (PWSTR szBuff, PCSTR szEncoded, UINT /* CodePage */)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTW (szBuff != NULL, "CHttpEncoderW::Utf8Decode: szBuff can not be NULL.") ;

	if ( szEncoded == NULL || szEncoded[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	::SafeInt<int>		cchRequired ;
	try {
		cchRequired = Utf8DecodeLen (szEncoded) ;
		cchRequired++ ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}

	if ( 0 == ::MultiByteToWideChar (CP_UTF8, 0, szEncoded, -1, szBuff, cchRequired.Value ()) )
		CHttpToolW::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;	
	return szBuff ;
}

/*!
 * This method returns the number of bytes required to make a URL-encoded string
 * (in Ansi character set) from an Unicode string.
 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
 * The returned value does not include the terminating NULL character.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szStr			[in] A string which is encoded.
 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
 *						     and that string is used to make a URL-encoded string.
 * \param CodePage		[in] A code page of the encoded string.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderW::UrlEncodeLenA (PCWSTR szStr, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	if ( szStr == NULL || szStr[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchEncoded = 0 ;
	PCWSTR				pchStr = szStr ;
	WCHAR				szUnicode[2] = {'\0', '\0'} ;
	CHAR				szAnsiOrUtf8[7] ;
	UINT				nActualCodePage = bUtf8Encoding ? CP_UTF8 : CodePage ;

	try {
		while ( *pchStr ) {
			szUnicode[0] = *(pchStr++) ;

			if ( 0 == ::WideCharToMultiByte (nActualCodePage, 0, szUnicode, 2, szAnsiOrUtf8, 7, NULL, NULL) )
				CHttpToolW::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
			cchEncoded += _UrlEncodeLen<CHttpToolW> (szAnsiOrUtf8) ;
		}
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}
	return cchEncoded.Value () ;
}

/*!
 * This method encodes a Unicode string to make a URL-encoded string (in Ansi character set).
 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szStr			[in] A string which is encoded.
 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
 *						     and that string is used to make a URL-encoded string.
 * \param CodePage		[in] A code page of the encoded string.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderW::UrlEncodeA (PSTR szBuff, PCWSTR szStr, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTW (szBuff != NULL, "CHttpEncoderW::UrlEncodeA: szBuff can not be NULL.") ;

	if ( szStr == NULL || szStr[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	PCWSTR		pchStr = szStr ;
	PSTR		pchBuff = szBuff ;
	WCHAR		szUnicode[2] = {'\0', '\0'} ;
	CHAR		szAnsiOrUtf8[7] ;
	UINT		nActualCodePage = bUtf8Encoding ? CP_UTF8 : CodePage ;

	while ( *pchStr ) {
		szUnicode[0] = *(pchStr++) ;

		if ( 0 == ::WideCharToMultiByte (nActualCodePage, 0, szUnicode, 2, szAnsiOrUtf8, 7, NULL, NULL) )
			CHttpToolW::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
		_UrlEncode (pchBuff, szAnsiOrUtf8) ;
		for (; *pchBuff != NULL; pchBuff++) ;
	}

	return szBuff ;
}

/*!
 * This method encodes a Unicode string to make a URL-encoded string (in Unicode character set).
 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
 * \param szStr			[in] A string which is encoded.
 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
 *						     and that string is used to make a URL-encoded string.
 * \param CodePage		[in] A code page of the encoded string.
 * \return				An encoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PWSTR CHttpEncoderW::UrlEncodeW (PWSTR szBuff, PCWSTR szStr, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTW (szBuff != NULL, "CHttpEncoderW::UrlEncodeW: szBuff can not be NULL.") ;

	if ( szStr == NULL || szStr[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	PCWSTR		pchStr = szStr ;
	PWSTR		pchBuff = szBuff ;
	WCHAR		szUnicode[2] = {'\0', '\0'} ;
	CHAR		szAnsiOrUtf8[7] ;
	UINT		nActualCodePage = bUtf8Encoding ? CP_UTF8 : CodePage ;

	while ( *pchStr ) {
		szUnicode[0] = *(pchStr++) ;

		if ( 0 == ::WideCharToMultiByte (nActualCodePage, 0, szUnicode, 2, szAnsiOrUtf8, 7, NULL, NULL) )
			CHttpToolW::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
		_UrlEncode (pchBuff, szAnsiOrUtf8) ;
		for (; *pchBuff != NULL; pchBuff++) ;
	}

	return szBuff ;
}

// [out] szAnsiChar		a buffer to save a returned Ansi character sequence.
//						The buffer must be allocated at least 3 characters.
// [in] szUtf8Char		An Utf8 character
void CHttpEncoderW::_Utf8CharToAnsiChar (PSTR szAnsiChar, PCSTR szUtf8Char, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTW (szAnsiChar != NULL && szUtf8Char != NULL
		, "CHttpEncoderW::_Utf8CharToAnsiChar: szAnsiChar and szUtf8Char can not be NULL.") ;

	WCHAR		wszUnicode[2] ;

	// Get a unicode character
	if ( 0 == ::MultiByteToWideChar (CP_UTF8, 0, szUtf8Char, -1, wszUnicode, 2) )
		CHttpToolW::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

	// Get a Ansi character sequence
	if ( 0 == ::WideCharToMultiByte (CodePage, 0, wszUnicode, 2, szAnsiChar, 3, NULL, NULL) )
		CHttpToolW::ThrowException (HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED, ::GetLastError ()) ;
}

/*!
 * This method returns the number of bytes required to decode an URL-encoded string. (Ansi version)
 * The returned value does not include the terminating NULL character.
 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szEncoded		[in] A string to decode.
 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
 *						     So the decoded string is converted into an Ansi string.
 * \param CodePage		[in] A code page of the decoded string.
 * \return				The number of bytes required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderW::UrlDecodeLenA (PCWSTR szEncoded, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	if ( szEncoded == NULL || szEncoded[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchDecoded = 0 ;
	PCWSTR				pchEncoded = szEncoded ;
	CHAR				szAnsi[3], szUtf8[7] ;
	CHAR *				pchAnsi ;

	try {
		do {
			if ( bUtf8Encoding ) {
				if ( !_GetNextUrlDecodedUtf8Char (szUtf8, &pchEncoded) )
					CHttpToolW::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

				// Converts into Ansi
				_Utf8CharToAnsiChar (szAnsi, szUtf8, CodePage) ;
			} else {
				if ( !_GetNextUrlDecodedAnsiChar (szAnsi, &pchEncoded, CodePage) )
					CHttpToolW::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;
			}

			for (pchAnsi = szAnsi; *pchAnsi != NULL; pchAnsi++, cchDecoded++) ;

		} while ( szAnsi[0] != '\0' ) ;
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}

	return cchDecoded.Value () ;
}

/*!
 * This method returns the number of unicode characters required to decode an URL-encoded string.
 * The returned value does not include the terminating NULL character.
 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szEncoded		[in] A string to decode.
 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
 *						     So the decoded string is converted into an Unicode string.
 * \param CodePage		[in] A code page of the decoded string.
 * \return				The number of unicode characters required. (Not including the terminating NULL character)
 * \throw				Throws a httpclientexception if an error occurs.
 */
DWORD CHttpEncoderW::UrlDecodeLenW (PCWSTR szEncoded, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	if ( szEncoded == NULL || szEncoded[0] == '\0' )
		return 0 ;

	::SafeInt<DWORD>	cchDecoded = 0 ;
	PCWSTR				pchEncoded = szEncoded ;

	try {
		if ( bUtf8Encoding ) {
			CHAR		szUtf8[7] ;

			while ( true ) {
				if ( !_GetNextUrlDecodedUtf8Char (szUtf8, &pchEncoded) )
					CHttpToolW::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

				if ( szUtf8[0] == '\0' )
					break ;

				cchDecoded++ ;
			}
			return cchDecoded.Value () ;
		}

		CHAR		szAnsi[3] ;
		while ( true ) {
			if ( !_GetNextUrlDecodedAnsiChar (szAnsi, &pchEncoded, CodePage) )
				CHttpToolW::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

			if ( szAnsi[0] == '\0' )
				break ;

			cchDecoded++ ;
		}
	} catch (::SafeIntException & e) {
		CHttpToolW::ThrowException (e) ;
	}

	return cchDecoded.Value () ;
}

/*!
 * This method decodes an URL-encoded string.
 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
 * \param szEncoded		[in] A string to decode.
 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
 *						     So the decoded string is converted into an Ansi string.
 * \param CodePage		[in] A code page of the decoded string.
 * \return				A decoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PSTR CHttpEncoderW::UrlDecodeA (PSTR szBuff, PCWSTR szEncoded, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTW (szBuff != NULL, "CHttpEncoderW::UrlDecodeA: szBuff can not be NULL.") ;

	if ( szEncoded == NULL || szEncoded[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	CHAR		szAnsi[3], szUtf8[7] ;
	PCWSTR		pchEncoded = szEncoded ;
	PSTR		pchBuff = szBuff ;
	PSTR		pchAnsi ;

	do {
		if ( bUtf8Encoding ) {
			if ( !_GetNextUrlDecodedUtf8Char (szUtf8, &pchEncoded) )
				CHttpToolW::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

			// Converts into a Ansi character
			_Utf8CharToAnsiChar (szAnsi, szUtf8, CodePage) ;
		} else {
			if ( !_GetNextUrlDecodedAnsiChar (szAnsi, &pchEncoded, CodePage) )
				CHttpToolW::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;
		}

		for (pchAnsi = szAnsi; *pchAnsi != '\0'; *(pchBuff++) = *pchAnsi, pchAnsi++) ;

	} while ( szAnsi[0] != '\0' ) ;

	*pchBuff = '\0' ;
	return szBuff ;
}

/*!
 * This method decodes an URL-encoded string.
 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
 * The detailed infomation about the Code-Page Identifiers is described in the MSDN documentation.
 *
 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
 * \param szEncoded		[in] A string to decode.
 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
 *						     So the decoded string is converted into an Unicode string.
 * \param CodePage		[in] A code page of the decoded string.
 * \return				A decoded string.
 * \throw				Throws a httpclientexception if an error occurs.
 */
PWSTR CHttpEncoderW::UrlDecodeW (PWSTR szBuff, PCWSTR szEncoded, BOOL bUtf8Encoding, UINT CodePage)
	throw (Exception &)
{
	HTTPCLIENT_ASSERTW (szBuff != NULL, "CHttpEncoderW::UrlDecodeW: szBuff can not be NULL.") ;

	if ( szEncoded == NULL || szEncoded[0] == '\0' ) {
		szBuff[0] = '\0' ;
		return szBuff ;
	}

	PCWSTR		pchEncoded = szEncoded ;
	PWSTR		pchBuff = szBuff ;

	if ( bUtf8Encoding ) {
		CHAR		szUtf8[7] ;

		while ( true ) {
			if ( !_GetNextUrlDecodedUtf8Char (szUtf8, &pchEncoded) )
				CHttpToolW::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

			if ( szUtf8[0] == '\0' )
				break ;

			// Converts into a unicode character
			if ( 0 == ::MultiByteToWideChar (CP_UTF8, 0, szUtf8, -1, pchBuff, 2) )
				CHttpToolW::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

			pchBuff++ ;
		}
	} else {
		CHAR		szAnsi[3] ;

		while ( true ) {
			if ( !_GetNextUrlDecodedAnsiChar (szAnsi, &pchEncoded, CodePage) )
				CHttpToolW::ThrowException (HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID) ;

			if ( szAnsi[0] == '\0' )
				break ;

			// Converts into a unicode character
			if ( 0 == ::MultiByteToWideChar (CodePage, 0, szAnsi, -1, pchBuff, 2) )
				CHttpToolW::ThrowException (HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED, ::GetLastError ()) ;

			pchBuff++ ;
		}
	}

	*pchBuff = '\0' ;
	return szBuff ;
}
///////////////////////////////////////// CHttpEncoder /////////////////////////////////////////


///////////////////////////////////////// CHttpResponseT /////////////////////////////////////////
/*!
 * This is the constructor which has three parameters which are the WinInet internet handles.
 * The handles are owned by the CHttpResponseT and the lifetime of the handles are managed by the CHttpResponseT.
 * If you pass NULL for hInternet or hConnection parameter, user has to manage the lifetime of the handles
 * which are not passed to the CHttpResponseT until the CHttpResponseT is destoried.
 *
 * \param hInternet		[in] An internet handle returned by ::InternetOpen.
 * \param hConnection	[in] A connection handle returned by ::InternetConnect.
 * \param hRequest		[in] A request handle returned by ::HttpSendRequest or ::HttpEndRequest function.
 *						     The hRequest parameter can not be NULL.
 */
template <typename HttpTool>
CHttpResponseT<HttpTool>::CHttpResponseT (HINTERNET hInternet, HINTERNET hConnection, HINTERNET hRequest)
	throw ()
{
	_Initialize (hInternet, hConnection, hRequest) ;
}

/*!
 * This is the constructor which has two parameters which are the WinInet internet handles.
 * The handles are owned by the CHttpResponseT and the lifetime of the handles are managed by the CHttpResponseT.
 * If you pass NULL for hConnection parameter, user has to manage the lifetime of the hConnection handle
 * which is not passed to the CHttpResponseT until the CHttpResponseT is destoried.
 * The user also has to manage the lifetime of the internet handle returned by ::InternetOpen
 * until the CHttpResponseT is destoried.
 *
 * \param hConnection	[in] A connection handle returned by ::InternetConnect.
 * \param hRequest		[in] A request handle returned by ::HttpSendRequest or ::HttpEndRequest function.
 *						     The hRequest parameter can not be NULL.
 */
template <typename HttpTool>
CHttpResponseT<HttpTool>::CHttpResponseT (HINTERNET hConnection, HINTERNET hRequest)
	throw ()
{
	_Initialize (NULL, hConnection, hRequest) ;
}

/*!
 * This is the constructor which has a parameter which is the WinInet internet handle.
 * The handle is owned by the CHttpResponseT and the lifetime of the handle is managed by the CHttpResponseT.
 * user has to manage the lifetime of the internet handles returned by ::InternetOpen and ::InternetConnect
 * until the CHttpResponseT is destoried.
 *
 * \param hRequest		[in] A request handle returned by ::HttpSendRequest or ::HttpEndRequest function.
 *						     The hRequest parameter can not be NULL.
 */
template <typename HttpTool>
CHttpResponseT<HttpTool>::CHttpResponseT (HINTERNET hRequest)
	throw ()
{
	_Initialize (NULL, NULL, hRequest) ;
}

/*!
 * This is a default destructor. It closes all internet handles saved in this object.
 */
template <typename HttpTool>
CHttpResponseT<HttpTool>::~CHttpResponseT (void)
	throw ()
{
	HttpTool::CloseRequest (m_hRequest) ;
	HttpTool::CloseConnection (m_hConnection) ;
	HttpTool::CloseInternet (m_hInternet) ;
	SAFEFREE (m_szStatusText) ;
}

/*!
 * This method initializes the internal member variables.
 *
 * \param hInternet		[in] An internet handle returned by ::InternetOpen.
 * \param hConnection	[in] A connection handle returned by ::InternetConnect.
 * \param hRequest		[in] A request handle returned by ::HttpSendRequest or ::HttpEndRequest function.
 *						     The hRequest parameter can not be NULL.
 */
template <typename HttpTool>
void CHttpResponseT<HttpTool>::_Initialize (HINTERNET hInternet, HINTERNET hConnection, HINTERNET hRequest)
	throw ()
{
	_ASSERTE ( hRequest != NULL ) ;

	m_hInternet = hInternet ;
	m_hConnection = hConnection ;
	m_hRequest = hRequest ;

	m_dwStatus = 0 ;
	m_szStatusText = NULL ;
	m_cbContLen = 0 ;
}

/*!
 * \internal
 * This method caches all HTTP headers of which name is the szName.
 *
 * \param szName		[in] The case-insensitive name of the header. NULL is not allowed.
 * \return				The number of cached headers.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
DWORD CHttpResponseT<HttpTool>::_LoadHeader (PCSZ szName)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpResponseT::_LoadHeader: m_hRequest can not be NULL.") ;
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpResponseT::_LoadHeader: szName can not be NULL.") ;

	PSZ					szHeader = NULL ;
	PSZ					szCopiedName = NULL ;
	::SafeInt<DWORD>	cHeader = 0 ;

	try {
		try {
			DWORD				nIdx = 0 ;
			::SafeInt<size_t>	cbName = 1 ;
			cbName += HttpTool::StringLen (szName) ;
			cbName *= sizeof (CharType) ;

			while ( true ) {
				if ( NULL == (szHeader = HttpTool::GetHeader (m_hRequest, szName, &nIdx)) )
					break ;

				cHeader++ ;

				// Copy the header name
				if ( NULL == (szCopiedName = (PSZ) ::malloc (cbName.Value ())) )
					HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

				HttpTool::StringCopy (szCopiedName, szName) ;
				m_mapHeader.AddPointerDirectly (szCopiedName, szHeader) ;
				szCopiedName = szHeader = NULL ;
			}
		} catch (::SafeIntException & e) {
			HttpTool::ThrowException (e) ;
		}
	} catch (Exception &) {
		m_mapHeader.RemoveAll (szName) ;	// All or nothing
		SAFEFREE (szHeader) ;
		SAFEFREE (szCopiedName) ;
		throw ;
	}

	return cHeader.Value () ;
}

/*!
 * This method returns the number of headers of which name is szName.
 *
 * \param szName		[in] A case-insensitive header name. NULL is not allowed.
 * \return				The number of headers of which name is bstrName.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
DWORD CHttpResponseT<HttpTool>::GetHeaderCount (PCSZ szName)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpResponseT::GetHeaderCount: szName can not be NULL.") ;

	DWORD		cHeader = m_mapHeader.Count (szName) ;

	// Tries to load headers if the header does not exist
	if ( cHeader == 0 )
		cHeader = _LoadHeader (szName) ;

	return cHeader ;
}

/*!
 * This method returns the header of which name is the szName.
 * If a header name has multiple values, you can specify a zero-based index for a specific value.
 * If a header specified by szName is not found or the index is out of range, it will return NULL.
 * Otherwise it always returns a null-terminated string. The returned string is owned by
 * the CHttpResponseT. So you can not free it.
 *
 * \param szName		[in] A case-insensitive header name. NULL is not allowed.
 * \param nIdx			[in] A zero-based index for a header which has multiple values.
 * \return				The requested header.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
typename CHttpResponseT<HttpTool>::PCSZ
CHttpResponseT<HttpTool>::GetHeader (PCSZ szName, DWORD nIdx = 0)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpResponseT::GetHeader: szName can not be NULL.") ;

	// Tries to load headers if the header does not exist
	if ( !m_mapHeader.Count (szName) && !_LoadHeader (szName) )
		return NULL ;

	return m_mapHeader.GetValue (szName, nIdx) ;
}

/*!
 * This method returns the HTTP status code of a HTTP response.
 *
 * \return				The HTTP status code.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
DWORD CHttpResponseT<HttpTool>::GetStatus (void)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpResponseT::GetStatus: m_hRequest can not be NULL.") ;

	// If the cached one exists
	if ( m_dwStatus != 0 )
		return m_dwStatus ;

	DWORD		dwBuffSize = sizeof (DWORD) ;

	if ( !::HttpQueryInfo (
			m_hRequest
			, HTTP_QUERY_STATUS_CODE		// Get the HTTP status code
				| HTTP_QUERY_FLAG_NUMBER
			, static_cast<void *> (&m_dwStatus)
			, &dwBuffSize					// Buffer size (byte)
			, NULL							// Does not use a header index
			)
		)
		HttpTool::ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;

	return m_dwStatus ;
}

/*!
 * This method returns the HTTP status text of a HTTP response.
 * It always returns a null-terminated string. The returned string is owned by
 * the CHttpResponse. So you can not free it.
 *
 * \return				The HTTP status text returned by a HTTP server.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
typename CHttpResponseT<HttpTool>::PCSZ
CHttpResponseT<HttpTool>::GetStatusText (void)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpResponseT::GetStatusText: m_hRequest can not be NULL.") ;

	// If the cached one exists
	if ( m_szStatusText != NULL )
		return m_szStatusText ;

	return (m_szStatusText = HttpTool::GetStatusText (m_hRequest)) ;
}

/*!
 * This method retrieves the content length of a returned HTTP response.
 * The content is the data stream except the HTTP headers. If the length is retrieved,
 * it will return TRUE, otherwise return FALSE.
 *
 * \param cbContLen		[out] A reference variable to save the content length.
 * \return				TRUE if the content length is available, otherwise FALSE.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
BOOL CHttpResponseT<HttpTool>::GetContentLength (DWORD & cbContLen)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpResponseT::GetContentLength: m_hRequest can not be NULL.") ;

	DWORD		dwBuffSize = sizeof (DWORD) ;

	if ( !::HttpQueryInfo (
			m_hRequest
			, HTTP_QUERY_CONTENT_LENGTH		// Reads the 'content-length' header
				| HTTP_QUERY_FLAG_NUMBER
			, static_cast<void *> (&cbContLen)
			, &dwBuffSize					// Buffer size (byte)
			, NULL							// Does not use a header index
			)
		) {

		if ( ::GetLastError () == ERROR_HTTP_HEADER_NOT_FOUND )
			return FALSE ;

		HttpTool::ThrowException (HTTPCLIENT_ERR_QUERYINFO_FAILED, ::GetLastError ()) ;
	}

	return TRUE ;
}

/*!
 * This method reads the content (the data stream except headers) of a returned HTTP response.
 *
 * \param pbyBuff		[out] A buffer to save the read content.
 * \param cbBuff		[in] A buffer size in byte.
 * \return				Returns the number of bytes read. Returns zero if all content is read.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
DWORD CHttpResponseT<HttpTool>::ReadContent (BYTE * pbyBuff, DWORD cbBuff)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpResponseT::ReadContent: m_hRequest can not be NULL.") ;
	HTTPCLIENT_ASSERT (pbyBuff != NULL, "CHttpResponseT::ReadContent: pbyBuff can not be NULL.") ;
	HTTPCLIENT_ASSERT (cbBuff != 0, "CHttpResponseT::ReadContent: cbBuff can not be zero.") ;

	DWORD			cbRead = 0 ;
	if ( !::InternetReadFile (
			m_hRequest
			, pbyBuff
			, cbBuff
			, &cbRead
			)
		)
		HttpTool::ThrowException (HTTPCLIENT_ERR_INTERNETREADFILE_FAILED, ::GetLastError ()) ;

	return cbRead ;
}

/*!
 * This method saves the content (the data stream except headers) of a returned HTTP response
 * to a file which is specified by szFilePath.
 *
 * \param szFilePath	[in] A file path to save the return content.
 * \param bOverwrite	[in] If this is TRUE, a file is overwritten if aleady exists. otherwise an exception is thrown.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
void CHttpResponseT<HttpTool>::SaveContent (PCSZ szFilePath, BOOL bOverwrite)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szFilePath != NULL, "CHttpResponseT::SaveContent: szFilePath can not be NULL.") ;

	// Checks whether the target file aleady exists.
	if ( !bOverwrite )
	{
		if ( HttpTool::FileExists (szFilePath) )
			HttpTool::ThrowException (HTTPCLIENT_ERR_FILE_ALEADY_EXISTS, NO_ERROR, szFilePath) ;
	}

	// Create a file to save content stream
	HANDLE		hFile = HttpTool::CreateFileAlwaysToWrite (szFilePath) ;

	if ( hFile == INVALID_HANDLE_VALUE )
		HttpTool::ThrowException (HTTPCLIENT_ERR_OPENFILE_FAILED, ::GetLastError (), szFilePath) ;

	const DWORD		cbBuff = 1024 * 10 ;				// use 10k buffer
	BYTE			byBuff[cbBuff] ;
	DWORD			cbRead, cbWritten, cbRemain ;

	// Reads the data stream returned by the HTTP server.
	while ( cbRead = ReadContent (byBuff, cbBuff) ) {
		cbRemain = cbRead ;

		do {
			if ( !::WriteFile (hFile, byBuff + (cbRead - cbRemain) , cbRemain, &cbWritten, NULL) ) {
				DWORD		dwLastError = ::GetLastError () ;
				::CloseHandle (hFile) ;
				HttpTool::ThrowException (HTTPCLIENT_ERR_WRITEFILE_FAILED, dwLastError, szFilePath) ;
			}
			cbRemain -= cbWritten ;
		} while ( cbRemain > 0 ) ;
	}
	::CloseHandle (hFile) ;
}


///////////////////////////////////////// CHttpResponseT /////////////////////////////////////////


///////////////////////////////////////// CHttpPostStatT /////////////////////////////////////////
/*!
 * This is a default constructor with no argument.
 */
template <typename HttpTool>
CHttpPostStatT<HttpTool>::CHttpPostStatT (void)
	throw ()
{
	// Initialize member variables
	_InitMemberVariables () ;
}

/*!
 * This is a copy constructor.
 * If memory allocation failed, the messages contained in the objPostStat object will
 * not be copied and the internal string pointers will point to NULL.
 *
 * \param objPostStat			[in] An CHttpPostStat object.
 */
template <typename HttpTool>
CHttpPostStatT<HttpTool>::CHttpPostStatT (const CHttpPostStatT & objPostStat)
	throw ()
{
	operator= (objPostStat) ;
}

/*!
 * This is a default destructor.
 */
template <typename HttpTool>
CHttpPostStatT<HttpTool>::~CHttpPostStatT (void)
	throw ()
{
	_MakeUnActive () ;
}

// Initialize member variables
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_InitMemberVariables (void)
	throw ()
{
	m_bIsActive = FALSE ;

	m_cbActualTotal = 0 ;
	m_cbActualPosted = 0 ;

	m_cbTotal = 0 ;
	m_cbPosted = 0 ;

	m_cParam = 0 ;
	m_cParamPosted = 0 ;
	m_cFile = 0 ;
	m_cFilePosted = 0 ;

	m_szCurrParam = NULL ;
	m_szCurrFile = NULL ;
	m_cbCurrParam = 0 ;
	m_cbCurrParamPosted = 0 ;
}

// Copy a string.
// The destination pointer will point to NULL if memory allocation failed.
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_SetString (PSZ * pszDest, PCSZ szSrc)
	throw ()
{
	_ASSERTE ( pszDest != NULL ) ;

	if ( szSrc == NULL ) {
		SAFEFREE ( (*pszDest) ) ;
		return ;
	}

	// If the source and destination string are the same, just return.
	if ( *pszDest != NULL ) {
		if ( 0 == HttpTool::StringCmp (*pszDest, szSrc) )
			return ;

		SAFEFREE ( (*pszDest) ) ;
	}

	*pszDest = (PSZ) ::malloc (sizeof (CharType) * (HttpTool::StringLen (szSrc) + 1)) ;

	// If memory allocation failed, just return.
	if ( *pszDest == NULL )
		return ;

	HttpTool::StringCopy (*pszDest, szSrc) ;
}

/*!
 * This method returns a boolean which indicates whether the POST is in progress or not.
 *
 * \return			TRUE if the POST is in progress, otherwise FALSE
 */
template <typename HttpTool>
BOOL CHttpPostStatT<HttpTool>::IsActive (void) const
	throw ()
{
	return m_bIsActive ;
}

/*!
 * This method returns the actual number of bytes to send to a HTTP web server.
 * The returned size includes the size of boundaries, parameter names, and other elements.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The actual number of bytes which is going to be sent to a HTTP web server
 */
template <typename HttpTool>
size_t CHttpPostStatT<HttpTool>::ActualTotalByte (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::ActualTotalByte: The post context is not active.") ;
	return m_cbActualTotal ;
}

/*!
 * This method returns the actual number of bytes posted to a HTTP web server.
 * The returned size includes the size of boundaries, parameter names, and other elements.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The actual number of bytes posted to a HTTP web server
 */
template <typename HttpTool>
size_t CHttpPostStatT<HttpTool>::ActualPostedByte (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::ActualPostedByte: The post context is not active.") ;
	return m_cbActualPosted ;
}

/*!
 * This method returns the number of bytes to send to a HTTP web server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The number of bytes to send to a HTTP web server
 */
template <typename HttpTool>
size_t CHttpPostStatT<HttpTool>::TotalByte (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::TotalByte: The post context is not active.") ;
	return m_cbTotal ;
}

/*!
 * This method returns the number of bytes posted to a HTTP web server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The number of bytes posted to a HTTP web server
 */
template <typename HttpTool>
size_t CHttpPostStatT<HttpTool>::PostedByte (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::PostedByte: The post context is not active.") ;
	return m_cbPosted ;
}

/*!
 * This method returns the total number of parameters to send to a HTTP web server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The total number of parameters
 */
template <typename HttpTool>
DWORD CHttpPostStatT<HttpTool>::TotalCount (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::TotalCount: The post context is not active.") ;
	return m_cParam ;
}

/*!
 * This method returns the number of posted parameters.
 * The count includes the current parameter which is being sent to the server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The number of posted parameters
 */
template <typename HttpTool>
DWORD CHttpPostStatT<HttpTool>::PostedCount (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::PostedCount: The post context is not active.") ;
	return m_cParamPosted ;
}

/*!
 * This method returns the number of file parameters.
 * The file parameter is a parameter which contains a file path to upload.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The number of file parameters
 */
template <typename HttpTool>
DWORD CHttpPostStatT<HttpTool>::FileCount (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::FileCount: The post context is not active.") ;
	return m_cFile ;
}

/*!
 * This method returns the number of posted file parameters.
 * The file parameter is a parameter which contains a file path to upload.
 * If the current parameter is a file parameter, then the count includes the current parameter.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The number of posted file parameters
 */
template <typename HttpTool>
DWORD CHttpPostStatT<HttpTool>::PostedFileCount (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::PostedFileCount: The post context is not active.") ;
	return m_cFilePosted ;
}

/*!
 * This method returns the current parameter name.
 * The current parameter is a parameter which is being sent to the server.
 * It always returns a null-terminated string. The returned string is owned by
 * the CHttpPostStat. So you can not free it.
 * If the system fails to allocate memory for the current parameter name,
 * it will return "NULL" instead of NULL.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The current parameter name
 */
template <typename HttpTool>
typename CHttpPostStatT<HttpTool>::PCSZ
CHttpPostStatT<HttpTool>::CurrParam (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::CurrParam: The post context is not active.") ;

	// If the string of the current parameter is NULL,
	// returns "NULL" instead of NULL
	if ( m_szCurrParam == NULL )
		return HttpTool::szNULL ;

	return m_szCurrParam ;
}

/*!
 * This method returns the file path which is currently being uploaded.
 * It always returns a null-terminated string. The returned string is owned by
 * the CHttpPostStat. So you can not free it.
 * If the current parameter is not a file parameter or the system fails to allocate
 * memory for the current file path, it will return "NULL".
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The current file path
 */
template <typename HttpTool>
typename CHttpPostStatT<HttpTool>::PCSZ
CHttpPostStatT<HttpTool>::CurrFile (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::CurrFile: The post context is not active.") ;

	// If the string of the current file is NULL,
	// returns "NULL" instead of NULL
	if ( m_szCurrFile == NULL )
		return HttpTool::szNULL ;

	return m_szCurrFile ;
}

/*!
 * This method returns the number of bytes of the current parameter.
 * The current parameter is a parameter which is being sent to the server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The number of bytes of the current parameter
 */
template <typename HttpTool>
size_t CHttpPostStatT<HttpTool>::CurrParamTotalByte (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::CurrParamTotalByte: The post context is not active.") ;
	return m_cbCurrParam ;
}

/*!
 * This method returns the number of posted bytes of the current parameter.
 * The current parameter is a parameter which is being sent to the server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The number of posted bytes of the current parameter
 */
template <typename HttpTool>
size_t CHttpPostStatT<HttpTool>::CurrParamPostedByte (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::CurrParamPostedByte: The post context is not active.") ;
	return m_cbCurrParamPosted ;
}

/*!
 * This method returns the number of remained bytes of the current parameter.
 * The current parameter is a parameter which is being sent to the server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			The number of remained bytes of the current parameter
 */
template <typename HttpTool>
size_t CHttpPostStatT<HttpTool>::CurrParamRemainByte (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::CurrParamRemainByte: The post context is not active.") ;
	return m_cbCurrParam - m_cbCurrParamPosted ;
}

/*!
 * This method returns TRUE if the current parameter is a file parameter,
 * otherwise it will return FALSE.
 * The current parameter is a parameter which is being sent to the server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			TRUE if the current parameter is a file parameter, otherwise FALSE
 */
template <typename HttpTool>
BOOL CHttpPostStatT<HttpTool>::CurrParamIsFile (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::CurrParamIsFile: The post context is not active.") ;
	return (m_szCurrFile != NULL) ;
}

/*!
 * This method returns TRUE if the current parameter is completely posted,
 * otherwise it will return FALSE.
 * The current parameter is a parameter which is being sent to the server.
 * If the IsActive is FALSE, you shouldn't call this method.
 *
 * \return			TRUE if the current parameter is completely posted, otherwise FALSE
 */
template <typename HttpTool>
BOOL CHttpPostStatT<HttpTool>::CurrParamIsComplete (void) const
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::CurrParamIsComplete: The post context is not active.") ;
	return (CurrParamTotalByte () == CurrParamPostedByte ()) ;
}

/*!
 * \internal
 * This method returns TRUE if all bytes are sent.
 *
 * \return			Returns TRUE if all bytes are sent
 */
template <typename HttpTool>
BOOL CHttpPostStatT<HttpTool>::_IsComplete (void) const
	throw ()
{
	_ASSERTE ( m_bIsActive ) ;
	return (ActualTotalByte () == ActualPostedByte ()) ;
}

/*!
 * \internal
 * This method tests whether the m_cbActualPosted is safe to increase.
 *
 * \param nBytes		[in] The number of bytes to add to the m_cbActualPosted.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_TestAddActualPostedBytes (size_t nBytes)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::_TestAddActualPostedBytes: The post context is not active.") ;

	try {
		::SafeInt<size_t>	cbActualPosted = m_cbActualPosted ;

		_ASSERTE ( (cbActualPosted + nBytes) <= m_cbActualTotal ) ;
		cbActualPosted += nBytes ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}
}

/*!
 * \internal
 * This method increases the m_cbActualPosted member variable.
 *
 * \param nBytes		[in] The number of bytes to add to the m_cbActualPosted.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_AddActualPostedBytes (size_t nBytes)
	throw (Exception &)
{
	_TestAddActualPostedBytes (nBytes) ;
	m_cbActualPosted += nBytes ;
}

/*!
 * \internal
 * This method cleans all internal states and closes the POST state.
 */
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_MakeUnActive (void)
	throw ()
{
	SAFEFREE (m_szCurrParam) ;
	SAFEFREE (m_szCurrFile) ;

	_InitMemberVariables () ;
}

/*!
 * \internal
 * This method starts a new state of the HTTP POST.
 *
 * \param cbActualTotal		[in] The actual total number of bytes of the request.
 * \param cbTotal			[in] The total number of bytes of the request.
 * \param cParam			[in] The number of parameters.
 * \param cFile				[in] The number of file parameters.
 */
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_MakeActive (size_t cbActualTotal, size_t cbTotal, DWORD cParam
												, DWORD cFile)
	throw ()
{
	_MakeUnActive () ;

	m_bIsActive = TRUE ;
	m_cbActualTotal = cbActualTotal ;
	m_cbTotal = cbTotal ;
	m_cParam = cParam ;
	m_cFile = cFile ;
}

/*!
 * \internal
 * This method tests whether a new current parameter is safely set.
 *
 * \param szCurrParam		[in] The name of the current parameter.
 * \param cbCurrParam		[in] The number of bytes of the current parameter.
 * \param bIsFile			[in] TRUE if the current parameter is a file parameter.
 * \param szCurrFile		[in] The file path of the current parameter.
 * \throw					Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_TestStartNewEntry (PCSZ /* szCurrParam */, size_t /* cbCurrParam */
												, BOOL bIsFile, PCSZ /* szCurrFile */)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::_TestStartNewEntry: The post context is not active.") ;

	try {
		::SafeInt<DWORD>		cParamPosted = m_cParamPosted ;
		_ASSERTE ( (cParamPosted + 1) <= m_cParam ) ;
		cParamPosted++ ;

		if ( bIsFile ) {
			::SafeInt<DWORD>	cFilePosted = m_cFilePosted ;
			_ASSERTE ( (cFilePosted + 1) <= m_cFile ) ;
			cFilePosted++ ;
		}
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}
}

/*!
 * \internal
 * This method sets a new current parameter.
 * If memory allocation is failed for strings (szCurrParam, szCurrFile),
 * NULL is saved. (and "NULL" is returned when the user requests these values)
 *
 * \param szCurrParam		[in] The name of the current parameter.
 * \param cbCurrParam		[in] The number of bytes of the current parameter.
 * \param bIsFile			[in] TRUE if the current parameter is a file parameter.
 * \param szCurrFile		[in] The file path of the current parameter.
 * \throw					Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_StartNewEntry (PCSZ szCurrParam, size_t cbCurrParam
												, BOOL bIsFile, PCSZ szCurrFile)
	throw (Exception &)
{
	_TestStartNewEntry (szCurrParam, cbCurrParam, bIsFile, szCurrFile) ;

	if ( bIsFile ) {
		m_cFilePosted++ ;
		_SetString (&m_szCurrFile, szCurrFile) ;
	} else {
		_SetString (&m_szCurrFile, NULL) ;
	}

	m_cParamPosted++ ;
	_SetString (&m_szCurrParam, szCurrParam) ;

	m_cbCurrParam = cbCurrParam ;
	m_cbCurrParamPosted = 0 ;
}

/*!
 * \internal
 * This method tests whether the counters related to the number of posted bytes is safe to increase.
 * (m_cbPosted, m_cbCurrParamPosted, m_cbActualPosted)
 *
 * \param nBytes			[in] The number of bytes to add.
 * \throw					Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_TestAddPostedBytes (size_t nBytes)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_bIsActive, "CHttpPostStatT::_TestAddPostedBytes: The post context is not active.") ;

	try {
		_TestAddActualPostedBytes (nBytes) ;

		::SafeInt<size_t>		cbPosted = m_cbPosted ;
		::SafeInt<size_t>		cbCurrParamPosted = m_cbCurrParamPosted ;

		_ASSERTE ( (cbPosted + nBytes) <= m_cbTotal ) ;
		_ASSERTE ( (cbCurrParamPosted + nBytes) <= m_cbCurrParam ) ;

		cbPosted += nBytes ;
		cbCurrParamPosted += nBytes ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}
}

/*!
 * \internal
 * This method increases all counters related to the number of posted bytes.
 * (m_cbPosted, m_cbCurrParamPosted, m_cbActualPosted)
 *
 * \param nBytes			[in] The number of bytes to add.
 * \throw					Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
void CHttpPostStatT<HttpTool>::_AddPostedBytes (size_t nBytes)
	throw (Exception &)
{
	_TestAddPostedBytes (nBytes) ;

	m_cbPosted += nBytes ;
	m_cbCurrParamPosted += nBytes ;
	_AddActualPostedBytes (nBytes) ;
}

/*!
 * This operator copies the state information from the source object.
 * All strings contained in the targeted object can be NULL if memory allocation failed.
 * (and "NULL" is returned when the user requests these values)
 *
 * \param objPostStat		[in] A CHttpPostStat object.
 * \return					The targeted object.
 */
template <typename HttpTool>
CHttpPostStatT<HttpTool> & 
CHttpPostStatT<HttpTool>::operator= (const CHttpPostStatT<HttpTool> & objPostStat)
	throw ()
{
	if ( this == &objPostStat )
		return *this ;

	_MakeUnActive () ;

	m_bIsActive = objPostStat.m_bIsActive ;

	m_cbActualTotal = objPostStat.m_cbActualTotal ;
	m_cbActualPosted = objPostStat.m_cbActualPosted ;

	m_cbTotal = objPostStat.m_cbTotal ;
	m_cbPosted = objPostStat.m_cbPosted ;

	m_cParam = objPostStat.m_cParam ;
	m_cParamPosted = objPostStat.m_cParamPosted ;
	m_cFile = objPostStat.m_cFile ;
	m_cFilePosted = objPostStat.m_cFilePosted ;

	_SetString (&m_szCurrParam, objPostStat.m_szCurrParam) ;
	_SetString (&m_szCurrFile, objPostStat.m_szCurrFile) ;

	m_cbCurrParam = objPostStat.m_cbCurrParam ;
	m_cbCurrParamPosted = objPostStat.m_cbCurrParamPosted ;

	return *this ;
}
///////////////////////////////////////// CHttpPostStatT /////////////////////////////////////////

///////////////////////////////////////// CHttpUrlAnalyzer /////////////////////////////////////////
/*!
 * This is the default constructor. If you pass a URL to the constructor, 
 * It analyzes a URL using Analyze method.
 *
 * \param szUrl			[in] An URL to analyze. 
 * \param CodePage		[in] A code page of the URL. If the URL is a unicode string,
 *						     this parameter is ignored.
 *						     CP_UTF8, CP_UTF7 are not allowed.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa	Analyze
 */
template <typename HttpTool>
CHttpUrlAnalyzerT<HttpTool>::CHttpUrlAnalyzerT (PCSZ szUrl, UINT CodePage)
	throw (Exception &)
{
	Analyze (szUrl, CodePage) ;
}

/*!
 * This method clears all saved information and sets to 0.
 */
template <typename HttpTool>
void CHttpUrlAnalyzerT<HttpTool>::Reset (void)
	throw ()
{
	m_nProtocolIdx = m_cchProtocol = m_nServerAddrIdx = m_cchServerAddr
		= m_nServerPortIdx = m_cchServerPort = m_nUrlPathIdx = m_cchUrlPath
		= m_nSearchIdx = m_cchSearch = m_nBookmarkIdx = m_cchBookmark = 0 ;
}

/*!
 * This method analyzes a URL and saves information about its component parts.
 * It does not check whether the URL is valid. The URL itself is not saved.
 *
 * \param szUrl			[in] An URL to analyze. 
 * \param CodePage		[in] A code page of the URL. If the URL is a unicode string,
 *						     this parameter is ignored.
 *						     CP_UTF8, CP_UTF7 are not allowed.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool>
void CHttpUrlAnalyzerT<HttpTool>::Analyze (PCSZ szUrl, UINT CodePage)
	throw (Exception &)
{
	// The unicode encodings are not allowed
	HTTPCLIENT_ASSERT ((CodePage != CP_UTF8) && (CodePage != CP_UTF7)
		, "CHttpUrlAnalyzerT::Analyze: CP_UTF8 and CP_UTF7 can not be used for the CodePage parameter.") ;

	Reset () ;

	if ( szUrl == NULL || szUrl[0] == '\0' )
		return ;

	// Checks overflow
	try {
		::SafeInt<DWORD>	dw = HttpTool::StringLen (szUrl) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	PCSZ		pchNextUrl = szUrl ;
	DWORD		nNextUrlCharIdx = 0 ;

	{
		// Get the protocol
		PCSZ		pchColonSlashSlash = NULL ;

		{
			// Finds "://" in the URL
			for (pchColonSlashSlash = pchNextUrl; *pchColonSlashSlash != '\0'; pchColonSlashSlash++) {

				// If the character is a leadbyte of the double byte character set
				if ( HttpTool::IsDBCSLeadByteEx (CodePage, *pchColonSlashSlash) ) {
					// Ignores a trailing byte
					if ( *(pchColonSlashSlash + 1) != '\0' )
						pchColonSlashSlash++ ;
					continue ;
				}

				if ( (*pchColonSlashSlash == ':')
						&& (*(pchColonSlashSlash + 1) != '\0') && (*(pchColonSlashSlash + 1) == '/')
						&& (*(pchColonSlashSlash + 2) != '\0') && (*(pchColonSlashSlash + 2) == '/') )
					break ;
			}
		}

		if ( *pchColonSlashSlash != '\0' ) {
			for (; pchNextUrl != pchColonSlashSlash; pchNextUrl++, m_cchProtocol++, nNextUrlCharIdx++) ;

			pchNextUrl += 3/* == strlen (szColonSlashSlash)*/ ;
			nNextUrlCharIdx += 3 ;
		}
	}

	{
		// Get server address and server port
		BOOL		bFoundColon = FALSE ;

		m_nServerAddrIdx = nNextUrlCharIdx ;
		for (; *pchNextUrl != '/' && *pchNextUrl != '?' && *pchNextUrl != '#' && *pchNextUrl != '\0';
				pchNextUrl++, nNextUrlCharIdx++) {

			// If the character is a leadbyte of the double byte character set
			if ( HttpTool::IsDBCSLeadByteEx (CodePage, *pchNextUrl) ) {
				m_cchServerPort += (bFoundColon ? 1 : 0) ;
				m_cchServerAddr += (bFoundColon ? 0 : 1) ;

				// Ignores a trailing byte
				if ( *(pchNextUrl + 1) != '\0' ) {
					pchNextUrl++ ;
					nNextUrlCharIdx++ ;
					m_cchServerPort += (bFoundColon ? 1 : 0) ;
					m_cchServerAddr += (bFoundColon ? 0 : 1) ;
				}
				continue ;
			}

			if ( bFoundColon ) {
				m_cchServerPort++ ;
				continue ;
			}

			if ( *pchNextUrl == ':' ) {
				m_nServerPortIdx = nNextUrlCharIdx + 1 ;
				bFoundColon = TRUE ;
				continue ;
			}

			m_cchServerAddr++ ;
		}

		if ( !bFoundColon )
			m_nServerPortIdx = nNextUrlCharIdx ;
	}

	{
		// Get path and search string
		BOOL		bFoundQuestionMark = FALSE ;

		m_nUrlPathIdx = nNextUrlCharIdx ;
		for (; *pchNextUrl != '#' && *pchNextUrl != '\0'; pchNextUrl++, nNextUrlCharIdx++) {

			// If the character is a leadbyte of the double byte character set
			if ( HttpTool::IsDBCSLeadByteEx (CodePage, *pchNextUrl) ) {
				m_cchSearch += (bFoundQuestionMark ? 1 : 0) ;
				m_cchUrlPath += (bFoundQuestionMark ? 0 : 1) ;

				// Ignores a trailing byte
				if ( *(pchNextUrl + 1) != '\0' ) {
					pchNextUrl++ ;
					nNextUrlCharIdx++ ;
					m_cchSearch += (bFoundQuestionMark ? 1 : 0) ;
					m_cchUrlPath += (bFoundQuestionMark ? 0 : 1) ;
				}
				continue ;
			}

			if ( bFoundQuestionMark ) {
				m_cchSearch++ ;
				continue ;
			}

			if ( *pchNextUrl == '?' ) {
				m_nSearchIdx = nNextUrlCharIdx ;
				m_cchSearch++ ;
				bFoundQuestionMark = TRUE ;
				continue ;
			}

			m_cchUrlPath++ ;
		}

		if ( !bFoundQuestionMark )
			m_nSearchIdx = nNextUrlCharIdx ;
	}

	{
		// Get bookmark
		m_nBookmarkIdx = nNextUrlCharIdx ;
		for (; *pchNextUrl != '\0'; pchNextUrl++, nNextUrlCharIdx++, m_cchBookmark++) ;
	}
}
///////////////////////////////////////// CHttpUrlAnalyzer /////////////////////////////////////////


///////////////////////////////////////// CHttpClientT /////////////////////////////////////////
/*!
 * This is a default constructor with no argument.
 */
template <typename HttpTool, typename HttpEncoder>
CHttpClientT<HttpTool, HttpEncoder>::CHttpClientT (void)
	throw ()
{
	// Initializes member variables
	m_bStrictFileCheck = FALSE ;
	m_bUseUtf8 = FALSE ;
	m_nAnsiCodePage = CP_ACP ;

	// variables related to the WinInet API
	m_szInternetUserAgent = NULL ;
	m_dwInternetAccessType = INTERNET_OPEN_TYPE_PRECONFIG ;
	m_szInternetProxyName = NULL ;
	m_szInternetProxyBypass = NULL ;
	m_dwInternetFlags = 0 ;
	m_szProxyUserName = NULL ;
	m_szProxyPassword = NULL ;


	// Initializes a boundary
	m_szBoundary = NULL ;
	m_szUploadContType = NULL ;
	m_szBoundaryA = NULL ;
	m_bNeedToFreeBoundaryA = FALSE ;
	try {
		m_szBoundary = HttpTool::CreateUploadBoundary () ;
		if ( m_szBoundary ) {
			m_szUploadContType = 
				(PSZ) ::malloc (sizeof (CharType) 
						* (30 /* ::strlen (szMultipartFormDataBoundary) */
						+ HttpTool::StringLen (m_szBoundary)
						+ 1)
				) ;
			if ( m_szUploadContType == NULL )
				throw "Out of memory" ;

			HttpTool::StringCopy (m_szUploadContType, HttpTool::szMultipartFormDataBoundary) ;
			HttpTool::StringCat (m_szUploadContType, m_szBoundary) ;
			m_szBoundaryA = _String2Ansi (m_szBoundary, m_bNeedToFreeBoundaryA) ;
		}
	} catch (...) {
		SAFEFREE (m_szBoundary) ;
		SAFEFREE (m_szUploadContType) ;
	}

	// Initializes the context related to the POST context
	_InitPostContext () ;
}

/*!
 * This is a default destructor.
 */
template <typename HttpTool, typename HttpEncoder>
CHttpClientT<HttpTool, HttpEncoder>::~CHttpClientT (void)
	throw ()
{
	// Releases the POST context
	_EndPostContext () ;

	// Releases the Boundary
	SAFEFREE (m_szBoundary) ;
	SAFEFREE (m_szUploadContType) ;
	if ( m_bNeedToFreeBoundaryA )
		SAFEFREE (m_szBoundaryA) ;

	// Releases the variables related to the WinInet API
	SAFEFREE (m_szInternetUserAgent) ;
	SAFEFREE (m_szInternetProxyName) ;
	SAFEFREE (m_szInternetProxyBypass) ;
	SAFEFREE (m_szProxyUserName) ;
	SAFEFREE (m_szProxyPassword) ;
}



template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::_String2AnsiLen (PCSZ szStr)
	throw (Exception &)
{
	return HttpEncoder::AnsiEncodeLen (szStr, m_nAnsiCodePage) ;
}

template <typename HttpTool, typename HttpEncoder>
PCSTR CHttpClientT<HttpTool, HttpEncoder>::_String2Ansi (PCSZ szStr, BOOL & bNeedToFree)
	throw (Exception &)
{
	if ( HttpTool::IsAnsi () ) {
		bNeedToFree = FALSE ;
		return reinterpret_cast<PCSTR> (szStr) ;
	}

	DWORD				cchAnsi = HttpEncoder::AnsiEncodeLen (szStr, m_nAnsiCodePage) ;
	::SafeInt<size_t>	cbRequired = 0 ;

	try {
		cbRequired += cchAnsi ;
		cbRequired += 1 ;
		cbRequired *= sizeof (CHAR) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	PSTR		szBuff = (PSTR) ::malloc (cbRequired.Value ()) ;
	if ( szBuff == NULL )
		HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	try {
		HttpEncoder::AnsiEncode (szBuff, szStr, m_nAnsiCodePage) ;
	} catch (Exception &) {
		SAFEFREE (szBuff) ;
		throw ;
	}

	bNeedToFree = TRUE ;
	return szBuff ;
}

template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::_UrlEncodeLenA (PCSZ szStr)
	throw (Exception &)
{
	return HttpEncoder::UrlEncodeLenA (szStr, m_bUseUtf8, m_nAnsiCodePage) ;
}

template <typename HttpTool, typename HttpEncoder>
PCSTR CHttpClientT<HttpTool, HttpEncoder>::_UrlEncodeA (PSTR szBuff, PCSZ szStr)
	throw (Exception &)
{
	return HttpEncoder::UrlEncodeA (szBuff, szStr, m_bUseUtf8, m_nAnsiCodePage) ;
}

template <typename HttpTool, typename HttpEncoder>
PCSTR CHttpClientT<HttpTool, HttpEncoder>::_UrlEncodeA (PCSZ szStr)
	throw (Exception &)
{
	DWORD				cchEncoded = _UrlEncodeLenA (szStr) ;
	::SafeInt<size_t>	cbRequired = 0 ;
	try {
		cbRequired += cchEncoded ;
		cbRequired += 1 ;
		cbRequired *= sizeof (CHAR) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	PSTR		szBuff = (PSTR) ::malloc (cbRequired.Value ()) ;
	if ( szBuff == NULL )
		HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	try {
		_UrlEncodeA (szBuff, szStr) ;
	} catch (Exception &) {
		SAFEFREE (szBuff) ;
		throw ;
	}
	return szBuff ;
}

template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::_UrlEncodeLen (PCSZ szStr)
	throw (Exception &)
{
	return HttpEncoder::UrlEncodeLen (szStr, m_bUseUtf8, m_nAnsiCodePage) ;
}

template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::_UrlEncode (PSZ szBuff, PCSZ szStr)
	throw (Exception &)
{
	return HttpEncoder::UrlEncode (szBuff, szStr, m_bUseUtf8, m_nAnsiCodePage) ;
}

template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::_UrlEncode (PCSZ szStr)
	throw (Exception &)
{
	DWORD				cchEncoded = _UrlEncodeLen (szStr) ;
	::SafeInt<size_t>	cbRequired = 0 ;
	try {
		cbRequired += cchEncoded ;
		cbRequired += 1 ;
		cbRequired *= sizeof (CharType) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	PSZ			szBuff = (PSZ) ::malloc (cbRequired.Value ()) ;
	if ( szBuff == NULL )
		HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	try {
		_UrlEncode (szBuff, szStr) ;
	} catch (Exception &) {
		SAFEFREE (szBuff) ;
		throw ;
	}
	return szBuff ;
}

template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::_Utf8EncodeLen (PCSZ szStr)
	throw (Exception &)
{
	return HttpEncoder::Utf8EncodeLen (szStr, m_nAnsiCodePage) ;
}

template <typename HttpTool, typename HttpEncoder>
PCSTR CHttpClientT<HttpTool, HttpEncoder>::_Utf8Encode (PSTR szBuff, PCSZ szStr)
	throw (Exception &)
{
	return HttpEncoder::Utf8Encode (szBuff, szStr, m_nAnsiCodePage) ;
}

template <typename HttpTool, typename HttpEncoder>
PCSTR CHttpClientT<HttpTool, HttpEncoder>::_Utf8Encode (PCSZ szStr)
	throw (Exception &)
{
	DWORD				cchEncoded = _Utf8EncodeLen (szStr) ;
	::SafeInt<size_t>	cbRequired = 0 ;
	try {
		cbRequired += cchEncoded ;
		cbRequired += 1 ;
		cbRequired *= sizeof (CHAR) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	PSTR		szBuff = (PSTR) ::malloc (cbRequired.Value ()) ;
	if ( szBuff == NULL )
		HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	try {
		_Utf8Encode (szBuff, szStr) ;
	} catch (Exception &) {
		SAFEFREE (szBuff) ;
		throw ;
	}
	return szBuff ;
}

/*!
 * This method allocates memory and copies the target string if the szTargetString and the szOldString
 * are not the same. If the szTargetString and the szOldString are the same, it returns szOldString.
 *
 * \param szTargetString	[in] A target string to compare.
 * \param szOldString		[in] An original string to compare.
 * \return					A copied target string or szOldString.
 * \throw					Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PSZ
CHttpClientT<HttpTool, HttpEncoder>::_AllocNCopyIfNewString (PCSZ szTargetString, PSZ szOldString)
	throw (Exception &)
{
	if ( szTargetString == szOldString )
		return szOldString ;

	if ( szTargetString == NULL )
		return NULL ;

	if ( szOldString && !HttpTool::StringCmp (szTargetString, szOldString) )
		return szOldString ;

	::SafeInt<size_t>	cbRequired = 0 ;
	try {
		cbRequired = HttpTool::StringLen (szTargetString) ;
		cbRequired++ ;
		cbRequired *= sizeof (CharType) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	PSZ		szNewString = (PSZ) ::malloc (cbRequired.Value ()) ;
	if ( szNewString == NULL )
		HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	HttpTool::StringCopy (szNewString, szTargetString) ;
	return szNewString ;
}

/*!
 * This method sets the current StrictFileCheck property.
 * If the StrictFileCheck is TRUE, an exception will be thrown
 * when it fails to open a file which is going to be uploaded.
 * If FALSE, it does nothing. The default is FALSE.
 *
 * \param bStrict		[in] A boolean value which specifies StrictFileCheck.
 * \return				Previously set value.
 *
 * The following code snippet demonstrates the usage of this property.
 * \code
...
CHttpClient					objHttpReq ;

// CHttpClient will throw a httpclientexception if it fails to open a file.
objHttpReq.SetStrictFileCheck (TRUE) ;

try {
	// Adds HTTP parameters
	objHttpReq.AddParam (_T ("title"), _T ("The memory of K-NET")) ;
	objHttpReq.AddParam (_T ("photo"), _T ("C:\\myphoto\\k-net.jpg"), ParamFile) ;

	// Starts a new HTTP UPLOAD
	objHttpReq.RequestUpload (...) or objHttpReq.BeginUpload (...) ;
	...

} catch (httpclientexception & e) {
	// If it failed to open the file (C:\myphoto\k-net.jpg)
	if ( e.LastError () == HTTPCLIENT_ERR_OPENFILE_FAILED ) {
		...		// Place error handling codes here.
	}
	...
}
 * \endcode
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::SetStrictFileCheck (BOOL bStrict)
	throw ()
{
	BOOL	bOldStrictFileCheck = m_bStrictFileCheck ;
	m_bStrictFileCheck = bStrict ;
	return bOldStrictFileCheck ;
}

/*!
 * This method returns the current StrictFileCheck property.
 * If the StrictFileCheck is TRUE, an exception will be thrown
 * when it fails to open a file which is going to be uploaded.
 * If FALSE, it does nothing. The default is FALSE.
 *
 * \return				StrictFileCheck property.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::GetStrictFileCheck (void) const
	throw ()
{
	return m_bStrictFileCheck ;
}

/*!
 * This method returns the current StrictFileCheck property.
 * If the StrictFileCheck is TRUE, an exception will be thrown
 * when it fails to open a file which is going to be uploaded.
 * If FALSE, it does nothing. The default is FALSE.
 *
 * \return				StrictFileCheck property.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::StrictFileCheck (void) const
	throw ()
{
	return GetStrictFileCheck () ;
}


/*!
 * This method sets the current UseUtf8 property.
 * If the UseUtf8 is TRUE, all request will be sent in UTF-8 encoding,
 * otherwise, all request will be sent in ANSI encoding.
 * If a web server uses UTF-8 encoding, you should set TRUE.
 * It is not allowed to call this method if the BeginPost or BeginUpload method is not finished.
 * The default is FALSE.
 * 
 * \param bUseUtf8		[in] A boolean value which specifies UseUtf8.
 * \return				previously set value.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::SetUseUtf8 (BOOL bUseUtf8)
	throw (Exception &)
{
	// It is not allowed to call this method if the POST context is active.
	HTTPCLIENT_ASSERT (!_PostContextIsActive ()
		, "CHttpClientT::SetUseUtf8: It is not allowed to call this method if the POST context is active.") ;

	BOOL	bOldUseUtf8 = m_bUseUtf8 ;
	m_bUseUtf8 = bUseUtf8 ;
	return bOldUseUtf8 ;
}

/*!
 * This method returns the current UseUtf8 property.
 * If the UseUtf8 is TRUE, all request will be sent in UTF-8 encoding,
 * otherwise, all request will be sent in ANSI encoding.
 * If a web server uses UTF-8 encoding, you should set TRUE.
 * The default is FALSE.
 *
 * \return				UseUtf8 property.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::GetUseUtf8 (void) const
	throw ()
{
	return m_bUseUtf8 ;
}

/*!
 * This method returns the current UseUtf8 property.
 * If the UseUtf8 is TRUE, all request will be sent in UTF-8 encoding,
 * otherwise, all request will be sent in ANSI encoding.
 * If a web server uses UTF-8 encoding, you should set TRUE.
 * The default is FALSE.
 *
 * \return				UseUtf8 property.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::UseUtf8 (void) const
	throw ()
{
	return GetUseUtf8 () ;
}

/*!
 * This method sets the current ANSI code page which is used for all non-unicode strings.
 * If a web server uses an ANSI character set, you should set an appropriate ANSI code page.
 * It is not allowed to call this method if the BeginPost or BeginUpload method is not finished.
 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
 * The default is CP_ACP (which is the system's ANSI code page).
 * 
 * \param nAnsiCodePage	[in] An ANSI code page to set.
 * \return				previously set value.
 *
 * \sa	::WideCharToMultiByte, ::MultiByteToWideChar
 */
template <typename HttpTool, typename HttpEncoder>
UINT CHttpClientT<HttpTool, HttpEncoder>::SetAnsiCodePage (UINT nAnsiCodePage)
	throw (Exception &)
{
	// It is not allowed to call this method if the POST context is active.
	HTTPCLIENT_ASSERT (!_PostContextIsActive ()
		, "CHttpClientT::SetAnsiCodePage: It is not allowed to call this method if the POST context is active.") ;

	// It is not allowed to set the unicode encodings
	HTTPCLIENT_ASSERT ((nAnsiCodePage != CP_UTF8) && (nAnsiCodePage != CP_UTF7)
		, "CHttpClientT::SetAnsiCodePage: CP_UTF8 and CP_UTF7 can not be used for the nAnsiCodePage parameter.") ;

	UINT	nOldAnsiCodePage = m_nAnsiCodePage ;
	m_nAnsiCodePage = nAnsiCodePage ;
	return nOldAnsiCodePage ;
}

/*!
 * This method returns the current ANSI code page which is used for all non-unicode strings.
 * If a web server uses an ANSI character set, you should set an appropriate ANSI code page.
 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
 * The default is CP_ACP (which is the system's ANSI code page).
 *
 * \return				The current ANSI code page.
 */
template <typename HttpTool, typename HttpEncoder>
UINT CHttpClientT<HttpTool, HttpEncoder>::GetAnsiCodePage (void) const
	throw ()
{
	return m_nAnsiCodePage ;
}

/*!
 * This method returns the current ANSI code page which is used for all non-unicode strings.
 * If a web server uses an ANSI character set, you should set an appropriate ANSI code page.
 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
 * The default is CP_ACP (which is the system's ANSI code page).
 *
 * \return				The current ANSI code page.
 */
template <typename HttpTool, typename HttpEncoder>
UINT CHttpClientT<HttpTool, HttpEncoder>::AnsiCodePage (void) const
	throw ()
{
	return GetAnsiCodePage () ;
}

/*!
 * This method erases all HTTP headers which is sent to a web server.
 *
 * \return				FALSE if all header is aleady erased, otherwise TRUE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::ClearHeader (void)
	throw ()
{
	return m_mapHeader.Clear () ;
}

/*!
 * This method erases a HTTP header at the given index specified by nIdx.
 *
 * \param nIdx			[in] A header index to remove.
 * \return				TRUE if the header is found and removed, otherwise FALSE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::RemoveHeader (DWORD nIdx)
	throw ()
{
	return m_mapHeader.Remove (nIdx) ;
}

/*!
 * This method erases a HTTP header of which name is szName.
 * If the header has multiple values, you can specify a zero-based index for a specific value.
 *
 * \param szName		[in] A case-insensitive header name to remove. NULL is not allowed.
 * \param nIdx			[in] A zero-based value index if the header has multiple values. The default is zero.
 * \return				TRUE if the header is found and removed, otherwise FALSE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::RemoveHeader (PCSZ szName, DWORD nIdx)
	throw ()
{
	return m_mapHeader.Remove (szName, nIdx) ;
}

/*!
 * This method erases all HTTP headers of which name is szName.
 *
 * \param szName		[in] A case-insensitive header name to remove. NULL is not allowed.
 * \return				TRUE if the header is found and removed, otherwise FALSE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::RemoveAllHeader (PCSZ szName)
	throw ()
{
	return m_mapHeader.RemoveAll (szName) ;
}

/*!
 * This method adds a new HTTP header.
 * The header's name and the header's value can not be NULL or an empty string.
 *
 * \param szName		[in] A case-insensitive header name. NULL or an empty string is not allowed.
 * \param szValue		[in] A header value. NULL or an empty string is not allowed.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::AddHeader (PCSZ szName, PCSZ szValue)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientT::AddHeader: szName can not be NULL.") ;
	HTTPCLIENT_ASSERT (szName[0] != '\0', "CHttpClientT::AddHeader: szName can not be an empty string.") ;
	HTTPCLIENT_ASSERT (szValue != NULL, "CHttpClientT::AddHeader: szValue can not be NULL.") ;
	HTTPCLIENT_ASSERT (szValue[0] != '\0', "CHttpClientT::AddHeader: szValue can not be an empty string.") ;
	m_mapHeader.Add (szName, szValue) ;
}

/*!
 * This method modifies the HTTP header value of which name is szName.
 * If the header has multiple values, you can specify a zero-based index for a specific value.
 * If the header is not found and the nIdx is zero, it will add a new header.
 * The header's name and the header's value can not be NULL or an empty string.
 *
 * \param szName		[in] A case-insensitive header name. NULL or an empty string is not allowed.
 * \param szValue		[in] A header value. NULL or an empty string is not allowed.
 * \param nIdx			[in] A zero-based value index if the header has multiple values. The default is zero.
 * \throw				Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetHeader (PCSZ szName, PCSZ szValue, DWORD nIdx)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szName != NULL, "CHttpClientT::SetHeader: szName can not be NULL.") ;
	HTTPCLIENT_ASSERT (szName[0] != '\0', "CHttpClientT::SetHeader: szName can not be an empty string.") ;
	HTTPCLIENT_ASSERT (szValue != NULL, "CHttpClientT::SetHeader: szValue can not be NULL.") ;
	HTTPCLIENT_ASSERT (szValue[0] != '\0', "CHttpClientT::SetHeader: szValue can not be an empty string.") ;
	m_mapHeader.Set (szName, szValue, 0, nIdx) ;
}

/*!
 * This method returns a HTTP header name at the given index specified by nIdx.
 * If the index is out of range, it will return NULL. Otherwise it always returns a null-terminated string.
 * The returned string is owned by the CHttpClientT. So you can not free it.
 *
 * \param nIdx			[in] A zero-based header index.
 * \return				A header name.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetHeaderName (DWORD nIdx)
	throw ()
{
	return m_mapHeader.GetKey (nIdx) ;
}

/*!
 * This method returns a HTTP header at the given index specified by nIdx.
 * If the index is out of range, it will return NULL. Otherwise it always returns a null-terminated string.
 * The returned string is owned by the CHttpClient. So you can not free it.
 *
 * \param nIdx			[in] A zero-based header index.
 * \return				A header.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetHeader (DWORD nIdx)
	throw ()
{
	return m_mapHeader.GetValue (nIdx) ;
}

/*!
 * This method returns a HTTP header of which name is szName.
 * If the header has multiple values, you can specify a zero-based index for a specific value.
 * If the header is not found, it will return NULL. Otherwise it always returns a null-terminated string.
 * The returned string is owned by the CHttpClient. So you can not free it.
 *
 * \param szName		[in] A case-insensitive header name to find. NULL is not allowed.
 * \param nIdx			[in] A zero-based value index if the header has multiple values. The default is zero.
 * \return				A header.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetHeader (PCSZ szName, DWORD nIdx)
	throw ()
{
	return m_mapHeader.GetValue (szName, nIdx) ;
}

/*!
 * This method returns whether the HTTP header of which name is szName exists.
 * If the header has multiple values, you can specify a zero-based index for a specific value.
 *
 * \param szName		[in] A case-insensitive header name to find. NULL is not allowed.
 * \param nIdx			[in] A zero-based value index if the header has multiple values. The default is zero.
 * \return				TRUE if the header is found, otherwise FALSE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::HeaderExists (PCSZ szName, DWORD nIdx)
	throw ()
{
	return m_mapHeader.Exists (szName, nIdx) ;
}

/*!
 * This method returns the number of HTTP headers of which name is szName.
 * If the szName is NULL, it will return the number of all headers.
 *
 * \param szName		[in] A case-insensitive header name to find.
 * \return				The number of headers.
 */
template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::GetHeaderCount (PCSZ szName)
	throw ()
{
	return m_mapHeader.Count (szName) ;
}

/*!
 * This method erases all HTTP parameters.
 * It is not allowed to call this method if the BeginPost or BeginUpload method is not finished.
 *
 * \return				FALSE if all parameter is aleady erased, otherwise TRUE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::ClearParam (void)
	throw (Exception &)
{
	// It is not allowed to call this method if the POST context is active.
	HTTPCLIENT_ASSERT (!_PostContextIsActive ()
		, "CHttpClientT::ClearParam: It is not allowed to call this method if the POST context is active.") ;
	return m_mapParam.Clear () ;
}

/*!
 * This method erases a HTTP parameter at the given index specified by nIdx.
 * It is not allowed to call this method if the BeginPost or BeginUpload method is not finished.
 *
 * \param nIdx			[in] A zero-based parameter index to remove.
 * \return				TRUE if the parameter is found and removed, otherwise FALSE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::RemoveParam (DWORD nIdx)
	throw (Exception &)
{
	// It is not allowed to call this method if the POST context is active.
	HTTPCLIENT_ASSERT (!_PostContextIsActive ()
		, "CHttpClientT::RemoveParam: It is not allowed to call this method if the POST context is active.") ;
	return m_mapParam.Remove (nIdx) ;
}

/*!
 * This method erases a HTTP parameter of which name is szName.
 * If the parameter has multiple values, you can specify a zero-based index for a specific value.
 * It is not allowed to call this method if the BeginPost or BeginUpload method is not finished.
 *
 * \param szName		[in] A case-insensitive parameter name to remove. NULL is not allowed.
 * \param nIdx			[in] A zero-based value index if the parameter has multiple values. The default is zero.
 * \return				TRUE if the parameter is found and removed, otherwise FALSE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::RemoveParam (PCSZ szName, DWORD nIdx)
	throw (Exception &)
{
	// It is not allowed to call this method if the POST context is active.
	HTTPCLIENT_ASSERT (!_PostContextIsActive ()
		, "CHttpClientT::RemoveParam: It is not allowed to call this method if the POST context is active.") ;
	return m_mapParam.Remove (szName, nIdx) ;
}

/*!
 * This method erases all HTTP parameters of which name is szName.
 * It is not allowed to call this method if the BeginPost or BeginUpload method is not finished.
 *
 * \param szName		[in] A case-insensitive parameter name to remove. NULL is not allowed.
 * \return				TRUE if the parameter is found and removed, otherwise FALSE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::RemoveAllParam (PCSZ szName)
	throw (Exception &)
{
	// It is not allowed to call this method if the POST context is active.
	HTTPCLIENT_ASSERT (!_PostContextIsActive ()
		, "CHttpClientT::RemoveAllParam: It is not allowed to call this method if the POST context is active.") ;
	return m_mapParam.RemoveAll (szName) ;
}

/*!
 * This method adds a new HTTP parameter. The dwParamAttr specifies parameter attributes.
 * For more information about parameter attributes, see the ParamAttr enumerator.
 * It is not allowed to call this method if the BeginPost or BeginUpload method is not finished.
 *
 * \param szName		[in] A case-insensitive parameter name. NULL is not allowed.
 * \param szValue		[in] A parameter value. The default is NULL.
 * \param dwParamAttr	[in] A parameter attribute.
 *							 This is a bitwise Ored value of the ParamAttr enumerator.
 *							 The default is ParamNormal.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa	ParamAttr
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::AddParam (PCSZ szName, PCSZ szValue, DWORD dwParamAttr)
	throw (Exception &)
{
	// It is not allowed to call this method if the POST context is active.
	HTTPCLIENT_ASSERT (!_PostContextIsActive ()
		, "CHttpClientT::AddParam: It is not allowed to call this method if the POST context is active.") ;
	m_mapParam.Add (szName, szValue, dwParamAttr) ;
}

/*!
 * This method modifies the HTTP parameter value of which name is szName.
 * If the parameter has multiple values, you can specify a zero-based index for a specific value.
 * If the parameter is not found and the nIdx is zero, it will add a new parameter.
 * The dwParamAttr specifies parameter attributes.
 * For more information about parameter attributes, see the ParamAttr enumerator.
 * It is not allowed to call this method if the BeginPost or BeginUpload method is not finished.
 *
 * \param szName		[in] A case-insensitive parameter name. NULL is not allowed.
 * \param szValue		[in] A parameter value. The default is NULL.
 * \param dwParamAttr	[in] A parameter attribute.
 *							 This is a bitwise Ored value of the ParamAttr enumerator.
 *							 The default is ParamNormal.
 * \param nIdx			[in] A zero-based value index if the parameter has multiple values.
 *							 The default is zero.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa	ParamAttr
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetParam (PCSZ szName, PCSZ szValue, DWORD dwParamAttr, DWORD nIdx)
	throw (Exception &)
{
	// It is not allowed to call this method if the POST context is active.
	HTTPCLIENT_ASSERT (!_PostContextIsActive ()
		, "CHttpClientT::SetParam: It is not allowed to call this method if the POST context is active.") ;
	m_mapParam.Set (szName, szValue, dwParamAttr, nIdx) ;
}

/*!
 * This method returns a HTTP parameter name at the given index specified by nIdx.
 * If the index is out of range, it will return NULL. Otherwise it always returns a null-terminated string.
 * The returned string is owned by the CHttpClient. So you can not free it.
 *
 * \param nIdx			[in] A zero-based parameter index.
 * \return				A parameter name.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetParamName (DWORD nIdx)
	throw ()
{
	return m_mapParam.GetKey (nIdx) ;
}

/*!
 * This method returns a HTTP parameter at the given index specified by nIdx.
 * If the index is out of range, it will return NULL. Otherwise it always returns a null-terminated string.
 * The returned string is owned by the CHttpClient. So you can not free it.
 *
 * \param nIdx			[in] A zero-based parameter index.
 * \return				A parameter.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetParam (DWORD nIdx)
	throw ()
{
	return m_mapParam.GetValue (nIdx) ;
}

/*!
 * This method returns a HTTP parameter of which name is szName.
 * If the parameter has multiple values, you can specify a zero-based index for a specific value.
 * If the parameter is not found, it will return NULL. Otherwise it always returns a null-terminated string.
 * The returned string is owned by the CHttpClient. So you can not free it.
 *
 * \param szName		[in] A case-insensitive parameter name to find. NULL is not allowed.
 * \param nIdx			[in] A zero-based value index if the parameter has multiple values.
 *							 The default is zero.
 * \return				A parameter.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetParam (PCSZ szName, DWORD nIdx)
	throw ()
{
	return m_mapParam.GetValue (szName, nIdx) ;
}

/*!
 * This method returns a HTTP parameter attribute at the given index specified by nIdx.
 * If the index is out of range or the attribute is 0, it will return 0.
 *
 * \param nIdx			[in] A zero-based parameter index.
 * \return				A parameter attribute.
 *
 * \sa	ParamAttr
 */
template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::GetParamAttr (DWORD nIdx)
	throw ()
{
	return m_mapParam.GetFlag (nIdx) ;
}

/*!
 * This method returns an attribute of the HTTP parameter of which name is szName.
 * If the parameter has multiple values, you can specify a zero-based index for a specific value.
 * If the parameter is not found or the attribute is 0, it will return 0.
 *
 * \param szName		[in] A case-insensitive parameter name to find. NULL is not allowed.
 * \param nIdx			[in] A zero-based value index if the parameter has multiple values.
 *							 The default is zero.
 * \return				A parameter attribute.
 *
 * \sa	ParamAttr
 */
template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::GetParamAttr (PCSZ szName, DWORD nIdx)
	throw ()
{
	return m_mapParam.GetFlag (szName, nIdx) ;
}

/*!
 * This method returns whether the HTTP parameter of which name is szName exists.
 * If the parameter has multiple values, you can specify a zero-based index for a specific value.
 *
 * \param szName		[in] A case-insensitive parameter name to find. NULL is not allowed.
 * \param nIdx			[in] A zero-based value index if the parameter has multiple values.
 *							 The default is zero.
 * \return				TRUE if the parameter is found, otherwise FALSE.
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::ParamExists (PCSZ szName, DWORD nIdx)
	throw ()
{
	return m_mapParam.Exists (szName, nIdx) ;
}

/*!
 * This method returns the number of HTTP parameters of which name is szName.
 * If the szName is NULL, it will return the number of all parameters.
 *
 * \param szName		[in] A case-insensitive parameter name to find.
 *							 The default is NULL.
 * \return				The number of parameters.
 */
template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::GetParamCount (PCSZ szName)
	throw ()
{
	return m_mapParam.Count (szName) ;
}



/*!
 * This method sets parameters for the ::InternetOpen function which is called internally before sending a request.
 * All parameters passed to this method is forwarded to the ::InternetOpen function.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * The following code snippet demonstrates the usage of this method.
 * \code
...
CHttpClient					objHttpReq ;

try {
	// Changes the HTTP user agent.
	objHttpReq.SetInternet (_T ("My Custom User Agent")) ;
	...

} catch (httpclientexception & e) {
	...		// Place error handling codes here.
}
 * \endcode
 *
 * \param szUserAgent		[in] A User Agent which corresponds to the lpszAgent parameter of the ::InternetOpen function.
 * \param dwAccessType		[in] A type of access which corresponds to the dwAccessType parameter of the ::InternetOpen function.
 * \param szProxyName		[in] A name of a proxy server which corresponds to the lpszProxyName parameter of the ::InternetOpen function.
 * \param szProxyBypass		[in] A proxy bypass list which corresponds to the lpszProxyBypass parameter of the ::InternetOpen function.
 * \param dwFlags			[in] Options which corresponds to the dwFlags parameter of the ::InternetOpen function.
 * \throw					Throws a httpclientexception if an error occurs.
 *
 * \sa		::InternetOpen
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetInternet (PCSZ szUserAgent, DWORD dwAccessType
													, PCSZ szProxyName, PCSZ szProxyBypass
													, DWORD dwFlags)
	throw (Exception &)
{
	PSZ			szNewUserAgent = NULL ;
	PSZ			szNewProxyName = NULL ;
	PSZ			szNewProxyBypass = NULL ;

	try {
		szNewUserAgent = _AllocNCopyIfNewString (szUserAgent, m_szInternetUserAgent) ;
		szNewProxyName = _AllocNCopyIfNewString (szProxyName, m_szInternetProxyName) ;
		szNewProxyBypass = _AllocNCopyIfNewString (szProxyBypass, m_szInternetProxyBypass) ;
	} catch (Exception &) {
		if ( m_szInternetUserAgent != szNewUserAgent )		SAFEFREE (szNewUserAgent) ;
		if ( m_szInternetProxyName != szNewProxyName )		SAFEFREE (szNewProxyName) ;
		if ( m_szInternetProxyBypass != szNewProxyBypass )	SAFEFREE (szNewProxyBypass) ;
		throw ;
	}

	if ( m_szInternetUserAgent != szNewUserAgent ) {
		SAFEFREE (m_szInternetUserAgent) ;
		m_szInternetUserAgent = szNewUserAgent ;
	}

	if ( m_szInternetProxyName != szNewProxyName ) {
		SAFEFREE (m_szInternetProxyName) ;
		m_szInternetProxyName = szNewProxyName ;
	}

	if ( m_szInternetProxyBypass != szNewProxyBypass ) {
		SAFEFREE (m_szInternetProxyBypass) ;
		m_szInternetProxyBypass = szNewProxyBypass ;
	}

	m_dwInternetAccessType = dwAccessType ;
	m_dwInternetFlags = dwFlags ;
}

/*!
 * This method sets the lpszAgent parameter for the ::InternetOpen function which is called internally before sending a request.
 * The parameter passed to this method is forwarded to the ::InternetOpen function.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \param szUserAgent		[in] A User Agent which corresponds to the lpszAgent parameter of the ::InternetOpen function.
 *								 If this is NULL, The default is used.
 * \throw					Throws a httpclientexception if an error occurs.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetInternetUserAgent (PCSZ szUserAgent)
	throw (Exception &)
{
	PSZ			szNewUserAgent = NULL ;

	try {
		szNewUserAgent = _AllocNCopyIfNewString (szUserAgent, m_szInternetUserAgent) ;
	} catch (Exception &) {
		if ( m_szInternetUserAgent != szNewUserAgent )		SAFEFREE (szNewUserAgent) ;
		throw ;
	}

	if ( m_szInternetUserAgent != szNewUserAgent ) {
		SAFEFREE (m_szInternetUserAgent) ;
		m_szInternetUserAgent = szNewUserAgent ;
	}
}

/*!
 * This method returns the lpszAgent parameter for the ::InternetOpen function which is called internally before sending a request.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \return					A User Agent which corresponds to the lpszAgent parameter of the ::InternetOpen function.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetInternetUserAgent (void)
	throw ()
{
	return m_szInternetUserAgent ? m_szInternetUserAgent : HttpTool::szDefUsrAgent ;
}


/*!
 * This method sets the dwAccessType parameter for the ::InternetOpen function which is called internally before sending a request.
 * The parameter passed to this method is forwarded to the ::InternetOpen function.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \param dwAccessType		[in] A type of access which corresponds to the dwAccessType parameter of the ::InternetOpen function.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetInternetAccessType (DWORD dwAccessType)
	throw ()
{
	m_dwInternetAccessType = dwAccessType ;
}

/*!
 * This method returns the dwAccessType parameter for the ::InternetOpen function which is called internally before sending a request.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \return					A type of access which corresponds to the dwAccessType parameter of the ::InternetOpen function.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::GetInternetAccessType (void)
	throw ()
{
	return m_dwInternetAccessType ;
}

/*!
 * This method sets the lpszProxyName parameter for the ::InternetOpen function which is called internally before sending a request.
 * The parameter passed to this method is forwarded to the ::InternetOpen function.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \param szProxyName		[in] A proxy server name which corresponds to the lpszProxyName parameter of the ::InternetOpen function.
 * \throw					Throws a httpclientexception if an error occurs.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetInternetProxyName (PCSZ szProxyName)
	throw (Exception &)
{
	PSZ			szNewProxyName = NULL ;

	try {
		szNewProxyName = _AllocNCopyIfNewString (szProxyName, m_szInternetProxyName) ;
	} catch (Exception &) {
		if ( m_szInternetProxyName != szNewProxyName )		SAFEFREE (szNewProxyName) ;
		throw ;
	}

	if ( m_szInternetProxyName != szNewProxyName ) {
		SAFEFREE (m_szInternetProxyName) ;
		m_szInternetProxyName = szNewProxyName ;
	}
}

/*!
 * This method returns the lpszProxyName parameter for the ::InternetOpen function which is called internally before sending a request.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \return					A proxy server name which corresponds to the lpszProxyName parameter of the ::InternetOpen function.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetInternetProxyName (void)
	throw ()
{
	return m_szInternetProxyName ;
}


/*!
 * This method sets the lpszProxyBypass parameter for the ::InternetOpen function which is called internally before sending a request.
 * The parameter passed to this method is forwarded to the ::InternetOpen function.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \param szProxyBypass		[in] A proxy bypass list which corresponds to the lpszProxyBypass parameter of the ::InternetOpen function.
 * \throw					Throws a httpclientexception if an error occurs.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetInternetProxyBypass (PCSZ szProxyBypass)
	throw (Exception &)
{
	PSZ			szNewProxyBypass = NULL ;

	try {
		szNewProxyBypass = _AllocNCopyIfNewString (szProxyBypass, m_szInternetProxyBypass) ;
	} catch (Exception &) {
		if ( m_szInternetProxyBypass != szNewProxyBypass )	SAFEFREE (szNewProxyBypass) ;
		throw ;
	}

	if ( m_szInternetProxyBypass != szNewProxyBypass ) {
		SAFEFREE (m_szInternetProxyBypass) ;
		m_szInternetProxyBypass = szNewProxyBypass ;
	}
}

/*!
 * This method returns the lpszProxyBypass parameter for the ::InternetOpen function which is called internally before sending a request.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \return					A proxy bypass list which corresponds to the lpszProxyBypass parameter of the ::InternetOpen function.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetInternetProxyBypass (void)
	throw ()
{
	return m_szInternetProxyBypass ;
}

/*!
 * This method sets the dwFlags parameter for the ::InternetOpen function which is called internally before sending a request.
 * The parameter passed to this method is forwarded to the ::InternetOpen function.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \param dwFlags			[in] Options which corresponds to the dwFlags parameter of the ::InternetOpen function.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetInternetFlags (DWORD dwFlags)
	throw ()
{
	m_dwInternetFlags = dwFlags ;
}

/*!
 * This method returns the dwFlags parameter for the ::InternetOpen function which is called internally before sending a request.
 * For more infomation about the ::InternetOpen function, see the MSDN documentation.
 *
 * \return					Options which corresponds to the dwFlags parameter of the ::InternetOpen function.
 *
 * \sa		::InternetOpen, SetInternet
 */
template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::GetInternetFlags (void)
	throw ()
{
	return m_dwInternetFlags ;
}


/*!
 * This method returns the number of characters required to make a URL for a HTTP 
 * GET request. If you want to pass some parameters to a web server using HTTP GET,
 * you have to append parameters to the requested URL. This method calculates 
 * the length of the URL for that situation. If the CHttpClient does not contain any
 * HTTP parameters, it returns the length of the szUrl. Otherwise it returns
 * the calculated length. It does not check whether the szUrl is valid.
 * If the szUrl is NULL, it returns the length of the appended parameters.
 * (Not including the beginning '?' character)
 * The returned value does not include the terminating NULL character.
 *
 * \param szUrl		[in] An URL which is used to calculate the length of the GET URL.
 * \return			The number of characters required. (Not including the terminating NULL character)
 * \throw			Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool, typename HttpEncoder>
DWORD CHttpClientT<HttpTool, HttpEncoder>::MakeGetUrlLen (PCSZ szUrl)
	throw (Exception &)
{
	// Calculates the number of required characters
	::SafeInt<DWORD>		cchUrlForGet = 0 ;

	try {
		cchUrlForGet = szUrl ? HttpTool::StringLen (szUrl) : 0 ;

		if ( m_mapParam.Empty () )
			return cchUrlForGet.Value () ;

		for (ConstMapIter iter = m_mapParam.Begin (); iter != m_mapParam.End (); ++iter) {
			// If the parameter's name is a URL-encoded string
			if ( (iter->second).dwFlag & ParamEncodedName )
				cchUrlForGet += (iter->first ? HttpTool::StringLen (iter->first) : 0) ;
			else
				cchUrlForGet += _UrlEncodeLen (iter->first) ;

			// If the parameter's value is a URL-encoded string
			if ( (iter->second).dwFlag & ParamEncodedValue )
				cchUrlForGet += ((iter->second).szValue ? HttpTool::StringLen ((iter->second).szValue) : 0) ;
			else
				cchUrlForGet += _UrlEncodeLen ((iter->second).szValue) ;

			cchUrlForGet += 2 ;		// for ('?' or '&') and '='
		}

		// If the szUrl is NULL, it does not insert '?' or '&' character
		// which divides the GET URL into the szUrl and HTTP parameters.
		if ( szUrl == NULL && m_mapParam.Count () >= 0 )
			cchUrlForGet-- ;

		// If the URL does not contain the path (ex. 'http://www.kw.ac.kr')
		// we have to append '/' to the URL.
		// It does not care whether the server address exists.
		// (It does not check the URL)
		if ( szUrl ) {
			CHttpUrlAnalyzer	objAnalyzer (szUrl, m_nAnsiCodePage) ;

			if ( objAnalyzer.PathLen () == 0 )
				cchUrlForGet++ ;
		}
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	return cchUrlForGet.Value () ;
}

/*!
 * This method returns a URL for a HTTP GET request.
 * If you want to pass some parameters to a web server using HTTP GET,
 * you have to append parameters to the requested URL. This method creates a URL for that situation. 
 * If the CHttpClient does not contain any HTTP parameters, it returns the szUrl.
 * It does not check whether the szUrl is valid.
 * If the szUrl is NULL, it copies only the appended parameters.
 * (Not including the beginning '?' character)
 *
 * \param szBuff	[out] A buffer to save the GET URL. The buffer can not be NULL.
 * \param szUrl		[in] An URL which is used to make a URL for a HTTP GET request.
 * \return			A generated URL.
 * \throw			Throws a httpclientexception if an error occurs.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PSZ 
CHttpClientT<HttpTool, HttpEncoder>::MakeGetUrl (PSZ szBuff, PCSZ szUrl)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (szBuff != NULL, "CHttpClientT::MakeGetUrl: szBuff can not be NULL.") ;

	if ( m_mapParam.Empty () ) {
		if ( szUrl == NULL ) {
			szBuff[0] = '\0' ;
			return szBuff ;
		}

		HttpTool::StringCopy (szBuff, szUrl) ;
		return szBuff ;
	}

	PSZ					pchBuff = szBuff ;
	DWORD				i ;
	CHttpUrlAnalyzer	objAnalyzer (szUrl, m_nAnsiCodePage) ;

	if ( szUrl ) {
		for (i = 0; i < objAnalyzer.PathIdx (); *(pchBuff++) = szUrl[i], i++) ;

		// If the path does not exist, it is needed to append '/' to the server address.
		if ( objAnalyzer.PathLen () == 0 )
			*(pchBuff++) = '/' ;

		// Appends the path and the search string.
		for (i = objAnalyzer.PathIdx (); i < objAnalyzer.BookmarkIdx (); *(pchBuff++) = szUrl[i], i++) ;

		// Copies '?' or '&'
		*(pchBuff++) = objAnalyzer.SearchLen () == 0 ? '?' : '&' ;
	}

	// Copies the HTTP parameters
	for (ConstMapIter iter = m_mapParam.Begin (); iter != m_mapParam.End (); ++iter) {
		if ( iter != m_mapParam.Begin () )
			*(pchBuff++) = '&' ;

		// If the parameter's name is a URL-encoded string
		if ( (iter->second).dwFlag & ParamEncodedName )
			HttpTool::StringCopy (pchBuff
				, iter->first ? iter->first : HttpTool::szEmptyString) ;
		else
			_UrlEncode (pchBuff, iter->first) ;

		for (; *pchBuff != '\0'; pchBuff++) ;

		*(pchBuff++) = '=' ;

		// If the parameter's value is a URL-encoded string
		if ( (iter->second).dwFlag & ParamEncodedValue )
			HttpTool::StringCopy (pchBuff
				, (iter->second).szValue ? (iter->second).szValue : HttpTool::szEmptyString) ;
		else
			_UrlEncode (pchBuff, (iter->second).szValue) ;

		for (; *pchBuff != '\0'; pchBuff++) ;
	}

	// Appends the bookmark
	for (i = objAnalyzer.BookmarkIdx (); i < objAnalyzer.BookmarkIdx () + objAnalyzer.BookmarkLen (); *(pchBuff++) = szUrl[i], i++) ;
	*pchBuff = '\0' ;

	return szBuff ;
}

/*!
 * This method returns a new HINTERNET handle which is returned by ::InternetOpen
 * using the settings saved in the CHttpClient.
 * The caller is responsible for closing the returned handle when it has finished with it.
 *
 * \return			A new HINTERNET handle.
 * \throw			Throws a httpclientexception if an error occurs.
 *
 * \sa	::InternetOpen
 */
template <typename HttpTool, typename HttpEncoder>
HINTERNET CHttpClientT<HttpTool, HttpEncoder>::OpenInternet (void)
	throw (Exception &)
{
	return HttpTool::OpenInternet (
		GetInternetUserAgent (), GetInternetAccessType (), GetInternetProxyName (),
		GetInternetProxyBypass (), GetInternetFlags ()) ;
}

/*!
 * This method returns a new HINTERNET handle which is returned by ::InternetConnect.
 * It only checks the protocol, server address and server port of the URL.
 * So you can use the following URLs.
 * <UL>
 *	<LI>http://www.k-net.or.kr:80/member.html?type=5 -> A full HTTP URL (or a full HTTPS URL)</LI>
 *	<LI>http://club.hooriza.com:80 -> A HTTP URL without path (or a HTTPS URL without path)</LI>
 *	<LI>wedding.makeself.net:80 -> Only a server location (http is assumed)</LI>
 *	<LI>www.kw.ac.kr/index.html -> A URL without protocol (http is assumed)</LI>
 *	<LI>ftp://knews.k-net.or.kr -> An URL with another protocol (http is assumed)</LI>
 * </UL>
 *
 * The caller is responsible for closing the returned handle when it has finished with it.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen.
 * \param szUrl			[in] An URL which is used to return a new handle.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A new HINTERNET handle.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa	::InternetConnect
 */
template <typename HttpTool, typename HttpEncoder>
HINTERNET CHttpClientT<HttpTool, HttpEncoder>::OpenConnection (HINTERNET hInternet, PCSZ szUrl, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (hInternet != NULL, "CHttpClientT::OpenConnection: hInternet can not be NULL.") ;

	INTERNET_PORT		nPort = INTERNET_DEFAULT_HTTP_PORT ;
	CHttpUrlAnalyzer	objAnalyzer (szUrl, m_nAnsiCodePage) ;

	// If the protocol is HTTPS, the default port is INTERNET_DEFAULT_HTTPS_PORT.
	if ( (objAnalyzer.ProtocolLen () == 5 /* ::strlen ("HTTPS") */)
			&& (0 == HttpTool::StringNICmp (
				szUrl + objAnalyzer.ProtocolIdx (), HttpTool::szHTTPS, objAnalyzer.ProtocolLen ())) )
			nPort = INTERNET_DEFAULT_HTTPS_PORT ;

	// Get the server port
	if ( objAnalyzer.PortLen () > 0 ) {
		// The port number can not be greater than 65535
		if ( objAnalyzer.PortLen () > 5 )
			HttpTool::ThrowException (HTTPCLIENT_ERR_INTERNET_PORT_NOT_VALID) ;

		CharType		szPort[6] ;
		HttpTool::StringNCopy (szPort, szUrl + objAnalyzer.PortIdx (), objAnalyzer.PortLen ()) ;
		szPort[objAnalyzer.PortLen ()] = '\0' ;

		unsigned long	ulPort ;
		PSZ				pEndPtr ;
		ulPort = HttpTool::StringToUL (szPort, &pEndPtr, 10) ;
		if ( *pEndPtr != NULL || ulPort > 65535 )
			HttpTool::ThrowException (HTTPCLIENT_ERR_INTERNET_PORT_NOT_VALID) ;
		nPort = static_cast<INTERNET_PORT> (ulPort) ;
	}

	// Get the server address
	if ( objAnalyzer.AddressLen () == 0 )
		HttpTool::ThrowException (HTTPCLIENT_ERR_INVALID_URL) ;

	::SafeInt<size_t>	cbAddress = 0 ;
	try {
		cbAddress = objAnalyzer.AddressLen () ;
		cbAddress += 1 ;
		cbAddress *= sizeof (CharType) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	PSZ				szServerAddr = (PSZ) ::malloc (cbAddress.Value ()) ;
	if ( szServerAddr == NULL )
		HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	HttpTool::StringNCopy (szServerAddr, szUrl + objAnalyzer.AddressIdx (), objAnalyzer.AddressLen ()) ;
	szServerAddr[objAnalyzer.AddressLen ()] = '\0' ;

	HINTERNET		hConnection = NULL ;
	try {
		hConnection = HttpTool::OpenConnection (hInternet, szServerAddr, nPort, szUsrName, szUsrPwd) ;
		// Applys proxy server account
		ApplyProxyAccount (hConnection) ;
	} catch (Exception &) {
		SAFEFREE (szServerAddr) ;
		HttpTool::CloseConnection (hConnection) ;
		throw ;
	}

	SAFEFREE (szServerAddr) ;
	return hConnection ;
}

/*!
 * This method sets a user name and a password which are used to access HTTP proxy server.
 * If the szUserName parameter is NULL, the proxy account saved in CHttpClient will be removed.
 * You can handle proxy authorization using this method, but there are some restrictions.
 * First, The HTTP POST is not safe if you are not an authorized user.
 * (It is possible that the proxy server closes the connection before the request is finished).
 * Second, Only the HTTP GET can be used to request proxy authorization.
 * (This restriction is caused by the WinInet API).
 * 
 * The following code snippet demonstrates the usage of this method.
 * \code
...
CHttpClient			objHttpReq ;
CHttpResponse *		pobjHttpRes = NULL ;

try {
	...		// Initialize the CHttpClient object

	// It is safe to use HTTP GET method.
    pobjHttpRes = objHttpReq.RequestGet (...) ;

	// If the server requests proxy authorization..
	if ( pobjHttpRes->GetStatus () == HTTP_STATUS_PROXY_AUTH_REQ ) {
		delete pobjHttpRes ;
		pobjHttpRes = NULL ;
	
		LPTSTR			szUserName = NULL ;
		LPTSTR			szPassword = NULL ;

		...		// Get a proxy user name and password

		// Set the proxy user name and password
		objHttpReq.SetProxyAccount (szUserName, szPassword) ;

		// Only a HTTP GET method can be used to request proxy authorization.
		pobjHttpRes = objHttpReq.RequestGet (...) ;

		if ( pobjHttpRes->GetStatus () == HTTP_STATUS_PROXY_AUTH_REQ ) {
			...		// If the authorization fails, try again or stop
		}
	}

	...		// Continue to execute

} catch (httpclientexception & e) {
	...		// Place error handling codes here.
}
 * \endcode
 *
 * \param szUserName	[in] A proxy user name. Empty string is not allowed.
 *							 If it is NULL, account information saved in CHttpClient will be removed.
 * \param szPassword	[in] A proxy password. NULL and empty string are not allowed.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa	ApplyProxyAccount, ::InternetSetOption, <a href="http://support.microsoft.com/?kbid=195650" target="_blank">How To Handle Proxy Authorization with WinInet</a>
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::SetProxyAccount (PCSZ szUserName, PCSZ szPassword)
	throw (Exception &)
{
	if ( szUserName == NULL ) {
		SAFEFREE (m_szProxyUserName) ;
		SAFEFREE (m_szProxyPassword) ;
		return ;
	}

	HTTPCLIENT_ASSERT (szUserName != NULL, "CHttpClientT::SetProxyAccount: szUserName can not be NULL.") ;
	HTTPCLIENT_ASSERT (HttpTool::StringLen (szUserName) > 0, "CHttpClientT::SetProxyAccount: szUserName can not be an empty string.") ;
	HTTPCLIENT_ASSERT (szPassword != NULL, "CHttpClientT::SetProxyAccount: szPassword can not be NULL.") ;
	HTTPCLIENT_ASSERT (HttpTool::StringLen (szPassword) > 0, "CHttpClientT::SetProxyAccount: szPassword can not be an empty string.") ;

	PSZ		szNewUserName = NULL ;
	PSZ		szNewPassword = NULL ;

	try {
		szNewUserName = _AllocNCopyIfNewString (szUserName, m_szProxyUserName) ;
		szNewPassword = _AllocNCopyIfNewString (szPassword, m_szProxyPassword) ;
	} catch (Exception &) {
		if ( m_szProxyUserName != szNewUserName )	SAFEFREE (szNewUserName) ;
		if ( m_szProxyPassword != szNewPassword )	SAFEFREE (szNewPassword) ;
		throw ;
	}

	if ( m_szProxyUserName != szNewUserName ) {
		SAFEFREE (m_szProxyUserName) ;
		m_szProxyUserName = szNewUserName ;
	}

	if ( m_szProxyPassword != szNewPassword ) {
		SAFEFREE (m_szProxyPassword) ;
		m_szProxyPassword = szNewPassword ;
	}
}

/*!
 * This method returns a proxy user name which is used to access HTTP proxy server.
 * It always returns a null-terminated string. The returned string is owned by
 * the CHttpClientT. So you can not free it.
 *
 * \return				A proxy user name.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetProxyUserName (void) const
	throw ()
{
	return m_szProxyUserName ? m_szProxyUserName : HttpTool::szEmptyString ;
}

/*!
 * This method returns the current proxy password.
 * It always returns a null-terminated string. The returned string is owned by
 * the CHttpClientT. So you can not free it.
 *
 * \return				The current proxy password.
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ
CHttpClientT<HttpTool, HttpEncoder>::GetProxyPassword (void) const
	throw ()
{
	return m_szProxyPassword ? m_szProxyPassword : HttpTool::szEmptyString ;
}

/*!
 * This method sets a proxy user name and password on the hConnection handle
 * using the settings saved in CHttpClient.
 * You can handle proxy authorization using this handle, but there are some restrictions.
 * First, The HTTP POST method is not safe if you are not authorized
 * (It is possible that the proxy server closes the connection before the request is finished).
 * Second, Only a HTTP GET method can be used to request proxy authorization
 * (This restriction is caused by the WinInet API).
 * If an exception occurs, it is possible that the proxy user name is only applied.
 * 
 * The following code snippet demonstrates the usage of this method.
 * \code
...
CHttpClient			objHttpReq ;
CHttpResponse *		pobjHttpRes = NULL ;
HINTERNET			hInternet = NULL ;
HINTERNET			hConnection = NULL ;

try {
	...		// Initialize the CHttpClient object

	// Get a new internet handle
	hInternet = objHttpReq.OpenInternet () ;

	// Get a new connection handle
	hConnection = objHttpReq.OpenConnection (hInternet, _T ("http://www.k-net.or.kr")) ;

	// It is safe to use HTTP GET method.
    pobjHttpRes = objHttpReq.RequestGet (hInternet, hConnection, ...) ;

	// If the server requests proxy authorization..
	if ( pobjHttpRes->GetStatus () == HTTP_STATUS_PROXY_AUTH_REQ ) {
		delete pobjHttpRes ;
		pobjHttpRes = NULL ;
	
		LPTSTR			szUserName = NULL ;
		LPTSTR			szPassword = NULL ;

		...		// Get a proxy user name and password

		// Set the proxy user name and password
		objHttpReq.SetProxyAccount (szUserName, szPassword) ;
		objHttpReq.ApplyProxyAccount (hConnection) ;

		// Only a HTTP GET method can be used to request proxy authorization.
		pobjHttpRes = objHttpReq.RequestGet (hInternet, hConnection, ...) ;

		if ( pobjHttpRes->GetStatus () == HTTP_STATUS_PROXY_AUTH_REQ ) {
			...		// If the authorization fails, try again or stop
		}
	}

	...		// Continue to execute

} catch (httpclientexception & e) {
	...		// Place error handling codes here.
}
 * \endcode
 *
 * \param hConnection	[in] A HINTERNET handle returned by ::InternetConnect function.
 *							 NULL is not allowed.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa	SetProxyAccount, ::InternetSetOption, <a href="http://support.microsoft.com/?kbid=195650" target="_blank">How To Handle Proxy Authorization with WinInet</a>
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::ApplyProxyAccount (HINTERNET hConnection)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (hConnection != NULL, "CHttpClientT::ApplyProxyAccount: hConnection can not be NULL.") ;

	if ( m_szProxyUserName == NULL || m_szProxyPassword == NULL )
		return ;

	::SafeInt<DWORD>	cchProxyUserName ;
	::SafeInt<DWORD>	cchProxyPassword ;

	try {
		cchProxyUserName = HttpTool::StringLen (m_szProxyUserName) ;
		cchProxyPassword = HttpTool::StringLen (m_szProxyPassword) ;
	} catch (::SafeIntException & e) {
		throw (e) ;
	}

	if ( cchProxyUserName == 0 || cchProxyPassword == 0 )
		return ;

	// Set a user name for Proxy
	HttpTool::InternetSetOption (
			hConnection
			, INTERNET_OPTION_PROXY_USERNAME
			, (LPVOID) m_szProxyUserName
			, cchProxyUserName.Value ()
		) ;
	
	// Set a password for Proxy
	HttpTool::InternetSetOption (
			hConnection
			, INTERNET_OPTION_PROXY_PASSWORD
			, (LPVOID) m_szProxyPassword
			, cchProxyPassword.Value ()
		) ;
}

/*!
 * This method returns a new HINTERNET handle which is returned by ::HttpOpenRequest
 * using the settings saved in the CHttpClient.
 * It only checks the protocol and the path of the URL.
 * So you can use the following URLs.
 * <UL>
 *	<LI>http://drama.kbs.co.kr/winter/people/cast_02.shtml -> A full HTTP URL (or a full HTTPS URL)</LI>
 *	<LI>http:///winter/people/cast_02.shtml -> A HTTP URL without server location (or a HTTPS URL without server location)</LI>
 *	<LI>www.eggfilm.com/classic/intro.htm -> A HTTP URL without protocol (http is assumed)</LI>
 *	<LI>/classic/intro.htm -> Only A path (http is assumed)</LI>
 *	<LI>http://www.k-net.or.kr -> An HTTP URL without path (uses "/" as the path)</LI>
 *	<LI>www.k-net.or.kr -> Only A server location (http is assumed, uses "/" as the path)</LI>
 *	<LI> -> A NULL or an empty string (http is assumed, uses "/" as the path)</LI>
 *	<LI>ftp://www.k-net.or.kr/member.html -> An URL with another protocol (http is assumed)</LI>
 * </UL>
 * 
 * The caller is responsible for closing the returned handle when it has finished with it.
 *
 * \param hConnection	[in] A valid handle returned by ::InternetConnect.
 * \param szVerb		[in] A HTTP verb which corresponds to the lpszVerb parameter of the ::HttpOpenRequest function.
 * \param szUrl			[in] An URL which is used to return a new handle.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \return				A new HINTERNET handle.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa	::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
HINTERNET CHttpClientT<HttpTool, HttpEncoder>::OpenRequest (HINTERNET hConnection
																  , PCSZ szVerb, PCSZ szUrl
																  , DWORD dwFlags, PCSZ szReferer)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (hConnection != NULL, "CHttpClientT::OpenRequest: hConnection can not be NULL.") ;

	CHttpUrlAnalyzer	objAnalyzer (szUrl, m_nAnsiCodePage) ;

	// If the protocol is HTTPS, adds the SSL flag.
	if ( (objAnalyzer.ProtocolLen () == 5 /* ::strlen ("HTTPS") */)
			&& (0 == HttpTool::StringNICmp (
				szUrl + objAnalyzer.ProtocolIdx (), HttpTool::szHTTPS, objAnalyzer.ProtocolLen ())) )
			dwFlags |= INTERNET_FLAG_SECURE ;

	PCSZ		szUrlPath ;
	BOOL		bNeedToFreeUrlPath = FALSE ;

	// If the path does not exist, use "/" as the path
	if ( objAnalyzer.PathLen () == 0 ) {
		size_t		cchAfterPath = szUrl ? HttpTool::StringLen (szUrl + objAnalyzer.PathIdx ()) : 0 ;

		if ( cchAfterPath == 0 )
			szUrlPath = HttpTool::szSlash ;
		else {

			::SafeInt<size_t>	cbAfterPath = 0 ;
			try {
				cbAfterPath = cchAfterPath ;
				cbAfterPath += 2 ;			// for '/' & '\0'
				cbAfterPath *= sizeof (CharType) ;
			} catch (::SafeIntException & e) {
				HttpTool::ThrowException (e) ;
			}

			szUrlPath = (PCSZ) ::malloc (cbAfterPath.Value ()) ;
			if ( szUrlPath == NULL )
				HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
			bNeedToFreeUrlPath = TRUE ;

			HttpTool::StringCopy (const_cast<PSZ> (szUrlPath), HttpTool::szSlash) ;
			HttpTool::StringCat (const_cast<PSZ> (szUrlPath), szUrl + objAnalyzer.PathIdx ()) ;
		}
	} else
		szUrlPath = szUrl + objAnalyzer.PathIdx () ;

	HINTERNET		hRequest = NULL ;

	try {
		hRequest = HttpTool::OpenRequest (hConnection, szVerb, szUrlPath, dwFlags, szReferer, m_nAnsiCodePage) ;

		// Adds the cache-control header if it is needed
		if ( dwFlags & INTERNET_FLAG_PRAGMA_NOCACHE )
			HttpTool::AddHeader (hRequest, HttpTool::szCacheControl, HttpTool::szNoCache, m_nAnsiCodePage) ;

	} catch (Exception &) {
		HttpTool::CloseRequest (hRequest) ;
		if ( bNeedToFreeUrlPath )
			SAFEFREE (szUrlPath) ;
		throw ;
	}

	if ( bNeedToFreeUrlPath )
		SAFEFREE (szUrlPath) ;

	return hRequest ;
}

/*!
 * This method adds the HTTP headers to the handle which is returned by ::HttpOpenRequest
 * using the settings saved in the CHttpClient.
 * It uses the headers which is saved in the CHttpClient.
 * If an exception occurs, it is possible that the headers are added partially.
 *
 * \param hRequest	[in] A valid handle which is returned by ::HttpOpenRequest.
 * \throw			Throws a httpclientexception if an error occurs.
 *
 * \sa		::HttpAddRequestHeaders
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::AddRequestHeader (HINTERNET hRequest)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (hRequest != NULL, "CHttpClientT::AddRequestHeader: hRequest can not be NULL.") ;

	// Adds user's headers.
	for (ConstMapIter iter = m_mapHeader.Begin (); iter != m_mapHeader.End (); ++iter)
		HttpTool::AddHeader (hRequest, iter->first, (iter->second).szValue, m_nAnsiCodePage) ;
}

/*!
 * \internal
 * This method retrieves the resource specified by the szUrl using HTTP GET request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::_RequestGetEx (HINTERNET hInternet, HINTERNET hConnection
														 , PCSZ szUrl, DWORD dwFlags, PCSZ szReferer
														 , PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	// Releases any existing Post Context
	_EndPostContext () ;

	PCSZ		szGetUrl = NULL ;
	BOOL		bNeedToFreeGetUrl = FALSE ;

	BOOL		bBorrowedInternet = TRUE ;
	BOOL		bBorrowedConnection = TRUE ;
	HINTERNET	hRequest = NULL ;

	CHttpResponse *		pobjRes = NULL ;

	try {

		// Get WinInet API handles
		if ( hInternet == NULL ) {
			hInternet = OpenInternet () ;
			bBorrowedInternet = FALSE ;
		}

		if ( hConnection == NULL ) {
			hConnection = OpenConnection (hInternet, szUrl, szUsrName, szUsrPwd) ;
			bBorrowedConnection = FALSE ;
		}

		szGetUrl = szUrl ;

		// Appends the GET parameters to the URL
		if ( !m_mapParam.Empty () ) {
			::SafeInt<DWORD>	cbGetUrl ;

			try {
				cbGetUrl = MakeGetUrlLen (szUrl) ;
				cbGetUrl += 1 ;
				cbGetUrl *= sizeof (CharType) ;
			} catch (::SafeIntException & e) {
				HttpTool::ThrowException (e) ;
			}

			if ( NULL == (szGetUrl = (PCSZ) ::malloc (cbGetUrl.Value ())) )
				HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

			bNeedToFreeGetUrl = TRUE ;
			MakeGetUrl (const_cast<PSZ> (szGetUrl), szUrl) ;
		}

		hRequest = OpenRequest (hConnection, HttpTool::szGET, szGetUrl, dwFlags, szReferer) ;
		AddRequestHeader (hRequest) ;			// Adds user's custom header

		if ( bNeedToFreeGetUrl ) {
			SAFEFREE (szGetUrl) ;
			bNeedToFreeGetUrl = FALSE ;
		}

		// Connect to the HTTP server
		HttpTool::SendRequest (hRequest, NULL, m_nAnsiCodePage) ;

		if ( NULL == (pobjRes = new CHttpResponse (
											bBorrowedInternet ? NULL : hInternet
											, bBorrowedConnection ? NULL : hConnection
											, hRequest)) )
			HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

	} catch (Exception &) {
		if ( bNeedToFreeGetUrl )	SAFEFREE (szGetUrl) ;
		HttpTool::CloseRequest (hRequest) ;
		if ( !bBorrowedConnection )	HttpTool::CloseRequest (hConnection) ;
		if ( !bBorrowedInternet )	HttpTool::CloseRequest (hInternet) ;
		throw ;
	} catch (...) {			// for the exception thrown by the new operator
		if ( bNeedToFreeGetUrl )	SAFEFREE (szGetUrl) ;
		HttpTool::CloseRequest (hRequest) ;
		if ( !bBorrowedConnection )	HttpTool::CloseRequest (hConnection) ;
		if ( !bBorrowedInternet )	HttpTool::CloseRequest (hInternet) ;
		HttpTool::ThrowException (HTTPCLIENT_ERR_UNEXPECTED_ERROR) ;
	}

	return pobjRes ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP GET request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestGet (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestGetEx (hInternet, hConnection, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP GET request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, ::InternetOpen
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse *
CHttpClientT<HttpTool, HttpEncoder>::RequestGet (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestGetEx (hInternet, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP GET request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestGet (PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestGetEx (szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP GET request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestGetEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags
														, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	return _RequestGetEx (hInternet, hConnection, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP GET request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestGetEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags
														 , PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	return _RequestGetEx (hInternet, NULL, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP GET request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestGetEx (PCSZ szUrl, DWORD dwFlags, PCSZ szReferer
																	 , PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	return _RequestGetEx (NULL, NULL, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;
}

/*!
 * This method starts a new HTTP POST request. If you call this method, you can retrieve progress
 * information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginPost (HINTERNET hInternet, HINTERNET hConnection
																			, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	BeginPostEx (hInternet, hConnection, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method starts a new HTTP POST request. If you call this method, you can retrieve progress
 * information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		OpenInternet, ::InternetOpen
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginPost (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	BeginPostEx (hInternet, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method starts a new HTTP POST request. If you call this method, you can retrieve progress
 * information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginPost (PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	BeginPostEx (szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method starts a new HTTP POST request. If you call this method, you can retrieve progress
 * information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginPostEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags
																	, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	_StartPostContext (hInternet, hConnection, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;	// Starts a new POST context
}

/*!
 * This method starts a new HTTP POST request. If you call this method, you can retrieve progress
 * information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed, OpenInternet, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginPostEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags, PCSZ szReferer
																	, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	_StartPostContext (hInternet, NULL, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;	// Starts a new POST context
}

/*!
 * This method starts a new HTTP POST request. If you call this method, you can retrieve progress
 * information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginPostEx (PCSZ szUrl, DWORD dwFlags, PCSZ szReferer
																	, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	_StartPostContext (NULL, NULL, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;	// Starts a new POST context
}

/*!
 * This method starts a new HTTP UPLOAD request which is a HTTP POST request with another content-type (multipart/form-data).
 * If you call this method, you can retrieve progress information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginUpload (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	BeginUploadEx (hInternet, hConnection, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method starts a new HTTP UPLOAD request which is a HTTP POST request with another content-type (multipart/form-data).
 * If you call this method, you can retrieve progress information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed, OpenInternet, ::InternetOpen
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginUpload (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	BeginUploadEx (hInternet, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method starts a new HTTP UPLOAD request which is a HTTP POST request with another content-type (multipart/form-data).
 * If you call this method, you can retrieve progress information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginUpload (PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	BeginUploadEx (szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method starts a new HTTP UPLOAD request which is a HTTP POST request with another content-type (multipart/form-data).
 * If you call this method, you can retrieve progress information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginUploadEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl
															, DWORD dwFlags, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	_StartUploadContext (hInternet, hConnection, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;	// Starts a new UPLOAD context
}

/*!
 * This method starts a new HTTP UPLOAD request which is a HTTP POST request with another content-type (multipart/form-data).
 * If you call this method, you can retrieve progress information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed, OpenInternet, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginUploadEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags, PCSZ szReferer
														   , PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	_StartUploadContext (hInternet, NULL, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;	// Starts a new UPLOAD context
}

/*!
 * This method starts a new HTTP UPLOAD request which is a HTTP POST request with another content-type (multipart/form-data).
 * If you call this method, you can retrieve progress information of the POST request by using the Query method.
 * To finish the request, you have to call the Proceed method.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		Query, Proceed, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::BeginUploadEx (PCSZ szUrl, DWORD dwFlags, PCSZ szReferer
																	, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	// Starts a new UPLOAD context
	_StartUploadContext (NULL, NULL, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;
}

/*!
 * This method queries progress information of the current POST context.
 * If you call the BeginPost or the BeginUpload method which starts a new POST context,
 * you can retrieve progress information by using this method.
 * If an exception occurs, the current POST context is automatically canceled.
 *
 * \param objPostStat	[in] A CHttpPostStatT object.
 *
 * \sa		CHttpPostStatT, BeginPost, BeginPostEx, BeginUpload, BeginUploadEx, Cancel, Proceed
 */
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::Query (CHttpPostStat & objPostStat)
	throw ()
{
	if ( _PostContextIsPost () )
		_QueryPostStat (objPostStat) ;
	else
		_QueryUploadStat (objPostStat) ;
}

/*!
 * This method cancels the current POST context which is started by
 * the BeginPost or the BeginUpload method.
 *
 * \return				Whether the operation canceled.
 *
 * \sa		BeginPost, BeginPostEx, BeginUpload, BeginUploadEx, Query, Proceed
 */
template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::Cancel (void)
	throw ()
{
	if ( _PostContextIsPost () )
		return _CancelPostContext () ;

	return _CancelUploadContext () ;
}

/*!
 * This method proceeds with the current POST context which is started by
 * the BeginPost or the BeginUpload method.
 * It transmits data of the current POST context to the web server.
 * If all bytes are transmitted, a pointer to a CHttpResponseT object is returned.
 * The returned pointer is allocated by the new operator.
 * So you have to delete the returned pointer.
 * If an exception occurs, the current POST context is automatically canceled.
 *
 * \param cbDesired		[in] The number of bytes to be transmitted.
 * \return				A pointer to a CHttpResponseT object if all bytes are transmitted,
 *						otherwise NULL.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, BeginPost, BeginPostEx, BeginUpload, BeginUploadEx, Query, Cancel
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::Proceed (DWORD cbDesired)
	throw (Exception &)
{
	if ( _PostContextIsPost () )
		return _ProceedPostContext (cbDesired) ;

	return _ProceedUploadContext (cbDesired) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP POST request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestPost (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestPostEx (hInternet, hConnection, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP POST request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, ::InternetOpen
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestPost (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestPostEx (hInternet, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP POST request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestPost (PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestPostEx (szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP POST request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestPostEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl
													, DWORD dwFlags, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	BeginPostEx (hInternet, hConnection, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;

	CHttpResponse *			pResponse = NULL ;
	while ( !(pResponse = Proceed (::SafeInt<DWORD>::MaxInt ())) ) ;
	return pResponse ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP POST request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestPostEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags
														  , PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	BeginPostEx (hInternet, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;

	CHttpResponse *			pResponse = NULL ;
	while ( !(pResponse = Proceed (::SafeInt<DWORD>::MaxInt ())) ) ;
	return pResponse ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP POST request.
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestPostEx (PCSZ szUrl, DWORD dwFlags, PCSZ szReferer
																, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	BeginPostEx (szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;

	CHttpResponse *			pResponse = NULL ;
	while ( !(pResponse = Proceed (::SafeInt<DWORD>::MaxInt ())) ) ;
	return pResponse ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP UPLOAD request
 * which is a HTTP POST request with another content-type (multipart/form-data).
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestUpload (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestUploadEx (hInternet, hConnection, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP UPLOAD request
 * which is a HTTP POST request with another content-type (multipart/form-data).
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, ::InternetOpen
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestUpload (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestUploadEx (hInternet, szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP UPLOAD request
 * which is a HTTP POST request with another content-type (multipart/form-data).
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param bUseCache		[in] Specifies whether to use cache.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestUpload (PCSZ szUrl, BOOL bUseCache)
	throw (Exception &)
{
	return RequestUploadEx (szUrl
		, bUseCache ? HTTPCLIENT_DEF_REQUEST_FLAGS : HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE) ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP UPLOAD request
 * which is a HTTP POST request with another content-type (multipart/form-data).
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * If the caller passes the hConnection parameter which is returned by ::InternetConnect,
 * the CHttpClient also uses the passed handle instead of creating a new one.
 * In this case, the caller has to pass the hInternet parameter which is used to get the hConnection parameter.
 * The caller is responsible for closing the hConnection handle when it has finished with it.
 * If the hConnection is not NULL, the CHttpClient does not use the server location part in the szUrl.
 * You can omit the server location part of the szUrl.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param hConnection	[in] A valid handle returned by ::InternetConnect. NULL is allowed.
 *						     If this parameter is not NULL, the caller has to pass the hInternet parameter
 *						     which is used to get the hConnection parameter.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, OpenConnection, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestUploadEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	BeginUploadEx (hInternet, hConnection, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;

	CHttpResponse *			pResponse = NULL ;
	while ( !(pResponse = Proceed (::SafeInt<DWORD>::MaxInt ())) ) ;
	return pResponse ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP UPLOAD request
 * which is a HTTP POST request with another content-type (multipart/form-data).
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * If the caller passes the hInternet parameter which is returned by ::InternetOpen,
 * the CHttpClient uses the passed handle instead of creating a new one.
 * The caller is responsible for closing the hInternet handle when it has finished with it.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param hInternet		[in] A valid handle returned by ::InternetOpen. NULL is allowed.
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, OpenInternet, ::InternetOpen, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestUploadEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	BeginUploadEx (hInternet, szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;

	CHttpResponse *			pResponse = NULL ;
	while ( !(pResponse = Proceed (::SafeInt<DWORD>::MaxInt ())) ) ;
	return pResponse ;
}

/*!
 * This method retrieves the resource specified by the szUrl using HTTP UPLOAD request
 * which is a HTTP POST request with another content-type (multipart/form-data).
 * If the szUrl does not start with "https://", "http" is used as the protocol.
 * The returned CHttpResponseT object is allocated by the new operator.
 * So you have to delete the returned CHttpResponseT object.
 *
 * \param szUrl			[in] A HTTP URL.
 * \param dwFlags		[in] A flags which corresponds to the dwFlags parameter of the ::HttpOpenRequest function.
 * \param szReferer		[in] A referer which corresponds to the lpszReferer parameter of the ::HttpOpenRequest function.
 * \param szUsrName		[in] An user name which corresponds to the lpszUsername parameter of the ::InternetConnect function.
 * \param szUsrPwd		[in] An user password which corresponds to the lpszPassword parameter of the ::InternetConnect function.
 * \return				A pointer to a CHttpResponseT object.
 * \throw				Throws a httpclientexception if an error occurs.
 *
 * \sa		CHttpResponseT, ::InternetConnect, ::HttpOpenRequest
 */
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::RequestUploadEx (PCSZ szUrl, DWORD dwFlags, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	BeginUploadEx (szUrl, dwFlags, szReferer, szUsrName, szUsrPwd) ;

	CHttpResponse *			pResponse = NULL ;
	while ( !(pResponse = Proceed (::SafeInt<DWORD>::MaxInt ())) ) ;
	return pResponse ;
}


template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_InitPostContext (void)
	throw ()
{
	m_ahPostedFiles = NULL ;
	m_aszMimeTypes = NULL ;
	m_szPostedValue = NULL ;
	m_bNeedToFreePostedValue = FALSE ;
	m_hInternet = NULL ;
	m_bBorrowedInternet = FALSE ;
	m_hConnection = NULL ;
	m_bBorrowedConnection = FALSE ;
	m_hRequest = NULL ;
		m_hLastReq = NULL; //zenden
	m_bIsPost = TRUE ;
	m_pbyCntxBuff = NULL ;
}

template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::_PostContextIsActive (void) const
	throw ()
{
	return m_objPostStat.IsActive () ;
}

template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::_PostContextIsPost (void) const
	throw ()
{
	return m_bIsPost ;
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_SetPostedValue (LPCSTR szPostedValue, BOOL bNeedToFree)
	throw ()
{
	if ( m_bNeedToFreePostedValue )
		SAFEFREE (m_szPostedValue) ;

	m_szPostedValue = szPostedValue ;
	m_bNeedToFreePostedValue = bNeedToFree ;
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_InitPostCntxBuff (void)
	throw (Exception &)
{
	// If the buffer is aleady allocated
	if ( m_pbyCntxBuff )
		return ;

	if ( NULL == (m_pbyCntxBuff = (BYTE *) ::malloc (HTTPCLIENT_POSTCNTX_BUFF_SIZE)) )
		HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_WritePost (BOOL bUseUtf8, PCSZ szValue, BOOL bIsValue)
	throw (Exception &)
{
	if ( szValue == NULL || szValue[0] == '\0' )
		return ;

	PCSTR		szValueA = NULL ;
	BOOL		bNeedToFreeValueA = FALSE ;

	try {
		if ( bUseUtf8 ) {
			szValueA = _Utf8Encode (szValue) ;
			bNeedToFreeValueA = TRUE ;
		} else
			szValueA = _String2Ansi (szValue, bNeedToFreeValueA) ;

		_WritePost (szValueA, bIsValue) ;
	} catch (Exception &) {
		if ( bNeedToFreeValueA )
			SAFEFREE (szValueA) ;
		throw ;
	}

	if ( bNeedToFreeValueA )
		SAFEFREE (szValueA) ;
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_WritePost (LPCSTR szValue, BOOL bIsValue)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpClientT::_WritePost: m_hRequest can not be NULL.") ;

	if ( szValue == NULL )
		return ;

	::SafeInt<DWORD>		cbValue ;
	try {
		cbValue = ::strlen (szValue) ;
	} catch (::SafeIntException & e) {
		HttpTool::ThrowException (e) ;
	}

	_WritePost (reinterpret_cast<const BYTE *> (szValue), cbValue.Value (), bIsValue) ;
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_WritePost (const BYTE * pbyBuff, DWORD cbyBuff, BOOL bIsValue)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpClientT::_WritePost: m_hRequest can not be NULL.") ;

	if ( pbyBuff == NULL || cbyBuff == 0 )
		return ;

	// Before increasing values. Checks overflow exception.
	if ( bIsValue )		m_objPostStat._TestAddPostedBytes (cbyBuff) ;
	else				m_objPostStat._TestAddActualPostedBytes (cbyBuff) ;

	HttpTool::InternetWriteFile (m_hRequest, pbyBuff, cbyBuff) ;

	if ( bIsValue )		m_objPostStat._AddPostedBytes (cbyBuff) ;
	else				m_objPostStat._AddActualPostedBytes (cbyBuff) ;
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_StartPostContext (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags, PCSZ szReferer
																		, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	// Closes the POST context if it exists.
	_EndPostContext () ;

	BOOL		bBorrowedInternet = TRUE ;
	BOOL		bBorrowedConnection = TRUE ;
	HINTERNET	hRequest = NULL ;

	try {
		// Calculates the total number of bytes to upload
		::SafeInt<size_t>	nTotalByte = 0 ;
		::SafeInt<size_t>	nValuesTotalByte = 0 ;
		PCSZ				szFirstParamName = NULL ;
		size_t				cbFirstParamValue = 0 ;

		if ( !m_mapParam.Empty () ) {
			ConstMapIter		iter ;

			size_t		cbValue ;
			for (iter = m_mapParam.Begin (); iter != m_mapParam.End (); ++iter) {
				nTotalByte += 1 ;		// '&'

				if ( (iter->second).dwFlag & ParamEncodedName )
					nTotalByte += _String2AnsiLen (iter->first) ;
				else
					nTotalByte += _UrlEncodeLenA (iter->first) ;

				nTotalByte += 1 ;		// '='

				if ( (iter->second).dwFlag & ParamEncodedValue )
					cbValue = _String2AnsiLen ((iter->second).szValue) ;
				else
					cbValue = _UrlEncodeLenA ((iter->second).szValue) ;

				nTotalByte += cbValue ;
				nValuesTotalByte += cbValue ;

				// If the item is the first item
				if ( iter == m_mapParam.Begin () ) {
					szFirstParamName = iter->first ;
					cbFirstParamValue = cbValue ;
				}
			}

			if ( nTotalByte > 0 )
				nTotalByte-- ;		// for the first '&'
		}

		::SafeInt<DWORD>		dwTotalByte = nTotalByte ;

		// Get WinInet handles
		if ( hInternet == NULL ) {
			hInternet = OpenInternet () ;
			bBorrowedInternet = FALSE ;
		}

		if ( hConnection == NULL ) {
			hConnection = OpenConnection (hInternet, szUrl, szUsrName, szUsrPwd) ;
			bBorrowedConnection = FALSE ;
		}

		hRequest = OpenRequest (hConnection, HttpTool::szPost, szUrl, dwFlags, szReferer) ;
		AddRequestHeader (hRequest) ;			// Adds user's custom header

		// Adds the Content-Type header
		HttpTool::AddHeader (hRequest, HttpTool::szContentType, HttpTool::szFormUrlEncoded, m_nAnsiCodePage) ;

		// Connects to the HTTP server
		HttpTool::SendRequestEx (hRequest, dwTotalByte.Value ()) ;

		// Starts a new POST state
		m_objPostStat._MakeActive (nTotalByte.Value (), nValuesTotalByte.Value (), m_mapParam.Count (), 0) ;

		m_bBorrowedInternet = bBorrowedInternet ;
		m_hInternet = hInternet ;
		m_bBorrowedConnection = bBorrowedConnection ;
		m_hConnection = hConnection ;
		m_hRequest = hRequest ;
		m_bIsPost = TRUE ;

		// Initializes the initial POST state
		if ( !m_mapParam.Empty () ) {
			m_objInitialStat = m_objPostStat ;
			// It always does not throw an overflow exception.
			// So it's safe. (doesn't need to restore the internal states)
			m_objInitialStat._StartNewEntry (szFirstParamName, cbFirstParamValue) ;	
		}

	} catch (::SafeIntException & e) {
		HttpTool::CloseRequest (hRequest) ;
		if ( !bBorrowedConnection )	HttpTool::CloseRequest (hConnection) ;
		if ( !bBorrowedInternet )	HttpTool::CloseRequest (hInternet) ;
		HttpTool::ThrowException (e) ;
	} catch (Exception &) {
		HttpTool::CloseRequest (hRequest) ;
		if ( !bBorrowedConnection )	HttpTool::CloseRequest (hConnection) ;
		if ( !bBorrowedInternet )	HttpTool::CloseRequest (hInternet) ;
		throw ;
	}
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_QueryPostStat (CHttpPostStat & objPostStat)
	throw ()
{
	// If the _ProceedPOSTContext method is not called yet,
	if ( (m_objPostStat.IsActive ()) && (m_objPostStat.TotalCount () > 0) 
									&& (m_objPostStat.PostedCount () == 0) )
		objPostStat = m_objInitialStat ;
	else
		objPostStat = m_objPostStat ;
}

template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::_CancelPostContext (void)
	throw ()
{
	if ( !m_objPostStat.IsActive () )
		return FALSE ;

	// Cancels the POST context
	_EndPostContext () ;
	return TRUE ;
}

// If all parameters are sent, returns a pointer to the CHttpResponseT.
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::_ProceedPostContext (DWORD nDesired)
	throw (Exception &)
{
	// If the Post context is not started
	if ( !m_objPostStat.IsActive () )
		HttpTool::ThrowException (HTTPCLIENT_ERR_POST_NOT_STARTED) ;

	HTTPCLIENT_ASSERT (m_hInternet != NULL, "CHttpClientT::_ProceedPostContext: m_hInternet can not be NULL.") ;
	HTTPCLIENT_ASSERT (m_hConnection != NULL, "CHttpClientT::_ProceedPostContext: m_hConnection can not be NULL.") ;
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpClientT::_ProceedPostContext: m_hRequest can not be NULL.") ;
	HTTPCLIENT_ASSERT (nDesired != 0, "CHttpClientT::_ProceedPostContext: nDesired can not be zero.") ;

	try {
		// If the current parameter is completed,
		if ( m_objPostStat.CurrParamIsComplete () ) {
	
			// If all parameters are sent
			if ( m_objPostStat.TotalCount () == m_objPostStat.PostedCount () ) {
				HttpTool::EndRequest (m_hRequest) ;

				// Releases the POST context
				return _ReleasePostResponse () ;
			}

			DWORD			nNextIdx = m_objPostStat.PostedCount () ;

			ConstMapIter	iter = m_mapParam.Begin () ;
			for (DWORD i = 0; i < nNextIdx; i++, ++iter) ;

			if ( nNextIdx != 0 )
				_WritePost ("&") ;

			// Writes the parameter name
			{
				PSTR			szNameA = NULL ;
				BOOL			bNeedToFreeNameA = FALSE ;

				try {
					if ( (iter->second).dwFlag & ParamEncodedName ) {
						szNameA = const_cast<PSTR> (_String2Ansi (iter->first, bNeedToFreeNameA)) ;
						_WritePost (szNameA) ;
					} else {
						::SafeInt<size_t>		cbRequired = _UrlEncodeLenA (iter->first) ;

						if ( cbRequired > 0 ) {
							cbRequired += 1 ;
							cbRequired *= sizeof (CHAR) ;

							if ( NULL == (szNameA = (PSTR) ::malloc (cbRequired.Value ())) )
								HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
							bNeedToFreeNameA = TRUE ;
							_UrlEncodeA (szNameA, iter->first) ;
							_WritePost (szNameA) ;
						}
					}
				} catch (::SafeIntException & e) {
					if ( bNeedToFreeNameA )
						SAFEFREE (szNameA) ;
					HttpTool::ThrowException (e) ;
				} catch (Exception &) {
					if ( bNeedToFreeNameA )
						SAFEFREE (szNameA) ;
					throw ;
				}

				if ( bNeedToFreeNameA ) {
					SAFEFREE (szNameA) ;
					bNeedToFreeNameA = FALSE ;
				}
			}

			_WritePost ("=") ;

			size_t			cbValue = 0 ;

			// Sets the value
			if ( (iter->second).szValue ) {
				PSTR			szValueA = NULL ;
				BOOL			bNeedToFreeValueA = FALSE ;

				if ( (iter->second).dwFlag & ParamEncodedValue ) {
					szValueA = const_cast<PSTR> (_String2Ansi ((iter->second).szValue, bNeedToFreeValueA)) ;
					cbValue = szValueA ? CHttpToolA::StringLen (szValueA) : 0 ;
				} else {
					cbValue = _UrlEncodeLenA ((iter->second).szValue) ;

					if ( cbValue > 0 ) {
						::SafeInt<size_t>	cbRequired = cbValue ;

						try {
							cbRequired += 1 ;
							cbRequired *= sizeof (CHAR) ;
						} catch (::SafeIntException & e) {
							HttpTool::ThrowException (e) ;
						}

						if ( NULL == (szValueA = static_cast<PSTR> (::malloc (cbRequired.Value ()))) )
							HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
						bNeedToFreeValueA = TRUE ;

						try {
							_UrlEncodeA (szValueA, (iter->second).szValue) ;
						} catch (Exception &) {
							if ( bNeedToFreeValueA )
								SAFEFREE (szValueA) ;
							throw ;
						}
					}
				}

				_SetPostedValue (szValueA, bNeedToFreeValueA) ;
			}

			// Starts a new parameter
			m_objPostStat._StartNewEntry (iter->first, cbValue) ;

			return NULL ;
		}

		DWORD			cbToWrite = nDesired ;

		if ( cbToWrite > m_objPostStat.CurrParamRemainByte () )
			cbToWrite = static_cast<DWORD> (m_objPostStat.CurrParamRemainByte ()) ;

		_WritePost (
			reinterpret_cast<const BYTE *> (m_szPostedValue + m_objPostStat.CurrParamPostedByte ())
			, cbToWrite, TRUE) ;

	} catch (Exception &) {
		_EndPostContext () ;				// Aborts the Post Context
		throw ;
	}

	return NULL ;
}

template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ 
CHttpClientT<HttpTool, HttpEncoder>::_GetUploadBoundary (void)
	throw ()
{
	return m_szBoundary ? m_szBoundary : HttpTool::szDefBoundary ;
}

template <typename HttpTool, typename HttpEncoder>
LPCSTR CHttpClientT<HttpTool, HttpEncoder>::_GetUploadBoundaryA (void)
	throw ()
{
	return m_szBoundaryA ? m_szBoundaryA : CHttpToolA::szDefBoundary ;
}

template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::PCSZ 
CHttpClientT<HttpTool, HttpEncoder>::_GetUploadContType (void)
	throw ()
{
	return m_szUploadContType ? m_szUploadContType : HttpTool::szDefUploadContType ;
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_StartUploadContext (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl
																	 , DWORD dwFlags, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd)
	throw (Exception &)
{
	// Closes any existing Post Context
	_EndPostContext () ;

	BOOL				bBorrowedInternet = TRUE ;
	BOOL				bBorrowedConnection = TRUE ;
	HINTERNET			hRequest = NULL ;

	HANDLE *			ahFileHandles = NULL ;
	LPSTR *				aszMimeTypes = NULL ;
	::SafeInt<DWORD>	nPostedFileCount = 0 ;

	try {
		// Calculates the nubmer of bytes to upload
		::SafeInt<size_t>	nTotalByte = 0 ;
		::SafeInt<size_t>	nValuesTotalByte = 0 ;
		size_t				cchBoundary = HttpTool::StringLen (_GetUploadBoundary ()) ;

		PCSZ				szFirstParamName = NULL ;
		PCSZ				szFirstParamFileName = NULL ;
		size_t				cbFirstParamValue = 0 ;
		BOOL				bFirstParamIsFile = FALSE ;

		if ( !m_mapParam.Empty () ) {
			try {
				ConstMapIter		iter ;

				// Get the number of files
				for (iter = m_mapParam.Begin (); iter != m_mapParam.End (); ++iter) {
					if ( (iter->second).dwFlag & ParamFile )
						nPostedFileCount++ ;
				}

				if ( nPostedFileCount.Value () ) {
					// Allocates memory for handles and MimeTypes of the uploaded files
					ahFileHandles = (HANDLE *) ::calloc (nPostedFileCount.Value (), sizeof (HANDLE)) ;
					if ( ahFileHandles == NULL )
						HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;

					// Initializes the file handles
					for (DWORD i = 0; i < nPostedFileCount; i++)
						ahFileHandles[i] = INVALID_HANDLE_VALUE ;

					aszMimeTypes = (LPSTR *) ::calloc (nPostedFileCount.Value (), sizeof (LPSTR)) ;
					if ( aszMimeTypes == NULL )
						HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
				}

				/*
					Calculates the total upload size

					<1> The number of bytes to upload If the parameter is not a file
						= strlen ("--") + strlen (Boundary) + strlen ("\r\n") 
							+ strlen ("Content-Disposition: form-data; name=\"\"\r\n\r\n")
							+ strlen (The name of the parameter) + strlen (The value of the parameter) + strlen ("\r\n") ;

					<2> The number of bytes to upload If the parameter is a file
						= strlen ("--") + strlen (Boundary) + strlen ("\r\n") 
							+ strlen ("Content-Disposition: form-data; name=\"\"; filename=\"\"\r\n")
							+ strlen (The name of the parameter) + strlen (The value of the parameter)
							+ strlen ("Content-Type: \r\n\r\n") + strlen (The Mime Type of the file)
							+ The length of the file + strlen ("\r\n")

					The last boundary
						= strlen ("--") + strlen (Boundary) + strlen ("--\r\n") 

					The total upload size
						= ( <1> * The number of normal parameters)
							+ ( <2> * The number of file parameters)
							+ The last boundary
				*/

				::SafeInt<size_t>	cbValue ;
				DWORD				nFileIdx = 0 ;

				nTotalByte = 0 ;
				nValuesTotalByte = 0 ;
				for (iter = m_mapParam.Begin (); iter != m_mapParam.End (); ++iter) {
					if ( !((iter->second).dwFlag & ParamFile) ) {
						// If the parameter is not a file
						//	= strlen ("--") + strlen (Boundary) + strlen ("\r\n") 
						//		+ strlen ("Content-Disposition: form-data; name=\"\"\r\n\r\n")
						//		+ strlen (The name of the parameter) + strlen (The value of the parameter) + strlen ("\r\n") ;
						nTotalByte += (cchBoundary + 4) ;
						nTotalByte += 43 ;

						if ( m_bUseUtf8 ) {
							nTotalByte += _Utf8EncodeLen (iter->first) ;
							cbValue = _Utf8EncodeLen ((iter->second).szValue) ;
						} else {
							nTotalByte += _String2AnsiLen (iter->first) ;
							cbValue = _String2AnsiLen ((iter->second).szValue) ;
						}

						nTotalByte += cbValue ;
						nValuesTotalByte += cbValue ;

						nTotalByte += 2 ;

						// Saves the state of the first parameter
						if ( iter == m_mapParam.Begin () ) {
							szFirstParamName = iter->first ;
							cbFirstParamValue = cbValue.Value () ;
							bFirstParamIsFile = FALSE ;
							szFirstParamFileName = NULL ;
						}
					} else {
						// If the parameter is a file
						//	= strlen ("--") + strlen (Boundary) + strlen ("\r\n") 
						//		+ strlen ("Content-Disposition: form-data; name=\"\"; filename=\"\"\r\n")
						//		+ strlen (The name of the parameter) + strlen (The value of the parameter)
						//		+ strlen ("Content-Type: \r\n\r\n") + strlen (The Mime Type of the file)
						//		+ The length of the file + strlen ("\r\n")
						nTotalByte += (cchBoundary + 4) ;
						nTotalByte += 54 ;

						if ( m_bUseUtf8 ) {
							nTotalByte += _Utf8EncodeLen (iter->first) ;
							nTotalByte += _Utf8EncodeLen ((iter->second).szValue) ;
						} else {
							nTotalByte += _String2AnsiLen (iter->first) ;
							nTotalByte += _String2AnsiLen ((iter->second).szValue) ;
						}

						nTotalByte += 18 ;

						// Get the file size and MimeType
						cbValue = 0 ;
						if ( (iter->second).szValue ) {
							// Open the file
							ahFileHandles[nFileIdx] = HttpTool::OpenFile ((iter->second).szValue) ;

							// Get the file size
							if ( ahFileHandles[nFileIdx] != INVALID_HANDLE_VALUE )
								cbValue = HttpTool::GetFileSize (ahFileHandles[nFileIdx], (iter->second).szValue) ;
						}

						// Throws an exception
						if ( m_bStrictFileCheck && (ahFileHandles[nFileIdx] == INVALID_HANDLE_VALUE) )
							HttpTool::ThrowException (HTTPCLIENT_ERR_OPENFILE_FAILED, ::GetLastError (), (iter->second).szValue) ;

						// Get the MimeType of the file
						aszMimeTypes[nFileIdx] = HttpTool::GetMimeType (ahFileHandles[nFileIdx], m_nAnsiCodePage) ;
						nTotalByte += CHttpToolA::StringLen (aszMimeTypes[nFileIdx]) ;
						nTotalByte += cbValue ;
						nValuesTotalByte += cbValue ;
						nTotalByte += 2 ;
						nFileIdx++ ;

						// Saves the state of the first parameter
						if ( iter == m_mapParam.Begin () ) {
							szFirstParamName = iter->first ;
							cbFirstParamValue = cbValue.Value () ;
							bFirstParamIsFile = TRUE ;
							szFirstParamFileName = (iter->second).szValue ;
						}
					}
				}

				// The last boundary
				//	= strlen ("--") + strlen (Boundary) + strlen ("--\r\n") 
				nTotalByte += (cchBoundary + 6) ;
			} catch (::SafeIntException & e) {
				HttpTool::ThrowException (e) ;
			}

		} else {
			// The total upload size
			//  = (strlen ("\r\n") + strlen ("--") + strlen (Boundary) + strlen ("--\r\n")
			nTotalByte = cchBoundary + 8 ;
			nValuesTotalByte = 0 ;
		}

		::SafeInt<DWORD>	dwTotalByte ;
		try {
			dwTotalByte = nTotalByte ;
		} catch (::SafeIntException & e) {
			HttpTool::ThrowException (e) ;
		}

		// Get WinInet handles
		if ( hInternet == NULL ) {
			hInternet = OpenInternet () ;
			bBorrowedInternet = FALSE ;
		}

		if ( hConnection == NULL ) {
			hConnection = OpenConnection (hInternet, szUrl, szUsrName, szUsrPwd) ;
			bBorrowedConnection = FALSE ;
		}

		hRequest = OpenRequest (hConnection, HttpTool::szPost, szUrl, dwFlags, szReferer) ;
		m_hLastReq = hRequest;
		AddRequestHeader (hRequest) ;			// Adds user's custom header
		
		// Adds the Content-Type header
		HttpTool::AddHeader (hRequest, HttpTool::szContentType, _GetUploadContType (), m_nAnsiCodePage) ;

		// Make a connection to the server
		HttpTool::SendRequestEx (hRequest, dwTotalByte.Value ()) ;

		// Activates the Post Context
		m_objPostStat._MakeActive (nTotalByte.Value (), nValuesTotalByte.Value (), m_mapParam.Count (), nPostedFileCount.Value ()) ;
		m_bBorrowedInternet = bBorrowedInternet ;
		m_hInternet = hInternet ;
		m_bBorrowedConnection = bBorrowedConnection ;
		m_hConnection = hConnection ;
		m_hRequest = hRequest ;
		m_ahPostedFiles = ahFileHandles ;
		ahFileHandles = NULL ;
		m_aszMimeTypes = aszMimeTypes ;
		aszMimeTypes = NULL ;
		m_bIsPost = FALSE ;

		// Saves the initial Post context
		if ( !m_mapParam.Empty () ) {
			m_objInitialStat = m_objPostStat ;
			// It always does not throw an overflow exception.
			// So it's safe. (doesn't need to restore the internal states)
			m_objInitialStat._StartNewEntry (szFirstParamName, cbFirstParamValue
										, bFirstParamIsFile, szFirstParamFileName) ;
		}

	} catch (Exception &) {
		HttpTool::CloseRequest (hRequest) ;
		if ( !bBorrowedConnection )	HttpTool::CloseRequest (hConnection) ;
		if ( !bBorrowedInternet )	HttpTool::CloseRequest (hInternet) ;

		for (DWORD i = 0; i < nPostedFileCount; i++) {
			if ( ahFileHandles ) {
				if ( ahFileHandles[i] != INVALID_HANDLE_VALUE ) {
					::CloseHandle (ahFileHandles[i]) ;
					ahFileHandles[i] = INVALID_HANDLE_VALUE ;
				}
			}

			if ( aszMimeTypes )
				SAFEFREE ( (aszMimeTypes[i]) ) ;
		}
		SAFEFREE (ahFileHandles) ;
		SAFEFREE (aszMimeTypes) ;
		throw ;
	}
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_QueryUploadStat (CHttpPostStat & objPostStat)
	throw ()
{
	// If the _ProceedUploadContext method is not called yet,
	if ( (m_objPostStat.IsActive ()) && (m_objPostStat.TotalCount () > 0)
										&& (m_objPostStat.PostedCount () == 0) )
		objPostStat = m_objInitialStat ;
	else
		objPostStat = m_objPostStat ;
}

template <typename HttpTool, typename HttpEncoder>
BOOL CHttpClientT<HttpTool, HttpEncoder>::_CancelUploadContext (void)
	throw ()
{
	if ( !m_objPostStat.IsActive () )
		return FALSE ;

	// Cancels the Post Context
	_EndPostContext () ;
	return TRUE ;
}

// If all parameters are sent, returns a pointer to the CHttpResponseT.
template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::_ProceedUploadContext (DWORD nDesired)
	throw (Exception &)
{
	// If the Post context is not started
	if ( !m_objPostStat.IsActive () )
		HttpTool::ThrowException (HTTPCLIENT_ERR_POST_NOT_STARTED) ;

	HTTPCLIENT_ASSERT (m_hInternet != NULL, "CHttpClientT::_ProceedUploadContext: m_hInternet can not be NULL.") ;
	HTTPCLIENT_ASSERT (m_hConnection != NULL, "CHttpClientT::_ProceedUploadContext: m_hConnection can not be NULL.") ;
	HTTPCLIENT_ASSERT (m_hRequest != NULL, "CHttpClientT::_ProceedUploadContext: m_hRequest can not be NULL.") ;
	HTTPCLIENT_ASSERT (nDesired != 0, "CHttpClientT::_ProceedUploadContext: nDesired can not be zero.") ;

	try {
		/*if ( m_objPostStat.CurrParamIsFile () ) {
			DWORD			nFileIndex = m_objPostStat.PostedFileCount () - 1 ;

				// Read from file
				::CloseHandle (m_ahPostedFiles[nFileIndex]);
		}*/
		// If all parameters are posted
		// releases the Post context
		if ( m_objPostStat._IsComplete () ) {
			HttpTool::EndRequest (m_hRequest) ;
			return _ReleasePostResponse () ;
		}

		LPCSTR		szBoundary = _GetUploadBoundaryA () ;

		// If there is no parameter to upload
		if ( m_objPostStat.TotalCount () == 0 ) {
			// Writes the last boundary
			_WritePost ("\r\n--") ;
			_WritePost (szBoundary) ;
			_WritePost ("--\r\n") ;
			return NULL ;
		}

		// If the current parameter is completed
		if ( m_objPostStat.CurrParamIsComplete () ) {


			// If all parameters are sent
			if ( m_objPostStat.TotalCount () == m_objPostStat.PostedCount () ) {
				// Writes the last boundary
				_WritePost ("--") ;
				_WritePost (szBoundary) ;
				_WritePost ("--\r\n") ;
				return NULL ;
			}

			DWORD				nNextIdx = m_objPostStat.PostedCount () ;
			DWORD				nNextFileIdx = m_objPostStat.PostedFileCount () ;
			PCSZ				szEntryFile = NULL ;
			::SafeInt<size_t>	nEntryValueTotalByte = 0 ;

			ConstMapIter	iter = m_mapParam.Begin () ;
			for (size_t i = 0; i < nNextIdx; i++, ++iter) ;

			if ( (iter->second).dwFlag & ParamFile ) {
				// If the parameter is a file parameter

				szEntryFile = (iter->second).szValue ;
				if ( m_ahPostedFiles[nNextFileIdx] != INVALID_HANDLE_VALUE )
					nEntryValueTotalByte = HttpTool::GetFileSize (m_ahPostedFiles[nNextFileIdx], szEntryFile) ;

				// Writes the boundary and headers
				_WritePost ("--") ;
				_WritePost (szBoundary) ;
				_WritePost ("\r\n") ;
				_WritePost ("Content-Disposition: form-data; name=\"") ;
				_WritePost (m_bUseUtf8, iter->first) ;		// Writes form name
				_WritePost ("\"; filename=\"") ;
				_WritePost (m_bUseUtf8, szEntryFile) ;		// Writes file path
				_WritePost ("\"\r\nContent-Type: ") ;
				_WritePost (m_aszMimeTypes[nNextFileIdx]) ;
				_WritePost ("\r\n\r\n") ;

			} else {
				// If the parameter is not a file parameter

				if ( (iter->second).szValue && (iter->second).szValue[0] != '\0' ) {
					LPCSTR		szPostedValue ;
					BOOL		bNeedToFree ;

					if ( m_bUseUtf8 ) {
						szPostedValue = _Utf8Encode ((iter->second).szValue) ;
						bNeedToFree = TRUE ;
					} else
						// Converts into a Ansi string
						szPostedValue = _String2Ansi ((iter->second).szValue, bNeedToFree) ;

					nEntryValueTotalByte = CHttpToolA::StringLen (szPostedValue) ;
					_SetPostedValue (szPostedValue, bNeedToFree) ;
				} else {
					_SetPostedValue (NULL, FALSE) ;
					nEntryValueTotalByte = 0 ;
				}

				// Writes the boundary and headers
				_WritePost ("--") ;
				_WritePost (szBoundary) ;
				_WritePost ("\r\n") ;
				_WritePost ("Content-Disposition: form-data; name=\"") ;
				_WritePost (m_bUseUtf8, iter->first) ;		// Write form name
				_WritePost ("\"\r\n\r\n") ;
			}

			// Starts a new parameter
			m_objPostStat._StartNewEntry (iter->first, nEntryValueTotalByte.Value ()
										, static_cast<BOOL> ((iter->second).dwFlag & ParamFile), szEntryFile) ;

			if ( nEntryValueTotalByte == 0 )
				_WritePost ("\r\n") ;
			return NULL ;
		}

		// Writes the requested number of bytes
		DWORD		cbToWrite = nDesired ;
		if ( cbToWrite > m_objPostStat.CurrParamRemainByte () )
			cbToWrite = static_cast<DWORD> (m_objPostStat.CurrParamRemainByte ()) ;

		DWORD			cbWritten = 0 ;

		if ( m_objPostStat.CurrParamIsFile () ) {
			_InitPostCntxBuff () ;

			DWORD			cbBuff ;				// The number of bytes to read
			DWORD			cbRead ;				// The number of bytes read from the file
			DWORD			nFileIdx = m_objPostStat.PostedFileCount () - 1 ;

			while ( cbWritten < cbToWrite ) {
				cbBuff = cbToWrite - cbWritten > 
					HTTPCLIENT_POSTCNTX_BUFF_SIZE ? HTTPCLIENT_POSTCNTX_BUFF_SIZE : cbToWrite - cbWritten ;

				// Read from file
				if ( !::ReadFile (m_ahPostedFiles[nFileIdx]
						, m_pbyCntxBuff
						, cbBuff
						, &cbRead
						, NULL)
					)
					HttpTool::ThrowException (HTTPCLIENT_ERR_READFILE_FAILED, ::GetLastError ()) ;

				// cbBuff and cbRead must be the same
				if ( cbBuff != cbRead )
					HttpTool::ThrowException (HTTPCLIENT_ERR_READ_UNEXPECTED_SIZE) ;

				_WritePost (m_pbyCntxBuff, cbBuff, TRUE) ;
				cbWritten += cbBuff ;
			}
		} else {
			_WritePost (
				reinterpret_cast<const BYTE *> (m_szPostedValue + m_objPostStat.CurrParamPostedByte ())
				, cbToWrite, TRUE) ;
			cbWritten = cbToWrite ;
		}

		// If all bytes are written, writes the last new line character
		if ( m_objPostStat.CurrParamRemainByte () == 0 )
			_WritePost ("\r\n") ;

	} catch (::SafeIntException & e) {
		_EndPostContext () ;			// Aborts the POST context if an error occurs
		HttpTool::ThrowException (e) ;
	} catch (Exception &) {
		_EndPostContext () ;			// Aborts the POST context if an error occurs
		throw ;
	}
	return NULL ;
}

template <typename HttpTool, typename HttpEncoder>
typename CHttpClientT<HttpTool, HttpEncoder>::CHttpResponse * 
CHttpClientT<HttpTool, HttpEncoder>::_ReleasePostResponse ()
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_objPostStat.IsActive (), "CHttpClientT::_ReleasePostResponse: The post context is not active.") ;

	HINTERNET		hReleasedInternet = NULL ;
	HINTERNET		hReleasedConnection = NULL ;
	HINTERNET		hReleasedRequest = NULL ;

	_ReleasePostContext (hReleasedInternet, hReleasedConnection, hReleasedRequest) ;
	CHttpResponse *			pobjRes = NULL ;

	try {
		if ( NULL == (pobjRes = new CHttpResponse (hReleasedInternet, hReleasedConnection, hReleasedRequest)) )
			throw "Out of memory" ;

	} catch (...) {
		HttpTool::CloseRequest (hReleasedRequest) ;
		HttpTool::CloseConnection (hReleasedConnection) ;
		HttpTool::CloseInternet (hReleasedInternet) ;
		HttpTool::ThrowException (HTTPCLIENT_ERR_OUT_OF_MEMORY) ;
	}

	return pobjRes ;
}

// Releases the POST context when the upload task has been completed
template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_ReleasePostContext (HINTERNET & hInternet, HINTERNET & hConnection, HINTERNET & hRequest)
	throw (Exception &)
{
	HTTPCLIENT_ASSERT (m_objPostStat.IsActive (), "CHttpClientT::_ReleasePostResponse: The post context is not active.") ;

	hInternet = m_bBorrowedInternet ? NULL : m_hInternet ;
	hConnection = m_bBorrowedConnection ? NULL : m_hConnection ;
	hRequest = m_hRequest ;

	m_hInternet = NULL ;
	m_bBorrowedInternet = FALSE ;
	m_hConnection = NULL ;
	m_bBorrowedConnection = FALSE ;
	//HttpTool::CloseRequest();
	if(m_hRequest)
	{
		//HttpTool::CloseRequest (m_hRequest) ;
	m_hRequest = NULL ;
	}

	// Closes all file handles and frees the MimeType
	if ( m_ahPostedFiles ) {
		for (DWORD i = 0; i < m_objPostStat.FileCount (); i++) {
			if ( m_ahPostedFiles[i] != INVALID_HANDLE_VALUE ) {
				::CloseHandle (m_ahPostedFiles[i]) ;
				m_ahPostedFiles[i] = INVALID_HANDLE_VALUE ;
			}

			if ( m_aszMimeTypes[i] )
				SAFEFREE ( (m_aszMimeTypes[i]) ) ;
		}

		SAFEFREE (m_ahPostedFiles) ;
		SAFEFREE (m_aszMimeTypes) ;
	}

	// Removes the posted value
	_SetPostedValue (NULL, FALSE) ;

	// Frees the buffer
	SAFEFREE (m_pbyCntxBuff) ;

	// Unactivate the Post context
	m_objPostStat._MakeUnActive () ;
}

template <typename HttpTool, typename HttpEncoder>
void CHttpClientT<HttpTool, HttpEncoder>::_EndPostContext (void)
	throw (Exception &)
{
	if ( !m_objPostStat.IsActive () )
		return ;

	HINTERNET		hInternet, hConnection, hRequest ;
	_ReleasePostContext (hInternet, hConnection, hRequest) ;

	// Closes internet handles
	HttpTool::CloseRequest (hRequest) ;
	HttpTool::CloseConnection (hConnection) ;
	HttpTool::CloseInternet (hInternet) ;
}
///////////////////////////////////////// CHttpClientT /////////////////////////////////////////

}

