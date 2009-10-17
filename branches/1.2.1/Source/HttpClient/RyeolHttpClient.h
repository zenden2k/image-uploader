/*!
 * \file	RyeolHttpClient.h
 * \brief	classes which helps to interact with a HTTP server.
 * \author	Jo Hyeong-ryeol
 * \since	2004.04.12
 * \version	$LastChangedRevision: 101 $
 *			$LastChangedDate: 2006-02-04 01:28:18 +0900 (토, 04 2 2006) $
 * 
 * <dl compact>
 * <dt><b>Requirements:</b></dt>
 * <dd>Requires Internet Explorer 4.0 or later.</dd><br>
 * <dd>Unicode version class support on Windows Me/98/95 requires Microsoft Layer for Unicode.</dd><br>
 * <dd>UTF-8 encoding support on Windows 95 requires Microsoft Layer for Unicode.</dd>
 * </dl>
 * \n

<h2>Introduction</h2>
This file contains classes which help to interact with a HTTP server.
The codes contained in this file depends on the documentation of STLPort 4.6.1
(describing about exception safety).<br>
<br><br>

<h2>How to use RyeolHttpClient</h2>

<p>
In your project, include the following files.
<ul>
    <li> RyeolException.h
    <li> RyeolException.cpp
    <li> RyeolHttpClient.h
    <li> RyeolHttpClient.cpp
    <li> SafeInt.hpp
</ul>

In your stdafx.h file, add the following line.

\code
#include "RyeolHttpClient.h"
\endcode


<h2>How to send a request using HTTP GET</h2>
<code>CHttpClient</code> supports <code>RequestGet</code> method which sends a request using HTTP GET.
\code
// Retrieves the resource specified by the szUrl using HTTP GET request.
// szUrl            [in] A HTTP URL.
// bUseCache        [in] Specifies whether to use cache.
CHttpResponse * CHttpClient::RequestGet (PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &amp;) ;
\endcode
The following code demonstrates the basic usage of the <code>RequestGet</code> method.
\code
using namespace Ryeol ;

CHttpClient         objHttpReq ;
CHttpResponse *     pobjHttpRes = NULL ;

try {
    // Initialize the User Agent
    objHttpReq.SetInternet (_T ("My User Agent v1.0")) ;

    // Specifies whether to use UTF-8 encoding. (This uses ANSI encoding)
    // Default is FALSE
    objHttpReq.SetUseUtf8 (FALSE) ;

    // Specifies a code page for ANSI strings. (This uses Korean)
    // Default is CP_ACP
    objHttpReq.SetAnsiCodePage (949) ;

    // Add user's custom HTTP headers
    objHttpReq.AddHeader (_T ("Ryeol-Magic"), _T ("My Magic Header")) ;
    objHttpReq.AddHeader (_T ("User-Magic"), _T ("User's Magic Header")) ;

    // Add user's parameters
    objHttpReq.AddParam (_T ("where"), _T ("nexearch")) ;
    objHttpReq.AddParam (_T ("frm"), _T ("t1")) ;
    objHttpReq.AddParam (_T ("query"), _T ("%C3%D6%C1%F6%BF%EC"), CHttpClient::ParamEncodedValue) ;

    // Send a request
    pobjHttpRes = objHttpReq.RequestGet (_T ("http://search.naver.com/search.naver")) ;

    ...     // Place codes to handle the returned CHttpResponse object.

} catch (httpclientexception &amp; e) {
    ...     // Place exception handling codes here.
}
\endcode


<h2>How to send a request using HTTP POST</h2>
The HTTP POST method is used in two ways. One is to post simple text, the other is to upload file.
To post simple text, <code>CHttpClient</code> provides <code>BeginPost</code> method.
\code
// Starts a new HTTP POST request
// szUrl            [in] A HTTP URL.
// bUseCache        [in] Specifies whether to use cache.
void CHttpClient::BeginPost (PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &amp;) ;
\endcode
The following code demonstrates the basic usage of the <code>BeginPost</code> method.
\code
using namespace Ryeol ;

CHttpClient         objHttpReq ;
CHttpResponse *     pobjHttpRes = NULL ;

try {
    // Initialize the User Agent
    objHttpReq.SetInternet (_T ("My User Agent v1.0")) ;

    // Add user's custom HTTP headers
    objHttpReq.AddHeader (_T ("Ryeol-Magic"), _T ("My Magic Header")) ;
    objHttpReq.AddHeader (_T ("User-Magic"), _T ("User's Magic Header")) ;

    // Add user's parameter
    objHttpReq.AddParam (_T ("st"), _T ("kw")) ;
    objHttpReq.AddParam (_T ("target"), _T ("WinInet")) ;

    // Start a new request
    objHttpReq.BeginPost (_T ("http://www.codeproject.com/info/search.asp")) ;

    // Specifies the number of bytes to send when the Proceed method is called.
    const DWORD     cbProceed = 1024 ;  // 1K

    do {

        ...     // Place codes to report progress information to user.

    } while ( !(pobjHttpRes = objHttpReq.Proceed (cbProceed)) ) ;

    ...     // Place codes to handle the returned CHttpResponse object.

} catch (httpclientexception &amp; e) {
    ...     // Place exception handling codes here.
}
\endcode

To upload file, <code>CHttpClient</code> provides <code>BeginUpload</code> method.
\code
// Starts a new UPLOAD request
// szUrl            [in] A HTTP URL.
// bUseCache        [in] Specifies whether to use cache.
void CHttpClient::BeginUpload (PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &amp;) ;
\endcode
The following code demonstrates the basic usage of the <code>BeginUpload</code> method.
\code
using namespace Ryeol ;

CHttpClient         objHttpReq ;
CHttpResponse *     pobjHttpRes = NULL ;

try {
    // Initialize the User Agent
    objHttpReq.SetInternet (_T ("My User Agent v1.0")) ;

    // Add user's custom HTTP headers
    objHttpReq.AddHeader (_T ("Ryeol-Magic"), _T ("My Magic Header")) ;
    objHttpReq.AddHeader (_T ("User-Magic"), _T ("User's Magic Header")) ;

    // Add user's parameters
    objHttpReq.AddParam (_T ("nohtml"), _T ("1")) ;
    objHttpReq.AddParam (_T ("title"), _T ("The K-NET photo")) ;
    objHttpReq.AddParam (_T ("content"), _T ("A photo of the K-NET")) ;

    // Specifies a file to upload
    objHttpReq.AddParam (_T ("ufile01"), _T ("D:\\My Photo\\K-NET\\photo1.jpg"), CHttpClient::ParamFile) ;

    // Start a new request
    objHttpReq.BeginUpload (_T ("http://club.hooriza.com/cmd/box.html?clubid=1&amp;boxid=53&amp;action=store&amp;link=")) ;

    // Specifies the number of bytes to send when the Proceed method is called.
    const DWORD     cbProceed = 1024 ;  // 1K

    do {

        ...     // Place codes to report progress information to user.

    } while ( !(pobjHttpRes = objHttpReq.Proceed (cbProceed)) ) ;

    ...     // Place codes to handle the returned CHttpResponse object.

} catch (httpclientexception &amp;& e) {
    ...     // Place exception handling codes here.
}
\endcode


<h2>How to handle the returned CHttpResponse object</h2>
When you send a request using <code>CHttpClient</code>, all method will return <code>CHttpResponse</code> object.
<code>CHttpResponse</code> represents the response returned by a HTTP web server.
<code>CHttpResponse</code> provides following methods.
\code
// Returns the number of headers of which name is the szName
DWORD CHttpResponse::GetHeaderCount (PCSZ szName) throw (Exception &amp;) ;

// Returns the header of which name is the szName
PCSZ CHttpResponse::GetHeader (PCSZ szName, DWORD nIdx = 0) throw (Exception &amp;) ;

// Returns the HTTP status code returned by a HTTP server
DWORD CHttpResponse::GetStatus (void) throw (Exception &amp;) ;

// Returns the HTTP status text returned by a HTTP server
PCSZ CHttpResponse::GetStatusText (void) throw (Exception &amp;) ;

// Retrieves the length of the content returned by a HTTP server
BOOL CHttpResponse::GetContentLength (DWORD &amp; cbContLen) throw (Exception &amp;) ;

// Reads the content returned by a HTTP server
DWORD CHttpResponse::ReadContent (BYTE * pbyBuff, DWORD cbBuff) throw (Exception &amp;) ;
\endcode
The following code demonstrates the basic usage of the <code>CHttpResponse</code> object.
\code
using namespace Ryeol ;

CHttpResponse *     pobjHttpRes = NULL ;

try {
    // Get the CHttpResponse object
    pobjHttpRes = ... ;

    // Reads the HTTP status code
    _tprintf (_T ("%u"), pobjHttpRes-&gt;GetStatus ()) ;
    // Reads the HTTP status text
    _tprintf (_T (" %s\n"), pobjHttpRes-&gt;GetStatusText ()) ;

    // Reads HTTP headers using an array of header names
    static LPCTSTR      szHeaders[] = 
    { _T ("Server"), _T ("Date"), _T ("X-Powered-By"), _T ("Content-Length"), _T ("Set-Cookie")
    , _T ("Expires"), _T ("Cache-control"), _T ("Connection"), _T ("Transfer-Encoding")
    , _T ("Content-Type") } ;

    LPCTSTR     szHeader ;
    for (size_t i = 0; i < sizeof (szHeaders) / sizeof (LPCTSTR); i++) {
        if ( szHeader = pobjHttpRes-&gt;GetHeader (szHeaders[i]) )
            _tprintf (_T ("%s: %s\n"), szHeaders[i], szHeader) ;
        else
            // If the header is not found..
            _tprintf (_T ("'%s' header does not exist..\r\n"), szHeaders[i]) ;
    }

    _tprintf (_T ("\r\n")) ;

    // Checks whether the returned stream is a text
    BOOL        bIsText = FALSE ;
    if ( szHeader = pobjHttpRes-&gt;GetHeader (_T ("Content-Type")) )
        bIsText = (0 == ::_tcsncicmp (szHeader, _T ("text/"), 5)) ;

    // Reads the length of the stream
    DWORD       dwContSize ;
    // If the length is not specified
    if ( !pobjHttpRes-&gt;GetContentLength (dwContSize) )
        dwContSize = 0 ;

    const DWORD     cbBuff = 1024 * 10 ;
    BYTE            byBuff[cbBuff] ;
    DWORD           dwRead ;
    size_t          cbTotal = 0 ;

    // Reads the data stream returned by the HTTP server.
    while ( dwRead = pobjHttpRes-&gt;ReadContent (byBuff, cbBuff - 1) ) {
        cbTotal += dwRead ;

        if ( bIsText ) {
            byBuff[dwRead] = '\0' ;
            printf ("%s", reinterpret_cast&lt;LPCSTR&gt; (byBuff)) ;
        }
    }

    if ( !bIsText )
        _tprintf (_T ("%u bytes skipped..\n"), cbTotal) ;

} catch (httpclientexception & e) {
    ...     // Place exception handling codes here.
}

delete pobjHttpRes ;
pobjHttpRes = NULL ;
\endcode


<h2>How to handle exception</h2>
When an error occurred, <code>httpclientexception</code> object is thrown.
\code
class httpclientexception {
public:
    // Returns the last error code. The error codes is defined in RyeolHttpClient.h
    DWORD LastError (void) const throw () ;

    // Returns the last error message.
    LPCTSTR errmsg (void) const throw () ;

    // Returns the last win32 error code retrieved by using ::GetLastError when an error occurred.
    DWORD Win32LastError (void) const throw () ;
} ;
\endcode
Before throwing an exception, most methods restore their internal states. (all or nothing like transaction)
But if you call <code>BeginPost</code> or <code>BeginUpload</code> method, the POST context is automatically canceled.
You should write the following try-catch block to handle the exception.
\code
using namespace Ryeol ;

try {

    ...     // Place codes which throw a httpclientexception exception

} catch (httpclientexception &amp; e) {
    _tprintf (_T ("An error has been occured\n")) ;
    _tprintf (_T ("ErrCode: 0x%x\n"), e.LastError ()) ;
    _tprintf (_T ("ErrMsg: %s\n"), e.errmsg ()) ;
    if ( e.Win32LastError () != NO_ERROR ) {
        TCHAR       szErrMsg[512] ;
        GetWinInetErrMsg (szErrMsg, 512, e.Win32LastError ()) ;

        _tprintf (_T ("Win32ErrCode: 0x%x\n"), e.Win32LastError ()) ;
        _tprintf (_T ("Win32ErrMsg: %s\n"), szErrMsg) ;
    }
}
\endcode


<h2>How to show progress information to user</h2>
If you call <code>BeginPost</code> or <code>BeginUpload</code> method, you can retrieve progress information using <code>Query</code> method.
\code
// Queries progress information of the current POST context
// objPostStat      [out] A CHttpPostStat object.
void CHttpClient::Query (CHttpPostStat &amp; objPostStat) throw () ;
\endcode
<code>CHttpPostStat</code> represents progress information of the current POST context.
The following code demonstrates the basic usage of the <code>CHttpPostStat</code> object.
\code
using namespace Ryeol ;

CHttpClient         objHttpReq ;
CHttpResponse *     pobjHttpRes = NULL ;
size_t              cbProceed = 1024 ;  // 1k

try {
    ... ;   // Intialize the CHttpClient object

    // Starts a new POST request
    objHttpReq.BeginPost (...) or objHttpReq.BeginUpload (...) ;

    // Displays progress information
    CHttpPostStat       objPostStat ;

    do {
        // Retrieves progress information
        objHttpReq.Query (objPostStat) ;

        _tprintf (_T ("\nPost in progress... %2u/%2u\n")
            , objPostStat.PostedCount ()            // The number of posted parameters
            , objPostStat.TotalCount ()) ;          // The total number of parameters

        _tprintf (_T ("%s: %10u/%10u %10u/%10u %10u/%10u\n")
            , objPostStat.CurrParam ()              // The name of the current parameter
            , objPostStat.CurrParamPostedByte ()    // The number of posted bytes of the current parameter
            , objPostStat.CurrParamTotalByte ()     // The total number of bytes of the current parameter
            , objPostStat.PostedByte ()             // The number of posted bytes of the request
            , objPostStat.TotalByte ()              // The total number of bytes of the request
            , objPostStat.ActualPostedByte ()       // The actual number of posted bytes of the request
            , objPostStat.ActualTotalByte ()) ;     // The actual total number of bytes of the request

        // If the current parameter is a file parameter, displays the file path
        if ( objPostStat.CurrParamIsFile () )
            _tprintf (_T ("--&gt;%s\n")
                , objPostStat.CurrFile ()) ;

        // Sends the number of bytes specified by cbProceed to the server
    } while ( !(pobjHttpRes = objHttpReq.Proceed (cbProceed)) ) ;

    ... ;   // Handles the returned CHttpResponse object

} catch (httpclientexception &amp; e) {
    ...     // Place exception handling codes here.
}
\endcode
 
 * Copyright &copy; 2006 by <a href="mailto:hyeongryeol@gmail.com">Jo Hyeong-ryeol</a>\n
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 */
#pragma once

#include <windows.h>			// for generic types, .. etc
#include <wininet.h>			// for the windows internet functions
#include <crtdbg.h>				// for the _ASSERTE macro
#include <objbase.h>			// for ::CoCreateGuid

#include <utility>				// for the STL pair type
#include <map>					// for the STL multimap container
#include <vector>				// for the STL vector container

#include "RyeolException.h"		// for exception classes
#include "SafeInt.hpp"			// for the SaftInt class

#pragma warning (push)
#pragma warning (disable: 4290)	// avoids 'C++ Exception Specification ignored' message
#pragma warning (disable: 4996)	// avoids 'This function or variable may be unsafe' message

/*!
 * \brief	The namespace of the Ryeol's library
 *
 * This is the namespace for source codes written by Jo Hyeong-ryeol.
 */
namespace Ryeol {

///////////////////////////////////////// Global constant definitions /////////////////////////////////////////
/*!
 * \brief	Default flags used by the CHttpClient class to open a HTTP request
 * 
 * These flags are default flags used by the CHttpClient to open a HTTP request. These flags
 * are actually the dwFlags parameter of the ::HttpOpenRequest function of the WinInet API.
 * For more detailed information about this flags, see microsoft's SDK documentation.
 *
 * \sa CHttpClientT<HttpTool, HttpEncoder>::RequestGetEx
 * \sa CHttpClientT<HttpTool, HttpEncoder>::BeginPostEx
 * \sa CHttpClientT<HttpTool, HttpEncoder>::BeginUploadEx
 * \sa CHttpClientT<HttpTool, HttpEncoder>::RequestPostEx
 * \sa CHttpClientT<HttpTool, HttpEncoder>::RequestUploadEx
 */
enum HttpClientDefFlag
{
	HTTPCLIENT_DEF_REQUEST_FLAGS				= INTERNET_FLAG_HYPERLINK
												| INTERNET_FLAG_KEEP_CONNECTION
												| INTERNET_FLAG_NO_UI
												| INTERNET_FLAG_RESYNCHRONIZE
	//!< The default flag which causes the CHttpClient to use the cache if a cached copy exists.

	, HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE		= INTERNET_FLAG_HYPERLINK
												| INTERNET_FLAG_KEEP_CONNECTION
												| INTERNET_FLAG_NO_UI
												| INTERNET_FLAG_RESYNCHRONIZE
												| INTERNET_FLAG_NO_CACHE_WRITE
												| INTERNET_FLAG_PRAGMA_NOCACHE
												| INTERNET_FLAG_RELOAD
	//!< The default flag which causes the CHttpClient not to use the cache.
} ;


/*!
 * \brief	These error codes represent the error occurred while processing an operation.
 * 
 * These error codes are custom error codes only for classes in this file.
 */
enum HttpClientErrorCode
{
	HTTPCLIENT_ERR_NOT_SPECIFIED						= 0		//!< Error was not occurred or not specified.

	// Normal error
	, HTTPCLIENT_ERR_UNEXPECTED_ERROR					= 100	//!< Unknown error occurred.
	, HTTPCLIENT_ERR_OUT_OF_RANGE						= 101	//!< The index is out of range.
	, HTTPCLIENT_ERR_OUT_OF_MEMORY						= 102	//!< The memory has been exhausted.
	, HTTPCLIENT_ERR_INVALID_URL						= 103	//!< The requested URL is not a valid URL.
	, HTTPCLIENT_ERR_POST_NOT_STARTED					= 104	//!< The post context is not started yet.
	, HTTPCLIENT_ERR_READ_UNEXPECTED_SIZE				= 105	//!< Couldn't read expected bytes from a file.
	, HTTPCLIENT_ERR_POST_NOT_FINISHED					= 106	//!< The post context has not been finished yet.
	, HTTPCLIENT_ERR_INTERNET_PORT_NOT_VALID			= 107	//!< The port number is not valid.
	, HTTPCLIENT_ERR_STD_EXCEPTION						= 108	//!< std::exception occurred.
	, HTTPCLIENT_ERR_ENCODED_URL_NOT_VALID				= 109	//!< The encoded URL is not valid.
	, HTTPCLIENT_ERR_INVALID_UTF8_CHARACTER				= 110	//!< The UTF8 string contains an invalid character.
	, HTTPCLIENT_ERR_UNEXPECTED_ARITHMETIC_ERROR		= 111	//!< An unexpected arithmetic error has been occurred.
	, HTTPCLIENT_ERR_ARITHMETIC_OVERFLOW				= 112	//!< An arithmetic overflow error has been occurred.
	, HTTPCLIENT_ERR_INT_DIVIDE_BY_ZERO					= 113	//!< An interger divide by zero exception has been occurred.
	, HTTPCLIENT_ERR_FILE_ALEADY_EXISTS					= 114	//!< A file aleady exists. So it doesn't overwrite it.


	// Normal error (which has a win32 error code) - Reserved


	// WinInet error (which has a win32 error code)
	, HTTPCLIENT_ERR_QUERYINFO_FAILED					= 400	//!< ::HttpQueryInfo failed.
	, HTTPCLIENT_ERR_INTERNETREADFILE_FAILED			= 401	//!< ::InternetReadFile failed.
	, HTTPCLIENT_ERR_INTERNETOPEN_FAILED				= 402	//!< ::InternetOpen failed.
	, HTTPCLIENT_ERR_INTERNETCONNECT_FAILED				= 403	//!< ::InternetConnect failed.
	, HTTPCLIENT_ERR_HTTPOPENREQUEST_FAILED				= 404	//!< ::HttpOpenRequest failed.
	, HTTPCLIENT_ERR_HTTPADDREQUESTHEADERS_FAILED		= 405	//!< ::HttpAddRequestHeaders failed.
	, HTTPCLIENT_ERR_HTTPSENDREQUEST_FAILED				= 406	//!< ::HttpSendRequest failed.
	, HTTPCLIENT_ERR_HTTPSENDREQUESTEX_FAILED			= 407	//!< ::HttpSendRequestEx failed.
	, HTTPCLIENT_ERR_INTERNETWRITEFILE_FAILED			= 408	//!< ::InternetWriteFile failed.
	, HTTPCLIENT_ERR_HTTPENDREQUEST_FAILED				= 409	//!< ::HttpEndRequest failed.
	, HTTPCLIENT_ERR_INTERNETSETOPTION_FAILED			= 410	//!< ::InternetSetOption failed.

	// Win32 API error (which has a win32 error code)
	, HTTPCLIENT_ERR_WIDECHARTOMULTIBYTE_FAILED			= 600	//!< ::WideCharToMultiByte failed.
	, HTTPCLIENT_ERR_MULTIBYTETOWIDECHAR_FAILED			= 601	//!< ::MultiByteToWideChar failed.
	, HTTPCLIENT_ERR_READFILE_FAILED					= 602	//!< ::ReadFile failed.
	, HTTPCLIENT_ERR_OPENFILE_FAILED					= 603	//!< OpenFile (::CreateFile) failed.
	, HTTPCLIENT_ERR_SETFILEPOINTER_FAILED				= 604	//!< ::SetFilePointer failed.
	, HTTPCLIENT_ERR_GETFILESIZE_FAILED					= 605	//!< ::GetFileSize failed.
	, HTTPCLIENT_ERR_WRITEFILE_FAILED					= 606	//!< ::WriteFile failed.

	// user-defined error
	, HTTPCLIENT_ERR_USER								= 1000	//!< Beginning of the user-defined error code.
																//! \nThe maximum value is HTTPCLIENT_ERR_USER + 99.
} ;

template <typename HttpTool, typename HttpEncoder> class CHttpClientT ;

///////////////////////////////////////// Global constant definitions /////////////////////////////////////////

///////////////////////////////////////// httpclientexception /////////////////////////////////////////
/*!
 * \brief	The standard exception class used by classes in this file. (Ansi Ver.)
 *
 * This class represents an exception occurred in HTTP client classes. All classes in this file
 * will throw this class when an error has been occurred.
 */
class httpclientexceptionA : public errmsg_exceptionA {
public:
	/*! \brief	Default constructor */
	httpclientexceptionA (void) throw () ;
	/*! \brief	Constructor with initial arguments */
	httpclientexceptionA (LPCSTR szErrMsg, DWORD dwLastError = HTTPCLIENT_ERR_NOT_SPECIFIED, DWORD dwWin32LastError = NO_ERROR) throw () ;

	/*! \brief	Returns last error code
	 *
	 * This error code represents the error occurred in classes of this file.
	 */
	inline DWORD LastError (void) const throw ()
	{
		return m_dwLastError ;
	}

	/*! \brief	Sets the last error code
	 *
	 * This error code represents the error occurred in classes of this file.
	 */
	inline void SetLastError (DWORD dwErrCode) throw ()
	{
		m_dwLastError = dwErrCode ;
	}

	/*! \brief	Returns last win32 error code
	 *
	 * Returns the last win32 error code retrieved by using ::GetLastError when an error occurred.
	 */
	inline DWORD Win32LastError (void) const throw ()
	{
		return m_dwWin32LastError ;
	}

	/*! \brief	Sets the last win32 error code
	 *
	 * This method sets the last win32 error code retrieved by using ::GetLastError.
	 */
	inline void SetWin32LastError (DWORD dwErrCode) throw ()
	{
		m_dwWin32LastError = dwErrCode ;
	}

private:
	DWORD			m_dwLastError ;			//!< The last error code.
	DWORD			m_dwWin32LastError ;	//!< The last win32 error code.
} ;

/*!
 * \brief	The standard exception class used by classes in this file. (Unicode Ver.)
 *
 * This class represents an exception occurred in HTTP client classes. All classes in this file
 * will throw this class when an error has been occurred.
 */
class httpclientexceptionW : public errmsg_exceptionW {
public:
	/*! \brief	Default constructor */
	httpclientexceptionW (void) throw () ;
	/*! \brief	Constructor with initial arguments */
	httpclientexceptionW (LPCWSTR szErrMsg, DWORD dwLastError = HTTPCLIENT_ERR_NOT_SPECIFIED, DWORD dwWin32LastError = NO_ERROR) throw () ;

	/*!
	 * \brief	Returns "httpclientexceptionW"
	 *
	 * This is not supported in Unicode version.
	 * It always returns "httpclientexceptionW".
	 *
	 * \return				"httpclientexceptionW"
	 */
	inline LPCSTR what (void) const throw ()
	{
		return "httpclientexceptionW" ;
	}

	/*! \brief	Returns last error code
	 *
	 * This error code represents the error occurred in classes of this file.
	 */
	inline DWORD LastError (void) const throw ()
	{
		return m_dwLastError ;
	}

	/*! \brief	Sets the last error code
	 *
	 * This error code represents the error occurred in classes of this file.
	 */
	inline void SetLastError (DWORD dwErrCode) throw ()
	{
		m_dwLastError = dwErrCode ;
	}

	/*! \brief	Returns last win32 error code
	 *
	 * Returns the last win32 error code retrieved by using ::GetLastError when an error occurred.
	 */
	inline DWORD Win32LastError (void) const throw ()
	{
		return m_dwWin32LastError ;
	}

	/*! \brief	Sets the last win32 error code
	 *
	 * This method sets the last win32 error code retrieved by using ::GetLastError.
	 */
	inline void SetWin32LastError (DWORD dwErrCode) throw ()
	{
		m_dwWin32LastError = dwErrCode ;
	}

private:
	DWORD			m_dwLastError ;			//!< The last error code.
	DWORD			m_dwWin32LastError ;	//!< The last win32 error code.
} ;

#ifdef UNICODE
	typedef httpclientexceptionW		httpclientexception ;
#else
	typedef httpclientexceptionA		httpclientexception ;
#endif

///////////////////////////////////////// httpclientexception /////////////////////////////////////////


///////////////////////////////////////// CHttpToolA /////////////////////////////////////////
/*!
 * \internal
 * \brief	This class contains utility methods. (Ansi Ver.)
 *
 * This class provides some utility methods and gives character type independence.
 * (Internal use only)
 */
class CHttpToolA
{
public:
	// Returns constant messages
	static inline LPCSTR GetConstMessage (DWORD nIdx) throw () ;

	// Methods related to the exception
	typedef	httpclientexceptionA			Exception ;
	static void ThrowException (DWORD nErrMsgIdx) throw (Exception &) ;
	static void ThrowException (LPCSTR szErrMsg, DWORD nErrMsgIdx = HTTPCLIENT_ERR_NOT_SPECIFIED) throw (Exception &) ;
	static void ThrowException (DWORD nErrMsgIdx, DWORD dwErrCode, LPCSTR szStrArg = NULL) throw (Exception &) ;
	static void ThrowException (LPCWSTR szErrMsg, DWORD nErrMsgIdx = HTTPCLIENT_ERR_NOT_SPECIFIED, DWORD dwErrCode = NO_ERROR) throw (Exception &) ;
	static void ThrowException (httpclientexceptionW & e) throw (Exception &) ;
	static void ThrowException (::SafeIntException & e) throw (Exception &) ;

	// String type definitions =======================================================
	typedef CHAR				CharType ;
	typedef LPSTR				PSZ ;
	typedef LPCSTR				PCSZ ;

	static inline BOOL IsAnsi (void) throw ()
	{
		return TRUE ;
	}

	// Wrapper methods for CRT string functions
	static inline size_t StringLen (LPCSTR szStr) throw ()
	{
		return ::strlen (szStr) ;
	}

	static inline LPSTR StringCopy (LPSTR szDest, LPCSTR szSrc) throw ()
	{
		return ::strcpy (szDest, szSrc) ;
	}

	static inline LPSTR StringCat (LPSTR szDest, LPCSTR szSrc) throw ()
	{
		return ::strcat (szDest, szSrc) ;
	}

	static inline LPSTR StringNCopy (LPSTR szDest, LPCSTR szSrc, size_t count) throw ()
	{
		return ::strncpy (szDest, szSrc, count) ;
	}

	static inline int StringNICmp (LPCSTR szStr1, LPCSTR szStr2, size_t count) throw ()
	{
		return ::_strnicmp (szStr1, szStr2, count) ;
	}

	static inline int StringCmp (LPCSTR szStr1, LPCSTR szStr2) throw ()
	{
		return ::strcmp (szStr1, szStr2) ;
	}

	static inline LPCSTR StringChr (LPCSTR szStr, int Chr) throw ()
	{
		return ::strchr (szStr, Chr) ;
	}

	static inline LPCSTR StringRChr (LPCSTR szStr, int Chr) throw ()
	{
		return ::strrchr (szStr, Chr) ;
	}

	static int SNPrintf (LPSTR buffer, size_t count, LPCSTR format, ...) throw ()
	{
		int			cchWritten ;
		va_list		args ;

		va_start (args, format) ;
		cchWritten = ::_vsnprintf (buffer, count, format, args) ;
		va_end (args) ;
		return cchWritten ;
	}

	static PCSTR StringStr (PCSTR szString, PCSTR szSearch) throw ()
	{
		return ::strstr (szString, szSearch) ;
	}

	static unsigned long StringToUL (PCSTR szString, PSTR * pEndPtr, int base) throw ()
	{
		return ::strtoul (szString, pEndPtr, base) ;
	}

	// Conversion methods
	static LPSTR Unicode2Ansi (LPCWSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;
	static LPWSTR Ansi2Unicode (LPCSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;

	// Methods for Ansi character set
	static BOOL IsDBCSLeadByteEx (UINT CodePage, CHAR TestChar) throw ()
	{
		return ::IsDBCSLeadByteEx (CodePage, TestChar) ;
	}

	// comparison functor operator (used by STL multimap)
	bool operator () (LPCSTR szKey1, LPCSTR szKey2) const throw () ;

	// Constants related to HTTP
	static LPCSTR							szDefUsrAgent ;
	static LPCSTR							szGET ;
	static LPCSTR							szPost ;
	static LPCSTR							szHTTP ;
	static LPCSTR							szHTTPS ;
	static LPCSTR							szSlash ;
	static LPCSTR							szCacheControl ;
	static LPCSTR							szNoCache ;
	static LPCSTR							szContentType ;
	static LPCSTR							szFormUrlEncoded ;
	static LPCSTR							szMultipartFormDataBoundary ;
	static LPCSTR							szDefBoundary ;
	static LPCSTR							szDefUploadContType ;
	static LPCSTR							szNULL ;
	static LPCSTR							szEmptyString ;
	static LPCSTR							szColonSlashSlash ;

	// Methods related to HTTP
	static HINTERNET OpenInternet (LPCSTR szUserAgent, DWORD dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG, LPCSTR szProxyName = NULL, LPCSTR szProxyBypass = NULL, DWORD dwFlags = 0) throw (Exception &) ;
	static void CloseInternet (HINTERNET & hInternet) throw () ;
	static HINTERNET OpenConnection (HINTERNET hInternet, LPCSTR szServerAddr, INTERNET_PORT nPort = INTERNET_DEFAULT_HTTP_PORT, LPCSTR szUsrName = NULL, LPCSTR szUsrPwd = NULL) throw (Exception &) ;
	static void CloseConnection (HINTERNET & hConnection) throw () ;
	static HINTERNET OpenRequest (HINTERNET hConnection, LPCSTR szMethod, LPCSTR szObjectName, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, LPCSTR szReferer = NULL, UINT CodePage = CP_ACP) throw (Exception &) ;
	static void CloseRequest (HINTERNET & hRequest) throw () ;
	static void AddHeader (HINTERNET hRequest, LPCSTR szName, LPCSTR szValue, UINT CodePage = CP_ACP) throw (Exception &) ;
	static void SendRequest (HINTERNET hRequest, LPCSTR szPosted = NULL, UINT CodePage = CP_ACP) throw (Exception &) ;
	static void SendRequestEx (HINTERNET hRequest, DWORD dwPostedSize) throw (Exception &) ;
	static void InternetWriteFile (HINTERNET hRequest, const BYTE * pbyBuff, DWORD cbyBuff) throw (Exception &) ;
	static void EndRequest (HINTERNET hRequest) throw (Exception &) ;
	static BOOL FileExists (LPCSTR szFilePath) throw (Exception &) ;
	static HANDLE OpenFile (LPCSTR szFilePath) throw (Exception &) ;
	static HANDLE CreateFileAlwaysToWrite (LPCSTR szFilePath) throw (Exception &) ;
	static DWORD GetFileSize (HANDLE hFile, LPCSTR szFilePath) throw (Exception &) ;
	static LPSTR GetMimeType (HANDLE hFile, UINT CodePage = CP_ACP) throw (Exception &) ;
	static LPSTR GetStatusText (HINTERNET hRequest) throw (Exception &) ;
	static LPSTR GetHeader (HINTERNET hRequest, LPCSTR szName, DWORD * pnIdx = NULL) throw (Exception &) ;
	static void InternetSetOption (HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength) throw (Exception &) ;
	static LPSTR CreateUploadBoundary (void) throw () ;
} ;
///////////////////////////////////////// CHttpToolA /////////////////////////////////////////


///////////////////////////////////////// CHttpToolW /////////////////////////////////////////
/*!
 * \internal
 * \brief	This class contains utility methods. (Unicode Ver.)
 *
 * This class provides some utility methods and gives character type independence.
 * (Internal use only)
 */
class CHttpToolW
{
public:
	// Returns constant messages
	static inline LPCWSTR GetConstMessage (int nIdx) throw () ;

	// Methods related to the exception
	typedef	httpclientexceptionW			Exception ;
	static void ThrowException (DWORD nErrMsgIdx) throw (Exception &) ;
	static void ThrowException (LPCWSTR szErrMsg, DWORD nErrMsgIdx = HTTPCLIENT_ERR_NOT_SPECIFIED) throw (Exception &) ;
	static void ThrowException (DWORD nErrMsgIdx, DWORD dwErrCode, LPCWSTR szStrArg = NULL) throw (Exception &) ;
	static void ThrowException (LPCSTR szErrMsg, DWORD nErrMsgIdx = HTTPCLIENT_ERR_NOT_SPECIFIED, DWORD dwErrCode = NO_ERROR) throw (Exception &) ;
	static void ThrowException (httpclientexceptionA & e) throw (Exception &) ;
	static void ThrowException (::SafeIntException & e) throw (Exception &) ;

	// String type definitions =======================================================
	typedef WCHAR				CharType ;
	typedef LPWSTR				PSZ ;
	typedef LPCWSTR				PCSZ ;

	static inline BOOL IsAnsi (void) throw ()
	{
		return FALSE ;
	}

	static inline size_t StringLen (LPCWSTR szStr) throw ()
	{
		return ::wcslen (szStr) ;
	}

	static inline LPWSTR StringCopy (LPWSTR szDest, LPCWSTR szSrc) throw ()
	{
		return ::wcscpy (szDest, szSrc) ;
	}

	static inline LPWSTR StringCat (LPWSTR szDest, LPCWSTR szSrc) throw ()
	{
		return ::wcscat (szDest, szSrc) ;
	}

	static inline LPWSTR StringNCopy (LPWSTR szDest, LPCWSTR szSrc, size_t count) throw ()
	{
		return ::wcsncpy (szDest, szSrc, count) ;
	}

	static inline int StringNICmp (LPCWSTR szStr1, LPCWSTR szStr2, size_t count) throw ()
	{
		return ::_wcsnicmp (szStr1, szStr2, count) ;
	}

	static inline int StringCmp (LPCWSTR szStr1, LPCWSTR szStr2) throw ()
	{
		return ::wcscmp (szStr1, szStr2) ;
	}

	static inline LPCWSTR StringChr (LPCWSTR szStr, WCHAR Chr) throw ()
	{
		return ::wcschr (szStr, Chr) ;
	}

	static inline LPCWSTR StringRChr (LPCWSTR szStr, WCHAR Chr) throw ()
	{
		return ::wcsrchr (szStr, Chr) ;
	}

	static int SNPrintf (LPWSTR buffer, size_t count, LPCWSTR format, ...) throw ()
	{
		int			cchWritten ;
		va_list		args ;

		va_start (args, format) ;
		cchWritten = ::_vsnwprintf (buffer, count, format, args) ;
		va_end (args) ;
		return cchWritten ;
	}

	static PCWSTR StringStr (PCWSTR szString, PCWSTR szSearch) throw ()
	{
		return ::wcsstr (szString, szSearch) ;
	}

	static unsigned long StringToUL (PCWSTR szString, PWSTR * pEndPtr, int base) throw ()
	{
		return ::wcstoul (szString, pEndPtr, base) ;
	}

	// Conversion methods
	static LPSTR Unicode2Ansi (LPCWSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;
	static LPWSTR Ansi2Unicode (LPCSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;

	// Methods for Ansi character set
	static BOOL IsDBCSLeadByteEx (UINT /* CodePage */, WCHAR /* TestChar */) throw ()
	{
		return FALSE ;		// always returns FALSE
	}

	// comparison functor operator (used by STL multimap)
	bool operator () (LPCWSTR szKey1, LPCWSTR szKey2) const throw () ;

	// Constants related to HTTP
	static LPCWSTR							szDefUsrAgent ;
	static LPCWSTR							szGET ;
	static LPCWSTR							szPost ;
	static LPCWSTR							szHTTP ;
	static LPCWSTR							szHTTPS ;
	static LPCWSTR							szSlash ;
	static LPCWSTR							szCacheControl ;
	static LPCWSTR							szNoCache ;
	static LPCWSTR							szContentType ;
	static LPCWSTR							szFormUrlEncoded ;
	static LPCWSTR							szMultipartFormDataBoundary ;
	static LPCWSTR							szDefBoundary ;
	static LPCWSTR							szDefUploadContType ;
	static LPCWSTR							szNULL ;
	static LPCWSTR							szEmptyString ;
	static LPCWSTR							szColonSlashSlash ;

	// Methods related to HTTP
	static HINTERNET OpenInternet (LPCWSTR szUserAgent, DWORD dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG, LPCWSTR szProxyName = NULL, LPCWSTR szProxyBypass = NULL, DWORD dwFlags = 0) throw (Exception &) ;
	static void CloseInternet (HINTERNET & hInternet) throw () ;
	static HINTERNET OpenConnection (HINTERNET hInternet, LPCWSTR szServerAddr, INTERNET_PORT nPort = INTERNET_DEFAULT_HTTP_PORT, LPCWSTR szUsrName = NULL, LPCWSTR szUsrPwd = NULL) throw (Exception &) ;
	static void CloseConnection (HINTERNET & hConnection) throw () ;
	static HINTERNET OpenRequest (HINTERNET hConnection, LPCWSTR szMethod, LPCWSTR szObjectName, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, LPCWSTR szReferer = NULL, UINT CodePage = CP_ACP) throw (Exception &) ;
	static void CloseRequest (HINTERNET & hRequest) throw () ;
	static void AddHeader (HINTERNET hRequest, LPCWSTR szName, LPCWSTR szValue, UINT CodePage = CP_ACP) throw (Exception &) ;
	static void SendRequest (HINTERNET hRequest, LPCWSTR szPosted = NULL, UINT CodePage = CP_ACP) throw (Exception &) ;
	static void SendRequestEx (HINTERNET hRequest, DWORD dwPostedSize) throw (Exception &) ;
	static void InternetWriteFile (HINTERNET hRequest, const BYTE * pbyBuff, DWORD cbyBuff) throw (Exception &) ;
	static void EndRequest (HINTERNET hRequest) throw (Exception &) ;
	static BOOL FileExists (LPCWSTR szFilePath) throw (Exception &) ;
	static HANDLE OpenFile (LPCWSTR szFilePath) throw (Exception &) ;
	static HANDLE CreateFileAlwaysToWrite (LPCWSTR szFilePath) throw (Exception &) ;
	static DWORD GetFileSize (HANDLE hFile, LPCWSTR szFilePath) throw (Exception &) ;
	static LPSTR GetMimeType (HANDLE hFile, UINT CodePage = CP_ACP) throw (Exception &) ;
	static LPWSTR GetStatusText (HINTERNET hRequest) throw (Exception &) ;
	static LPWSTR GetHeader (HINTERNET hRequest, LPCWSTR szName, DWORD * pnIdx = NULL) throw (Exception &) ;
	static void InternetSetOption (HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength) throw (Exception &) ;
	static LPWSTR CreateUploadBoundary (void) throw () ;
} ;
///////////////////////////////////////// CHttpToolW /////////////////////////////////////////


///////////////////////////////////////// CHttpClientMapT /////////////////////////////////////////
/*!
 * \internal
 * \brief	This is a private map for classes in this file.
 *
 * This is a private map for classes in this file. It uses STL's multimap.
 * If a method's return value is a string or contains a string, it guarantees that
 * it does not return NULL for string. (It returns "" instead of NULL)
 * It returns NULL only if an error occurred or the specified element is not found.
 */
template <typename HttpTool>
class CHttpClientMapT
{
public:
	// Basic type definitions ====================================================
	typedef typename HttpTool::Exception		Exception ;
	typedef typename HttpTool::CharType			CharType ;
	typedef typename HttpTool::PSZ				PSZ ;
	typedef typename HttpTool::PCSZ				PCSZ ;

	CHttpClientMapT (void) throw () ;
	virtual ~CHttpClientMapT (void) throw () ;

	// MapValue
	typedef struct _MapValue {
		PCSZ		szValue ;
		DWORD		dwFlag ;
		void Delete (void) throw () {
			::free ((void *) szValue) ;
			szValue = NULL ;
		}
	} MapValue ;

	typedef typename std::multimap<PCSZ, MapValue, HttpTool>::const_iterator	ConstMapIter ;

	BOOL Clear (void) throw () ;
	BOOL Remove (DWORD nIdx) throw () ;
	BOOL RemoveAll (PCSZ szName) throw (Exception &) ;
	BOOL Remove (PCSZ szName, DWORD nIdx = 0) throw (Exception &) ;

	BOOL Exists (PCSZ szName, DWORD nIdx = 0) throw (Exception &) ;
	DWORD Count (PCSZ szName = NULL) throw () ;
	BOOL Empty (void) const throw () ;

	PCSZ GetKey (DWORD nIdx) throw () ;

	MapValue Get (DWORD nIdx) throw (Exception &) ;
	PCSZ GetValue (DWORD nIdx) throw () ;
	DWORD GetFlag (DWORD nIdx) throw () ;

	MapValue Get (PCSZ szName, DWORD nIdx = 0) throw () ;
	PCSZ GetValue (PCSZ szName, DWORD nIdx = 0) throw () ;
	DWORD GetFlag (PCSZ szName, DWORD nIdx = 0) throw () ;

	void AddPointerDirectly (PSZ szName, PSZ szValue = NULL, BOOL dwFlag = 0) throw (Exception &) ;
	void Add (PCSZ szName, PCSZ szValue = NULL, BOOL dwFlag = 0) throw (Exception &) ;
	void Set (PCSZ szName, PCSZ szValue = NULL, BOOL dwFlag = 0, DWORD nIdx = 0) throw (Exception &) ;

	ConstMapIter Begin () const throw () ;
	ConstMapIter End () const throw () ;

private:
	// Type definitions for map ===================================================
	typedef typename std::multimap<PCSZ, MapValue, HttpTool>			Map ;
	typedef typename std::multimap<PCSZ, MapValue, HttpTool>::iterator	MapIter ;
	typedef typename std::multimap<PCSZ, MapValue, HttpTool>::size_type	MapSizeType ;
	typedef std::pair<PCSZ, MapValue>									MapItem ;
	Map																	m_map ;
} ;

typedef CHttpClientMapT<CHttpToolA>		CHttpClientMapA ;
typedef CHttpClientMapT<CHttpToolW>		CHttpClientMapW ;

#ifdef UNICODE
	typedef CHttpClientMapW		CHttpClientMap ;
#else
	typedef CHttpClientMapA		CHttpClientMap ;
#endif
///////////////////////////////////////// CHttpClientMapT /////////////////////////////////////////


///////////////////////////////////////// CHttpEncoderT /////////////////////////////////////////
/*!
 * \brief	This class encodes or decodes a string to use in HTTP operation. (Ansi version)
 *
 * This class supports various encoding and decoding methods which can be used in various HTTP operations.
 */
class CHttpEncoderA
{
public:
	typedef httpclientexceptionA				Exception ;	//!< typedef of httpclientexceptionA

	/*! \brief	Returns the number of bytes required to make an Ansi string from a string. */
	static DWORD AnsiEncodeLen (PCSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Encodes a string using Ansi character set. */
	static PSTR AnsiEncode (PSTR szBuff, PCSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the number of bytes required to make a string from an Ansi string. */
	static DWORD AnsiDecodeLen (PCSTR szEncoded, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Decodes an Ansi string to make a string. */
	static PSTR AnsiDecode (PSTR szBuff, PCSTR szEncoded, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the number of bytes required to encode a string in UTF-8. */
	static DWORD Utf8EncodeLen (PCSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Encodes a string in UTF-8. */
	static PSTR Utf8Encode (PSTR szBuff, PCSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the number of bytes required to decode a UTF-8 string. */
	static DWORD Utf8DecodeLen (PCSTR szEncoded, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Decodes an UTF-8 string to make a string. */
	static PSTR Utf8Decode (PSTR szBuff, PCSTR szEncoded, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the number of bytes required to encode an Ansi string using URL encoding. */
	static DWORD UrlEncodeLenA (PCSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*!
	 * \brief	Returns the number of unicode characters required to encode an Ansi string using URL encoding.
	 *
	 * This method returns the number of unicode characters required to make a URL-encoded string
	 * (in Unicode character set) from an Ansi string.
	 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
	 * The returned value does not include a terminating NULL character.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szStr			[in] A string which is encoded.
	 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
	 *						     and that string is used to make a URL-encoded string.
	 * \param CodePage		[in] A code page of the szStr parameter.
	 * \return				The number of unicode characters required. (Not including a terminating NULL character)
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline DWORD UrlEncodeLenW (PCSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{return UrlEncodeLenA (szStr, bUtf8Encoding, CodePage) ;}
	/*!
	 * \brief	Returns the number of bytes required to encode an Ansi string using URL encoding.
	 *
	 * This method returns the number of bytes required to make a URL-encoded string
	 * (in Ansi character set) from an Ansi string.
	 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
	 * The returned value does not include a terminating NULL character.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szStr			[in] A string which is encoded.
	 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
	 *						     and that string is used to make a URL-encoded string.
	 * \param CodePage		[in] A code page of the szStr parameter.
	 * \return				The number of bytes required. (Not including a terminating NULL character)
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline DWORD UrlEncodeLen (PCSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{
		return UrlEncodeLenA (szStr, bUtf8Encoding, CodePage) ;
	}

	/*! \brief	Encodes a string using URL encoding. */
	static PSTR UrlEncodeA (PSTR szBuff, PCSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Encodes a string using URL encoding. */
	static PWSTR UrlEncodeW (PWSTR szBuff, PCSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*!
	 * \brief	Encodes a string using URL encoding.
	 * 
	 * This method encodes a Ansi string using URL encoding.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
	 * \param szStr			[in] A string which is encoded.
	 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
	 *						     and that string is used to make a URL-encoded string.
	 * \param CodePage		[in] A code page of the szStr parameter.
	 * \return				An encoded string.
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline PSTR UrlEncode (PSTR szBuff, PCSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{
		return UrlEncodeA (szBuff, szStr, bUtf8Encoding, CodePage) ;
	}

	/*! \brief	Returns the number of bytes required to decode an URL-encoded string. */
	static DWORD UrlDecodeLenA (PCSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Returns the number of unicode characters required to decode an URL-encoded string. */
	static DWORD UrlDecodeLenW (PCSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*!
	 * \brief	Returns the number of bytes required to decode an URL-encoded string.
	 *
	 * This method returns the number of bytes required to decode an URL-encoded string. (Ansi version)
	 * The returned value does not include a terminating NULL character.
	 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szEncoded		[in] A string to decode.
	 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
	 *						     So the decoded string is converted into an Ansi string.
	 * \param CodePage		[in] A code page of the decoded string.
	 * \return				The number of bytes required. (Not including a terminating NULL character)
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline DWORD UrlDecodeLen (PCSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{
		return UrlDecodeLenA (szEncoded, bUtf8Encoding, CodePage) ;
	}

	/*! \brief	Decodes an URL-encoded string. */
	static PSTR UrlDecodeA (PSTR szBuff, PCSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Decodes an URL-encoded string. */
	static PWSTR UrlDecodeW (PWSTR szBuff, PCSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*!
	 * \brief	Decodes an URL-encoded string.
	 *
	 * This method decodes an URL-encoded string.
	 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
	 * \param szEncoded		[in] A string to decode.
	 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
	 *						     So the decoded string is converted into an Ansi string.
	 * \param CodePage		[in] A code page of the decoded string.
	 * \return				A decoded string.
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline PSTR UrlDecode (PSTR szBuff, PCSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{
		return UrlDecodeA (szBuff, szEncoded, bUtf8Encoding, CodePage) ;
	}

private:
	/*! \internal \brief	Converts an Ansi character into an UTF-8 character. */
	static void _AnsiCharToUtf8Char (PSTR szUtf8Char, PCSTR szAnsiChar, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \internal \brief	Converts an UTF-8 character into an Ansi character. */
	static void _Utf8CharToAnsiChar (PSTR szAnsiChar, PCSTR szUtf8Char, UINT CodePage = CP_ACP) throw (Exception &) ;
} ;

/*!
 * \brief	This class encodes or decodes a string to use in HTTP operation. (Unicode version)
 *
 * This class supports various encoding and decoding methods to use in various HTTP operations.
 */
class CHttpEncoderW
{
public:
	typedef httpclientexceptionW				Exception ;	//!< typedef of httpclientexceptionW

	/*! \brief	Returns the number of bytes required to make an Ansi string from a string. */
	static DWORD AnsiEncodeLen (PCWSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Encodes a string using Ansi character set. */
	static PSTR AnsiEncode (PSTR szBuff, PCWSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the number of unicode characters required to make a string from an Ansi string. */
	static DWORD AnsiDecodeLen (PCSTR szEncoded, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Decodes an Ansi string to make a string. */
	static PWSTR AnsiDecode (PWSTR szBuff, PCSTR szEncoded, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the number of characters required to encode a string in UTF-8. */
	static DWORD Utf8EncodeLen (PCWSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Encodes a string using UTF-8 encoding. */
	static PSTR Utf8Encode (PSTR szBuff, PCWSTR szStr, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the number of characters required to decode an UTF-8 string. */
	static DWORD Utf8DecodeLen (PCSTR szEncoded, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Decodes an UTF-8 string. */
	static PWSTR Utf8Decode (PWSTR szBuff, PCSTR szEncoded, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the number of bytes required to encode an Unicode string using URL encoding. */
	static DWORD UrlEncodeLenA (PCWSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*!
	 * \brief	Returns the number of unicode characters required to encode an Unicode string using URL encoding.
	 *
	 * This method returns the number of unicode characters required to make a URL-encoded string
	 * (in Unicode character set) from an Unicode string.
	 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
	 * The returned value does not include a terminating NULL character.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szStr			[in] A string which is encoded.
	 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
	 *						     and that string is used to make a URL-encoded string.
	 * \param CodePage		[in] A code page of the encoded string.
	 * \return				The number of unicode characters required. (Not including a terminating NULL character)
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline DWORD UrlEncodeLenW (PCWSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{return UrlEncodeLenA (szStr, bUtf8Encoding, CodePage) ;}
	/*!
	 * \brief	Returns the number of unicode characters required to encode an Unicode string using URL encoding.
	 *
	 * This method returns the number of unicode characters required to make a URL-encoded string
	 * (in Unicode character set) from an Unicode string.
	 * The URL-encoded string is a string that is safe to transmit from the Web server to a client.
	 * The returned value does not include a terminating NULL character.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szStr			[in] A string which is encoded.
	 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
	 *						     and that string is used to make a URL-encoded string.
	 * \param CodePage		[in] A code page of the encoded string.
	 * \return				The number of unicode characters required. (Not including a terminating NULL character)
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline DWORD UrlEncodeLen (PCWSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{
		return UrlEncodeLenW (szStr, bUtf8Encoding, CodePage) ;
	}

	/*! \brief	Encodes a string using URL encoding. */
	static PSTR UrlEncodeA (PSTR szBuff, PCWSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Encodes a string using URL encoding. */
	static PWSTR UrlEncodeW (PWSTR szBuff, PCWSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*!
	 * \brief	Encodes a string using URL encoding.
	 *
	 * This method encodes a Unicode string using URL encoding.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szBuff		[out] A buffer to save the encoded string. The buffer can not be NULL.
	 * \param szStr			[in] A string which is encoded.
	 * \param bUtf8Encoding	[in] If this is TRUE, the string is encoded using UTF-8 encoding
	 *						     and that string is used to make a URL-encoded string.
	 * \param CodePage		[in] A code page of the encoded string.
	 * \return				An encoded string.
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline PWSTR UrlEncode (PWSTR szBuff, PCWSTR szStr, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{
		return UrlEncodeW (szBuff, szStr, bUtf8Encoding, CodePage) ;
	}

	/*! \brief	Returns the number of bytes required to decode an URL-encoded string. */
	static DWORD UrlDecodeLenA (PCWSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Returns the number of unicode characters required to decode an URL-encoded string. */
	static DWORD UrlDecodeLenW (PCWSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*!
	 * \brief	Returns the number of unicode characters required to decode an URL-encoded string.
	 *
	 * This method returns the number of unicode characters required to decode an URL-encoded string.
	 * The returned value does not include a terminating NULL character.
	 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szEncoded		[in] A string to decode.
	 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
	 *						     So the decoded string is converted into an Unicode string.
	 * \param CodePage		[in] A code page of the decoded string.
	 * \return				The number of unicode characters required. (Not including a terminating NULL character)
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline DWORD UrlDecodeLen (PCWSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{
		return UrlDecodeLenW (szEncoded, bUtf8Encoding, CodePage) ;
	}

	/*! \brief	Decodes an URL-encoded string. */
	static PSTR UrlDecodeA (PSTR szBuff, PCWSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Decodes an URL-encoded string. */
	static PWSTR UrlDecodeW (PWSTR szBuff, PCWSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*!
	 * \brief	Decodes an URL-encoded string.
	 *
	 * This method decodes an URL-encoded string.
	 * This method does not support the URL-encoded string which contains a unicode character by using the %u or %x prefix.
	 * For more infomation about the Code-Page Identifiers, see the MSDN documentation.
	 *
	 * \param szBuff		[out] A buffer to save the decoded string. The buffer can not be NULL.
	 * \param szEncoded		[in] A string to decode.
	 * \param bUtf8Encoding	[in] If this is TRUE, the decoded string is assumed an UTF-8 string.
	 *						     So the decoded string is converted into an Unicode string.
	 * \param CodePage		[in] A code page of the decoded string.
	 * \return				A decoded string.
	 * \throw				Throws a httpclientexception if an error occurred.
	 */
	static inline PWSTR UrlDecode (PWSTR szBuff, PCWSTR szEncoded, BOOL bUtf8Encoding = FALSE, UINT CodePage = CP_ACP) throw (Exception &)
	{
		return UrlDecodeW (szBuff, szEncoded, bUtf8Encoding, CodePage) ;
	}

private:
	static void _Utf8CharToAnsiChar (PSTR szAnsiChar, PCSTR szUtf8Char, UINT CodePage = CP_ACP) throw (Exception &) ;
} ;

#ifdef UNICODE
	typedef CHttpEncoderW		CHttpEncoder ;
#else
	typedef CHttpEncoderA		CHttpEncoder ;
#endif

///////////////////////////////////////// CHttpEncoderT /////////////////////////////////////////


///////////////////////////////////////// CHttpResponseT /////////////////////////////////////////
/*!
 * \brief	This class represents a response returned by a HTTP web server.
 *
 * This class provides functionalities to handle a response which is returned by a HTTP web server.
 * An instance of This class is returned if the request methods of the CHttpClientT class succeeds.
 *
 * \sa		CHttpClientT, CHttpPostStatT
 *
 * The following code sample demonstrates the usage of the CHttpResponse class.
 * \code

...

using namespace Ryeol ;

...

CHttpResponse *		pobjHttpRes = NULL ;

try {
	// Get the CHttpResponse object
	pobjHttpRes = ... ;

	// Reads the HTTP status code
	_tprintf (_T ("%u"), pobjHttpRes->GetStatus ()) ;
	// Reads the HTTP status text
	_tprintf (_T (" %s\n"), pobjHttpRes->GetStatusText ()) ;

	// Reads HTTP headers using an array of header names
	static LPCTSTR		szHeaders[] = 
	{ _T ("Server"), _T ("Date"), _T ("X-Powered-By"), _T ("Content-Length"), _T ("Set-Cookie")
	, _T ("Expires"), _T ("Cache-control"), _T ("Connection"), _T ("Transfer-Encoding")
	, _T ("Content-Type") } ;

	LPCTSTR		szHeader ;
	for (size_t i = 0; i < sizeof (szHeaders) / sizeof (LPCTSTR); i++) {
		if ( szHeader = pobjHttpRes->GetHeader (szHeaders[i]) )
			_tprintf (_T ("%s: %s\r\n"), szHeaders[i], szHeader) ;
		else
			// If the header is not found..
			_tprintf (_T ("'%s' header does not exist..\r\n"), szHeaders[i]) ;
	}

	_tprintf (_T ("\r\n")) ;

	// Checks whether the returned stream is a text
	BOOL		bIsText = FALSE ;
	if ( szHeader = pobjHttpRes->GetHeader (_T ("Content-Type")) )
		bIsText = (0 == ::_tcsncicmp (szHeader, _T ("text/"), 5)) ;

	// Reads the length of the stream
	DWORD		dwContSize ;
	// If the length is not specified
	if ( !pobjHttpRes->GetContentLength (dwContSize) )
		dwContSize = 0 ;

	const DWORD		cbBuff = 1024 * 10 ;
	BYTE			byBuff[cbBuff] ;
	DWORD			dwRead ;
	size_t			cbTotal = 0 ;

	// Reads the data stream returned by the HTTP server.
	while ( dwRead = pobjHttpRes->ReadContent (byBuff, cbBuff - 1) ) {
		cbTotal += dwRead ;

		if ( bIsText ) {
			byBuff[dwRead] = '\0' ;
			printf ("%s", reinterpret_cast<LPCSTR> (byBuff)) ;
		}
	}

	if ( !bIsText )
		_tprintf (_T ("%u bytes skipped..\r\n"), cbTotal) ;

} catch (httpclientexception & e) {
	_tprintf (_T ("An error has been occurred\n")) ;
	_tprintf (_T ("ErrCode: 0x%x Win32ErrCode: 0x%x\n"), e.LastError (), e.Win32LastError ()) ;
	_tprintf (_T ("ErrMsg: %s\n"), e.errmsg ()) ;
}

delete pobjHttpRes ;
pobjHttpRes = NULL ;

 * \endcode
 */
template <typename HttpTool>
class CHttpResponseT
{
public:
	// Basic type definitions ====================================================
	typedef typename HttpTool::Exception		Exception ;	//!< typedef of httpclientexception
	typedef typename HttpTool::CharType			CharType ;	//!< typedef of character type
	typedef typename HttpTool::PSZ				PSZ ;		//!< typedef of null-terminated string
	typedef typename HttpTool::PCSZ				PCSZ ;		//!< typedef of constant null-terminated string

	/*! \brief	Constructor with three internet handles */
	CHttpResponseT (HINTERNET hInternet, HINTERNET hConnection, HINTERNET hRequest) throw () ;
	/*! \brief	Constructor with two internet handles */
	CHttpResponseT (HINTERNET hConnection, HINTERNET hRequest) throw () ;
	/*! \brief	Constructor with one internet handles */
	CHttpResponseT (HINTERNET hRequest) throw () ;

	/*! \brief	Default destructor */
	virtual ~CHttpResponseT (void) throw () ;

	/*! \brief	Returns the number of headers of which name is szName */
	DWORD GetHeaderCount (PCSZ szName) throw (Exception &) ;
	/*! \brief	Returns the header of which name is szName */
	PCSZ GetHeader (PCSZ szName, DWORD nIdx = 0) throw (Exception &) ;	

	/*! \brief	Returns the HTTP status code */
	DWORD GetStatus (void) throw (Exception &) ;
	/*! \brief	Returns the HTTP status text */
	PCSZ GetStatusText (void) throw (Exception &) ;
	/*! \brief	Retrieves the content length */
	BOOL GetContentLength (DWORD & cbContLen) throw (Exception &) ;
	/*! \brief	Reads the content of a returned HTTP response */
	DWORD ReadContent (BYTE * pbyBuff, DWORD cbBuff) throw (Exception &) ;
	/*! \brief	Saves the content of a returned HTTP response to a file */
	void SaveContent (PCSZ szFilePath, BOOL bOverwrite = FALSE) throw (Exception &) ;
	
	/*! \brief	Returns the raw internet handle */
	inline HINTERNET GetInternetHandle (void) const throw () { return m_hInternet ; }
	/*! \brief	Returns the raw connection handle */
	inline HINTERNET GetConnectionHandle (void) const throw () { return m_hConnection ; }
	/*! \brief	Returns the raw request handle */
	inline HINTERNET GetRequestHandle (void) const throw () { return m_hRequest ; }

private:
	/*! \internal \brief	Initializes the internal member variables */
	void _Initialize (HINTERNET hInternet, HINTERNET hConnection, HINTERNET hRequest) throw () ;	
	/*! \internal \brief	Caches HTTP headers whose name equal to the szName */
	DWORD _LoadHeader (PCSZ szName) throw (Exception &) ;

	CHttpClientMapT<HttpTool>	m_mapHeader ;			//!< Cached headers

	HINTERNET			m_hInternet ;					//!< An internet handle
	HINTERNET			m_hConnection ;					//!< A connection handle
	HINTERNET			m_hRequest ;					//!< A request handle

	DWORD				m_dwStatus ;					//!< A cached status code
	PCSZ				m_szStatusText ;				//!< A cached status text
	size_t				m_cbContLen ;					//!< A cached content-length
} ;

/*! \brief	CHttpResponse class (Ansi version) */
typedef CHttpResponseT<CHttpToolA>		CHttpResponseA ;
/*! \brief	CHttpResponse class (Unicode version) */
typedef CHttpResponseT<CHttpToolW>		CHttpResponseW ;

#ifdef UNICODE
	/*! \brief	CHttpResponse class (Generic type version) */
	typedef CHttpResponseW		CHttpResponse ;
#else
	/*! \brief	CHttpResponse class (Generic type version) */
	typedef CHttpResponseA		CHttpResponse ;
#endif
///////////////////////////////////////// CHttpResponseT /////////////////////////////////////////

///////////////////////////////////////// CHttpPostStatT /////////////////////////////////////////
/*!
 * \brief	This class represents progress information of the HTTP POST operation.
 *
 * The purpose of this class is to provide progress information to user.
 * If you call BeginPost or BeginUpload method of the CHttpClient class, you can retrieve
 * progress information by using the Query method of the CHttpClient class.
 *
 * \sa		CHttpClientT, CHttpResponseT
 *
 * The following code sample demonstrates the usage of the CHttpPostStatT class.
 * \code

...

using namespace Ryeol ;

...

CHttpClient					objHttpReq ;
CHttpResponse *				pobjHttpRes = NULL ;
size_t						cbProceed = 1024 ;	// 1k

try {
	... ;	// Intialize the CHttpClient object

	// Starts a new POST request
	objHttpReq.BeginPost (...) or objHttpReq.BeginUpload (...) ;

	// Displays progress information
	CHttpPostStat			objPostStat ;

	do {
		// Retrieves progress information
		objHttpReq.Query (objPostStat) ;

		_tprintf (_T ("\nPost in progress... %2u/%2u\n")
			, objPostStat.PostedCount ()			// The number of posted parameters
			, objPostStat.TotalCount ()) ;			// The total number of parameters

		_tprintf (_T ("%s: %10u/%10u %10u/%10u %10u/%10u\n")
			, objPostStat.CurrParam ()				// The name of the current parameter
			, objPostStat.CurrParamPostedByte ()	// The number of posted bytes of the current parameter
			, objPostStat.CurrParamTotalByte ()		// The total number of bytes of the current parameter
			, objPostStat.PostedByte ()				// The number of posted bytes of the request
			, objPostStat.TotalByte ()				// The total number of bytes of the request
			, objPostStat.ActualPostedByte ()		// The actual number of posted bytes of the request
			, objPostStat.ActualTotalByte ()) ;		// The actual total number of bytes of the request

		// If the current parameter is a file parameter, displays the file path
		if ( objPostStat.CurrParamIsFile () )
			_tprintf (_T ("-->%s\n")
				, objPostStat.CurrFile ()) ;

		// Sends the number of bytes specified by cbProceed to the server
	} while ( !(pobjHttpRes = objHttpReq.Proceed (cbProceed)) ) ;

	... ;	// Handles the returned CHttpResponse object

} catch (httpclientexception & e) {
	_tprintf (_T ("An error has been occurred\n")) ;
	_tprintf (_T ("ErrCode: 0x%x Win32ErrCode: 0x%x\n"), e.LastError (), e.Win32LastError ()) ;
	_tprintf (_T ("ErrMsg: %s\n"), e.errmsg ()) ;
}

...

 * \endcode
 */
template <typename HttpTool>
class CHttpPostStatT {
public:
	// Basic type definitions ====================================================
	typedef typename HttpTool::Exception		Exception ;	//!< typedef of httpclientexception
	typedef typename HttpTool::CharType			CharType ;	//!< typedef of character type
	typedef typename HttpTool::PSZ				PSZ ;		//!< typedef of null-terminated string
	typedef typename HttpTool::PCSZ				PCSZ ;		//!< typedef of constant null-terminated string

	/*! \brief	Default constructor */
	CHttpPostStatT (void) throw () ;
	/*! \brief	Copy constructor */
	CHttpPostStatT (const CHttpPostStatT & objPostStat) throw () ;
	/*! \brief	Default destructor */
	virtual ~CHttpPostStatT (void) throw () ;

	/*! \brief	Indicates whether the POST is in progress or not */
	BOOL IsActive (void) const throw () ;

	/*! \brief	Returns the actual number of bytes to send to a HTTP web server */
	size_t ActualTotalByte (void) const throw (Exception &) ;
	/*! \brief	Returns the actual number of bytes posted to a HTTP web server */
	size_t ActualPostedByte (void) const throw (Exception &) ;

	/*! \brief	Returns the number of bytes to send to a HTTP web server */
	size_t TotalByte (void) const throw (Exception &) ;
	/*! \brief	Returns the number of bytes posted to a HTTP web server */
	size_t PostedByte (void) const throw (Exception &) ;

	/*! \brief	Returns the total number of parameters */
	DWORD TotalCount (void) const throw (Exception &) ;
	/*! \brief	Returns the number of posted parameters */
	DWORD PostedCount (void) const throw (Exception &) ;
	/*! \brief	Returns the number of file parameters */
	DWORD FileCount (void) const throw (Exception &) ;
	/*! \brief	Returns the number of posted file parameters */
	DWORD PostedFileCount (void) const throw (Exception &) ;

	/*! \brief	Returns the current parameter name */
	PCSZ CurrParam (void) const throw (Exception &) ;
	/*! \brief	Returns the current file path */
	PCSZ CurrFile (void) const throw (Exception &) ;
	/*! \brief	Returns the number of bytes of the current parameter */
	size_t CurrParamTotalByte (void) const throw (Exception &) ;
	/*! \brief	Returns the number of posted bytes of the current parameter */
	size_t CurrParamPostedByte (void) const throw (Exception &) ;
	/*! \brief	Returns the number of remained bytes of the current parameter */
	size_t CurrParamRemainByte (void) const throw (Exception &) ;
	/*! \brief	Returns whether the current parameter is a file parameter */
	BOOL CurrParamIsFile (void) const throw (Exception &) ;
	/*! \brief	Returns whether the current parameter is completely posted */
	BOOL CurrParamIsComplete (void) const throw (Exception &) ;

	/*! \brief	Assignment operator. */
	CHttpPostStatT & operator= (const CHttpPostStatT & objPostStat) throw () ;

private:
	friend CHttpClientT<HttpTool, CHttpEncoderA> ;
	friend CHttpClientT<HttpTool, CHttpEncoderW> ;

	void _InitMemberVariables (void) throw () ;
	void _SetString (PSZ * pszDest, PCSZ szSrc) throw () ;

	/*! \internal \brief	Returns TRUE if all bytes are sent */
	BOOL _IsComplete (void) const throw () ;

	/*! \internal \brief	Tests whether the m_cbActualPosted is safe to increase. */
	void _TestAddActualPostedBytes (size_t nBytes) throw (Exception &) ;
	/*! \internal \brief	Increases only m_cbActualPosted */
	void _AddActualPostedBytes (size_t nBytes) throw (Exception &) ;

	/*! \internal \brief	Cleans all internal states and closes the POST state */
	void _MakeUnActive (void) throw () ;
	/*! \internal \brief	Starts a new state of the HTTP POST */
	void _MakeActive (size_t cbActualTotal, size_t cbTotal, DWORD cParam, DWORD cFile) throw () ;

	/*! \internal \brief	Tests whether a new current parameter is safely set */
	void _TestStartNewEntry (PCSZ szCurrParam, size_t cbCurrParam, BOOL bIsFile = FALSE, PCSZ szCurrFile = NULL) throw (Exception &) ;
	/*! \internal \brief	Sets a new current parameter */
	void _StartNewEntry (PCSZ szCurrParam, size_t cbCurrParam, BOOL bIsFile = FALSE, PCSZ szCurrFile = NULL) throw (Exception &) ;

	/*! \internal \brief	Increases all counter related to the number of posted bytes */
	void _TestAddPostedBytes (size_t nBytes) throw (Exception &) ;
	/*! \internal \brief	Increases all counter related to the number of posted bytes */
	void _AddPostedBytes (size_t nBytes) throw (Exception &) ;

	BOOL		m_bIsActive ;			//!< Whether the POST is in progress

	size_t		m_cbActualTotal ;		//!< The actual total number of bytes
	size_t		m_cbActualPosted ;		//!< The actual number of posted bytes

	size_t		m_cbTotal ;				//!< The total number of bytes
	size_t		m_cbPosted ;			//!< The number of posted bytes

	DWORD		m_cParam ;				//!< The number of parameters
	DWORD		m_cParamPosted ;		//!< The number of posted parameters (includes the current parameter)
	DWORD		m_cFile ;				//!< The number of file parameters
	DWORD		m_cFilePosted ;			//!< The number of posted file parameters
										//!  (includes the current parameter if the current parameter is a file parameter)

	PSZ			m_szCurrParam ;			//!< The name of the current parameter
	PSZ			m_szCurrFile ;			//!< The file path of the current parameter

	size_t		m_cbCurrParam ;			//!< The total number of bytes of the current parameter
	size_t		m_cbCurrParamPosted ;	//!< The number of posted bytes of the current parameter
} ;

/*! \brief	CHttpPostStat class (Ansi version) */
typedef CHttpPostStatT<CHttpToolA>					CHttpPostStatA ;
/*! \brief	CHttpPostStat class (Unicode version) */
typedef CHttpPostStatT<CHttpToolW>					CHttpPostStatW ;

#ifdef UNICODE
	/*! \brief	CHttpPostStat class (Generic type version) */
	typedef CHttpPostStatW		CHttpPostStat ;
#else
	/*! \brief	CHttpPostStat class (Generic type version) */
	typedef CHttpPostStatA		CHttpPostStat ;
#endif
///////////////////////////////////////// CHttpPostStatT /////////////////////////////////////////


///////////////////////////////////////// CHttpUrlAnalyzer /////////////////////////////////////////
/*!
 * \brief	This class analyzes a URL into its component parts.
 *
 * This class analyzes a URL into its component parts and saves information about each parts.
 * It always try to analyze the URL to get the best result. It does not check whether the URL is valid. 
 * The URL itself is not saved.
 *
 * The following code sample demonstrates the usage of the CHttpUrlAnalyzer class.
 * \code

try {
	// A URL to analyze
	PCTSTR				tszUrl = _T ("http://wedding.makeself.net:80/board_story/list.php?frmtext=&frmtarget=title#ck") ;
	CHttpUrlAnalyzer	objAnalyzer (tszUrl) ;
	TCHAR				tszBuff[256] ;

	_tprintf (_T ("The analyzed URL : %s\n\n"), tszUrl) ;

	// Get protocol ("http" is printed)
	_tcsncpy (tszBuff, tszUrl + objAnalyzer.ProtocolIdx (), objAnalyzer.ProtocolLen ()) ;
	tszBuff[objAnalyzer.ProtocolLen ()] = '\0' ;
	_tprintf (_T ("Protocol         : %s\n"), tszBuff) ;

	// Get server address ("wedding.makeself.net" is printed)
	_tcsncpy (tszBuff, tszUrl + objAnalyzer.AddressIdx (), objAnalyzer.AddressLen ()) ;
	tszBuff[objAnalyzer.AddressLen ()] = '\0' ;
	_tprintf (_T ("Server Address   : %s\n"), tszBuff) ;

	// Get server port ("80" is printed)
	_tcsncpy (tszBuff, tszUrl + objAnalyzer.PortIdx (), objAnalyzer.PortLen ()) ;
	tszBuff[objAnalyzer.PortLen ()] = '\0' ;
	_tprintf (_T ("Server Port      : %s\n"), tszBuff) ;

	// Get path ("/board_story/list.php" is printed)
	_tcsncpy (tszBuff, tszUrl + objAnalyzer.PathIdx (), objAnalyzer.PathLen ()) ;
	tszBuff[objAnalyzer.PathLen ()] = '\0' ;
	_tprintf (_T ("Url Path         : %s\n"), tszBuff) ;

	// Get search string ("?frmtext=&frmtarget=title" is printed)
	_tcsncpy (tszBuff, tszUrl + objAnalyzer.SearchIdx (), objAnalyzer.SearchLen ()) ;
	tszBuff[objAnalyzer.SearchLen ()] = '\0' ;
	_tprintf (_T ("Search string    : %s\n"), tszBuff) ;

	// Get bookmark ("#ck" is printed)
	_tcsncpy (tszBuff, tszUrl + objAnalyzer.BookmarkIdx (), objAnalyzer.BookmarkLen ()) ;
	tszBuff[objAnalyzer.BookmarkLen ()] = '\0' ;
	_tprintf (_T ("Bookmark         : %s\n"), tszBuff) ;
} catch (httpclientexception & e) {
	// handle the exception
}

 * \endcode
 */
template <typename HttpTool>
class CHttpUrlAnalyzerT
{
public:
	typedef typename HttpTool::Exception	Exception ;		//!< typedef of httpclientexception
	typedef typename HttpTool::CharType		CharType ;		//!< typedef of character type
	typedef typename HttpTool::PSZ			PSZ ;			//!< typedef of null-terminated string
	typedef typename HttpTool::PCSZ			PCSZ ;			//!< typedef of constant null-terminated string

	/*! \brief	Default constructor */
	CHttpUrlAnalyzerT (PCSZ szUrl = NULL, UINT CodePage = CP_ACP) throw (Exception &) ;
	/*! \brief	Resets all internal states */
	void Reset (void) throw () ;
	/*! \brief	Analyzes a URL into its component parts */
	void Analyze (PCSZ szUrl, UINT CodePage = CP_ACP) throw (Exception &) ;

	/*! \brief	Returns the start index of the protocol part in the URL
	 *
	 * This method returns the start index of the protocol part in the URL.
	 *
	 * \return		The start index of the protocol part.
	 */
	inline DWORD ProtocolIdx (void) const throw () { return m_nProtocolIdx ; }
	/*! \brief	Returns the length of the protocol part in the URL.
	 *
	 * This method returns the length of the protocol part in the URL.
	 *
	 * \return		The length of the protocol part.
	 */
	inline DWORD ProtocolLen (void) const throw () { return m_cchProtocol ; }
	/*! \brief	Returns the start index of the address part in the URL
	 *
	 * This method returns the start index of the address part in the URL.
	 *
	 * \return		The start index of the address part.
	 */
	inline DWORD AddressIdx (void) const throw () { return m_nServerAddrIdx ; }
	/*! \brief	Returns the length of the address part in the URL
	 *
	 * This method returns the length of the address part in the URL.
	 *
	 * \return		The length of the address part.
	 */
	inline DWORD AddressLen (void) const throw () { return m_cchServerAddr ; }
	/*! \brief	Returns the start index of the port part in the URL
	 *
	 * This method returns the start index of the port part in the URL.
	 *
	 * \return		The start index of the port part.
	 */
	inline DWORD PortIdx (void) const throw () { return m_nServerPortIdx ; }
	/*! \brief	Returns the length of the port part in the URL
	 *
	 * This method returns the length of the port part in the URL.
	 *
	 * \return		The length of the port part.
	 */
	inline DWORD PortLen (void) const throw () { return m_cchServerPort ; }
	/*! \brief	Returns the start index of the path part in the URL
	 *
	 * This method returns the start index of the path part in the URL.
	 *
	 * \return		The start index of the path part.
	 */
	inline DWORD PathIdx (void) const throw () { return m_nUrlPathIdx ; }
	/*! \brief	Returns the length of the path part in the URL
	 *
	 * This method returns the length of the path part in the URL.
	 *
	 * \return		The length of the path part.
	 */
	inline DWORD PathLen (void) const throw () { return m_cchUrlPath ; }
	/*! \brief	Returns the start index of the search string part in the URL
	 *
	 * This method returns the start index of the search string part in the URL.
	 *
	 * \return		The start index of the search string part.
	 */
	inline DWORD SearchIdx (void) const throw () { return m_nSearchIdx ; }
	/*! \brief	Returns the length of the search string part in the URL
	 *
	 * This method returns the length of the search string part in the URL.
	 *
	 * \return		The length of the search string part.
	 */
	inline DWORD SearchLen (void) const throw () { return m_cchSearch ; }
	/*! \brief	Returns the start index of the bookmark part in the URL
	 *
	 * This method returns the start index of the bookmark part in the URL.
	 *
	 * \return		The start index of the bookmark part.
	 */
	inline DWORD BookmarkIdx (void) const throw () { return m_nBookmarkIdx ; }
	/*! \brief	Returns the length of the bookmark part in the URL
	 *
	 * This method returns the length of the bookmark part in the URL.
	 *
	 * \return		The length of the bookmark part.
	 */
	inline DWORD BookmarkLen (void) const throw () { return m_cchBookmark ; }

private:
	DWORD		m_nProtocolIdx, m_cchProtocol ;
	DWORD		m_nServerAddrIdx, m_cchServerAddr ;
	DWORD		m_nServerPortIdx, m_cchServerPort ;
	DWORD		m_nUrlPathIdx, m_cchUrlPath ;
	DWORD		m_nSearchIdx, m_cchSearch ;
	DWORD		m_nBookmarkIdx, m_cchBookmark ;
} ;

/*! \brief	CHttpUrlAnalyzer class (Ansi version) */
typedef CHttpUrlAnalyzerT<CHttpToolA>		CHttpUrlAnalyzerA ;
/*! \brief	CHttpUrlAnalyzer class (Unicode version) */
typedef CHttpUrlAnalyzerT<CHttpToolW>		CHttpUrlAnalyzerW ;

#ifdef UNICODE
	/*! \brief	CHttpUrlAnalyzer class (Generic type version) */
	typedef CHttpUrlAnalyzerW		CHttpUrlAnalyzer ;
#else
	/*! \brief	CHttpUrlAnalyzer class (Generic type version) */
	typedef CHttpUrlAnalyzerA		CHttpUrlAnalyzer ;
#endif
///////////////////////////////////////// CHttpUrlAnalyzer /////////////////////////////////////////


///////////////////////////////////////// CHttpClientT /////////////////////////////////////////
/*!
 * \brief	This class helps you to interact with a HTTP web server.
 *
 * This is a helper class which supports HTTP GET, POST and UPLOAD (multipart/form-data).
 *
 * \sa		CHttpResponseT, CHttpPostStatT
 */
template <typename HttpTool, typename HttpEncoder>
class CHttpClientT
{
public:
	// Basic type definition ====================================================
	typedef typename HttpTool::Exception			Exception ;			//!< typedef of httpclientexception
	typedef typename HttpTool::CharType				CharType ;			//!< typedef of character type
	typedef typename HttpTool::PSZ					PSZ ;				//!< typedef of null-terminated string
	typedef typename HttpTool::PCSZ					PCSZ ;				//!< typedef of constant null-terminated string
	typedef typename CHttpPostStatT<HttpTool>		CHttpPostStat ;		//!< typedef of the CHttpPostStatT class
	typedef typename CHttpResponseT<HttpTool>		CHttpResponse ;		//!< typedef of the CHttpResponseT class
	typedef typename CHttpUrlAnalyzerT<HttpTool>	CHttpUrlAnalyzer ;	//!< typedef of the CHttpUrlAnalyzerT class

	/*! \brief	Default constructor */
	CHttpClientT (void) throw () ;
	/*! \brief	Default destructor */
	virtual ~CHttpClientT (void) throw () ;

	// Basic options of the class =======================================
	/*! \brief	Sets the StrictFileCheck property */
	BOOL SetStrictFileCheck (BOOL bStrict) throw () ;
	/*! \brief	Returns the current StrictFileCheck property */
	BOOL GetStrictFileCheck (void) const throw () ;
	/*! \brief	Returns the current StrictFileCheck property */
	BOOL StrictFileCheck (void) const throw () ;
	/*! \brief	Sets the UseUtf8 property */
	BOOL SetUseUtf8 (BOOL bUseUtf8) throw (Exception &) ;
	/*! \brief	Returns the current UseUtf8 property */
	BOOL GetUseUtf8 (void) const throw () ;
	/*! \brief	Returns the current UseUtf8 property */
	BOOL UseUtf8 (void) const throw () ;
	/*! \brief	Sets the current ANSI code page */
	UINT SetAnsiCodePage (UINT nAnsiCodePage) throw (Exception &) ;
	/*! \brief	Returns the current ANSI code page */
	UINT GetAnsiCodePage (void) const throw () ;
	/*! \brief	Returns the current ANSI code page */
	UINT AnsiCodePage (void) const throw () ;
	
	// Methods related to the HTTP Headers ==============================
	/*! \brief	Erases all HTTP headers */
	BOOL ClearHeader (void) throw () ;
	/*! \brief	Erases a HTTP header at the given index */
	BOOL RemoveHeader (DWORD nIdx) throw () ;
	/*! \brief	Erases a HTTP header at the specified position */
	BOOL RemoveHeader (PCSZ szName, DWORD nIdx = 0) throw () ;
	/*! \brief	Erases all HTTP headers of which name is szName */
	BOOL RemoveAllHeader (PCSZ szName) throw () ;
	/*! \brief	Adds a new HTTP header */
	void AddHeader (PCSZ szName, PCSZ szValue) throw (Exception &) ;
	/*! \brief	Sets a HTTP header at the specified position */
	void SetHeader (PCSZ szName, PCSZ szValue, DWORD nIdx = 0) throw (Exception &) ;
	/*! \brief	Returns a HTTP header name at the given index */
	PCSZ GetHeaderName (DWORD nIdx) throw () ;
	/*! \brief	Returns a HTTP header at the given index */
	PCSZ GetHeader (DWORD nIdx) throw () ;
	/*! \brief	Returns a HTTP header at the specified position */
	PCSZ GetHeader (PCSZ szName, DWORD nIdx = 0) throw () ;
	/*! \brief	Returns whether the specified HTTP header exists */
	BOOL HeaderExists (PCSZ szName, DWORD nIdx = 0) throw () ;
	/*! \brief	Returns the number of HTTP headers of which name is szName */
	DWORD GetHeaderCount (PCSZ szName = NULL) throw () ;


	// Methods related to the HTTP Parameters ===========================
	/*!
	* \brief	An enum for attributes of the HTTP parameter
	* 
	* This enumerator specifies several attributes for the HTTP parameter.
	* You can specify a combination of the following flag constants.
	*/
	enum ParamAttr
	{
		ParamNormal			= 0x00000000		//!< The parameter is a normal parameter.
		, ParamFile			= 0x00000001		//!< The parameter is a file parameter.
												//!  This means that the parameter's value contains the file path
												//!  and the file specified by the file path is uploaded when the HTTP UPLOAD is started.
		, ParamEncodedName	= 0x00000002		//!< The parameter's name is a URL-Encoded string.
												//!  This means that the parameter's name is not encoded using URL-encoding
												//!  before sending to the web server when the HTTP GET or POST is started.
		, ParamEncodedValue	= 0x00000004		//!< The parameter's value is a URL-Encoded string.
												//!  This means that the parameter's value is not encoded using URL-encoding
												//!  before sending to the web server when the HTTP GET or POST is started.
		, ParamEncoded		= ParamEncodedName
							| ParamEncodedValue	//!< The parameter's name and value are URL-Encoded strings.
												//!  This means that the parameter's name and value are not encoded using URL-encoding
												//!  before sending to the web server when the HTTP GET or POST is started.
	} ;

	/*! \brief	Erases all HTTP parameters */
	BOOL ClearParam (void) throw (Exception &) ;
	/*! \brief	Erases a HTTP parameter at the given index */
	BOOL RemoveParam (DWORD nIdx) throw (Exception &) ;
	/*! \brief	Erases a HTTP parameter at the specified position */
	BOOL RemoveParam (PCSZ szName, DWORD nIdx = 0) throw (Exception &) ;
	/*! \brief	Erases all HTTP parameters of which name is szName */
	BOOL RemoveAllParam (PCSZ szName) throw (Exception &) ;
	/*! \brief	Adds a new HTTP parameter */
	void AddParam (PCSZ szName, PCSZ szValue = NULL, DWORD dwParamAttr = ParamNormal) throw (Exception &) ;
	/*! \brief	Sets a HTTP parameter at the specified position */
	void SetParam (PCSZ szName, PCSZ szValue = NULL, DWORD dwParamAttr = ParamNormal, DWORD nIdx = 0) throw (Exception &) ;
	/*! \brief	Returns a HTTP parameter name at the given index */
	PCSZ GetParamName (DWORD nIdx) throw () ;
	/*! \brief	Returns a HTTP parameter at the given index */
	PCSZ GetParam (DWORD nIdx) throw () ;
	/*! \brief	Returns a HTTP parameter at the specified position */
	PCSZ GetParam (PCSZ szName, DWORD nIdx = 0) throw () ;
	/*! \brief	Returns a HTTP parameter attribute at the given index */
	DWORD GetParamAttr (DWORD nIdx) throw () ;
	/*! \brief	Returns a HTTP parameter attribute at the specified position */
	DWORD GetParamAttr (PCSZ szName, DWORD nIdx = 0) throw () ;
	/*! \brief	Returns whether the specified HTTP parameter exists */
	BOOL ParamExists (PCSZ szName, DWORD nIdx = 0) throw () ;
	/*! \brief	Returns the number of HTTP parameters of which name is szName */
	DWORD GetParamCount (PCSZ szName = NULL) throw () ;

	// Methods related to the HTTP Operations =========================
	/*! \brief	Sets parameters for the ::InternetOpen function */
	void SetInternet (PCSZ szUserAgent = NULL, DWORD dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG, PCSZ szProxyName = NULL, PCSZ szProxyBypass = NULL, DWORD dwFlags = 0) throw (Exception &) ;
	/*! \brief	Sets the lpszAgent parameter for the ::InternetOpen function */
	void SetInternetUserAgent (PCSZ szUserAgent = NULL) throw (Exception &) ;
	/*! \brief	Returns the lpszAgent parameter for the ::InternetOpen function */
	PCSZ GetInternetUserAgent (void) throw () ;
	/*! \brief	Sets the dwAccessType parameter for the ::InternetOpen function */
	void SetInternetAccessType (DWORD dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG) throw () ;
	/*! \brief	Returns the dwAccessType parameter for the ::InternetOpen function */
	DWORD GetInternetAccessType (void) throw () ;
	/*! \brief	Sets the lpszProxyName parameter for the ::InternetOpen function */
	void SetInternetProxyName (PCSZ szProxyName = NULL) throw (Exception &) ;
	/*! \brief	Returns the lpszProxyName parameter for the ::InternetOpen function */
	PCSZ GetInternetProxyName (void) throw () ;
	/*! \brief	Sets the lpszProxyBypass parameter for the ::InternetOpen function */
	void SetInternetProxyBypass (PCSZ szProxyBypass = NULL) throw (Exception &) ;
	/*! \brief	Returns the lpszProxyBypass parameter for the ::InternetOpen function */
	PCSZ GetInternetProxyBypass (void) throw () ;
	/*! \brief	Sets the dwFlags parameter for the ::InternetOpen function */
	void SetInternetFlags (DWORD dwFlags = 0) throw () ;
	/*! \brief	Returns the dwFlags parameter for the ::InternetOpen function */
	DWORD GetInternetFlags (void) throw () ;


	/*! \brief	Returns the number of characters required to make a URL for a HTTP GET request */
	DWORD MakeGetUrlLen (PCSZ szUrl) throw (Exception &) ;
	/*! \brief	Returns a URL for a HTTP GET request */
	PSZ MakeGetUrl (PSZ szBuff, PCSZ szUrl) throw (Exception &) ;

	/*! \brief	Returns a new HINTERNET handle which is returned by ::InternetOpen */
	HINTERNET OpenInternet (void) throw (Exception &) ;

	/*! \brief	Returns a new HINTERNET handle which is returned by ::InternetConnect */
	HINTERNET OpenConnection (HINTERNET hInternet, PCSZ szUrl, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;

	/*! \brief	Sets a proxy user name and a password */
	void SetProxyAccount (PCSZ szUserName = NULL, PCSZ szPassword = NULL) throw (Exception &) ;
	/*! \brief	Returns a proxy user name which is used to access HTTP proxy server */
	PCSZ GetProxyUserName (void) const throw () ;
	/*! \brief	Returns a proxy password which is used to access HTTP proxy server */
	PCSZ GetProxyPassword (void) const throw () ;
	/*! \brief	Sets a proxy user name and password on a specified HINTERNET handle */
	void ApplyProxyAccount (HINTERNET hConnection) throw (Exception &) ;

	/*! \brief	Returns a new HINTERNET handle which is returned by ::HttpOpenRequest */
	HINTERNET OpenRequest (HINTERNET hConnection, PCSZ szVerb, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL) throw (Exception &) ;
	/*! \brief	Add the HTTP headers to the request handle */
	void AddRequestHeader (HINTERNET hRequest) throw (Exception &) ;


private:
	/*! \internal \brief	Retrieves the resource specified by the szUrl using HTTP GET request */
	CHttpResponse * _RequestGetEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags, PCSZ szReferer, PCSZ szUsrName, PCSZ szUsrPwd) throw (Exception &) ;

public:
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP GET request */
	CHttpResponse * RequestGet (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP GET request */
	CHttpResponse * RequestGet (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP GET request */
	CHttpResponse * RequestGet (PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;

	/*! \brief	Retrieves the resource specified by the szUrl using HTTP GET request */
	CHttpResponse * RequestGetEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP GET request */
	CHttpResponse * RequestGetEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP GET request */
	CHttpResponse * RequestGetEx (PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;

	// Progressive POST
	/*! \brief	Starts a new HTTP POST request */
	void BeginPost (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Starts a new HTTP POST request */
	void BeginPost (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Starts a new HTTP POST request */
	void BeginPost (PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;

	/*! \brief	Starts a new HTTP POST request */
	void BeginPostEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Starts a new HTTP POST request */
	void BeginPostEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Starts a new HTTP POST request */
	void BeginPostEx (PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;

	// Progressive UPLOAD
	/*! \brief	Starts a new HTTP UPLOAD request */
	void BeginUpload (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Starts a new HTTP UPLOAD request */
	void BeginUpload (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Starts a new HTTP UPLOAD request */
	void BeginUpload (PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;

	/*! \brief	Starts a new HTTP UPLOAD request */
	void BeginUploadEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Starts a new HTTP UPLOAD request */
	void BeginUploadEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Starts a new HTTP UPLOAD request */
	void BeginUploadEx (PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;

	/*! \brief	Queries progress information of the current POST context */
	void Query (CHttpPostStat & objPostStat) throw () ;
	/*! \brief	Cancels the current POST context */
	BOOL Cancel (void) throw () ;
	/*! \brief	Proceeds with the current POST context */
	CHttpResponse * Proceed (DWORD cbDesired) throw (Exception &) ;

	// Non progressive POST
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP POST request */
	CHttpResponse * RequestPost (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP POST request */
	CHttpResponse * RequestPost (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP POST request */
	CHttpResponse * RequestPost (PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;

	/*! \brief	Retrieves the resource specified by the szUrl using HTTP POST request */
	CHttpResponse * RequestPostEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP POST request */
	CHttpResponse * RequestPostEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP POST request */
	CHttpResponse * RequestPostEx (PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;

	// Non progressive UPLOAD
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP UPLOAD request */
	CHttpResponse * RequestUpload (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP UPLOAD request */
	CHttpResponse * RequestUpload (HINTERNET hInternet, PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP UPLOAD request */
	CHttpResponse * RequestUpload (PCSZ szUrl, BOOL bUseCache = FALSE) throw (Exception &) ;

	/*! \brief	Retrieves the resource specified by the szUrl using HTTP UPLOAD request */
	CHttpResponse * RequestUploadEx (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP UPLOAD request */
	CHttpResponse * RequestUploadEx (HINTERNET hInternet, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	/*! \brief	Retrieves the resource specified by the szUrl using HTTP UPLOAD request */
	CHttpResponse * RequestUploadEx (PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;

private:
	DWORD _String2AnsiLen (PCSZ szStr) throw (Exception &) ;
	PCSTR _String2Ansi (PCSZ szStr, BOOL & bNeedToFree) throw (Exception &) ;

	DWORD _UrlEncodeLenA (PCSZ szStr) throw (Exception &) ;
	PCSTR _UrlEncodeA (PSTR szBuff, PCSZ szStr) throw (Exception &) ;
	PCSTR _UrlEncodeA (PCSZ szStr) throw (Exception &) ;
	DWORD _UrlEncodeLen (PCSZ szStr) throw (Exception &) ;
	PCSZ _UrlEncode (PSZ szBuff, PCSZ szStr) throw (Exception &) ;
	PCSZ _UrlEncode (PCSZ szStr) throw (Exception &) ;

	DWORD _Utf8EncodeLen (PCSZ szStr) throw (Exception &) ;
	PCSTR _Utf8Encode (PSTR szBuff, PCSZ szStr) throw (Exception &) ;
	PCSTR _Utf8Encode (PCSZ szStr) throw (Exception &) ;

	/*! \brief	Make a copied string if the szTargetString and the szOldString are not the same */
	PSZ _AllocNCopyIfNewString (PCSZ szTargetString, PSZ szOldString) throw (Exception &) ;

private:
	// Options and member variables ========================================
	BOOL							m_bStrictFileCheck ;
	PSZ								m_szBoundary ;
	LPCSTR							m_szBoundaryA ;
	BOOL							m_bNeedToFreeBoundaryA ;
	PSZ								m_szUploadContType ;
	BOOL							m_bUseUtf8 ;
	UINT							m_nAnsiCodePage ;

	typedef	typename CHttpClientMapT<HttpTool>					Map ;
	typedef	typename CHttpClientMapT<HttpTool>::ConstMapIter	ConstMapIter ;
	Map			m_mapHeader ;		// map for HTTP headers
	Map			m_mapParam ;		// map for HTTP parameters

	// Member variables for the WinInet internet handle
	PSZ								m_szInternetUserAgent ;
	DWORD							m_dwInternetAccessType ;
	PSZ								m_szInternetProxyName ;
	PSZ								m_szInternetProxyBypass ;
	DWORD							m_dwInternetFlags ;
	PSZ								m_szProxyUserName ;			// User name for proxy server
	PSZ								m_szProxyPassword ;			// Password for proxy server

	// HTTP POST context ==================================================
	CHttpPostStat					m_objInitialStat ;
	CHttpPostStat					m_objPostStat ;
	HANDLE *						m_ahPostedFiles ;			// a pointer to an array of the files to post
	LPSTR *							m_aszMimeTypes ;			// a pointer to an array of the mime-type of the files to post
	LPCSTR							m_szPostedValue ;			// a string which is being posted
	BOOL							m_bNeedToFreePostedValue ;	// whether the m_szPostedValue is need to free
	HINTERNET						m_hInternet ;
	BOOL							m_bBorrowedInternet ;
	HINTERNET						m_hConnection ;
	BOOL							m_bBorrowedConnection ;
	public:
	HINTERNET						m_hRequest ;
		HINTERNET						m_hLastReq ;
	private:
	BOOL							m_bIsPost ;					// whether the request is POST or UPLOAD
	BYTE *							m_pbyCntxBuff ;				// The buffer for the Post Context

	void _InitPostContext (void) throw () ;

	BOOL _PostContextIsActive (void) const throw () ;
	BOOL _PostContextIsPost (void) const throw () ;
	void _SetPostedValue (LPCSTR szPostedValue, BOOL bNeedToFree) throw () ;
	void _InitPostCntxBuff (void) throw (Exception &) ;
	void _WritePost (BOOL bUseUtf8, PCSZ szValue, BOOL bIsValue = FALSE) throw (Exception &) ;
	void _WritePost (LPCSTR szValue, BOOL bIsValue = FALSE) throw (Exception &) ;
	void _WritePost (const BYTE * pbyBuff, DWORD cbyBuff, BOOL bIsValue = FALSE) throw (Exception &) ;

	void _StartPostContext (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	void _QueryPostStat (CHttpPostStat & objPostStat) throw () ;
	BOOL _CancelPostContext (void) throw () ;
	CHttpResponse * _ProceedPostContext (DWORD nDesired) throw (Exception &) ;

	PCSZ _GetUploadBoundary (void) throw () ;
	LPCSTR _GetUploadBoundaryA (void) throw () ;
	PCSZ _GetUploadContType (void) throw () ;

	void _StartUploadContext (HINTERNET hInternet, HINTERNET hConnection, PCSZ szUrl, DWORD dwFlags = HTTPCLIENT_DEF_REQUEST_FLAGS_NOCACHE, PCSZ szReferer = NULL, PCSZ szUsrName = NULL, PCSZ szUsrPwd = NULL) throw (Exception &) ;
	void _QueryUploadStat (CHttpPostStat & objPostStat) throw () ;
	BOOL _CancelUploadContext (void) throw () ;
	CHttpResponse * _ProceedUploadContext (DWORD nDesired) throw (Exception &) ;

	CHttpResponse * _ReleasePostResponse (void) throw (Exception &) ;
	void _ReleasePostContext (HINTERNET & hInternet, HINTERNET & hConnection, HINTERNET & hRequest) throw (Exception &) ;
	void _EndPostContext (void) throw (Exception &) ;
} ;

/*! \brief	CHttpClient class (Ansi version) */
typedef CHttpClientT<CHttpToolA, CHttpEncoderA>		CHttpClientA ;
/*! \brief	CHttpClient class (Unicode version) */
typedef CHttpClientT<CHttpToolW, CHttpEncoderW>		CHttpClientW ;

#ifdef UNICODE
	/*! \brief	CHttpClient class (Generic type version) */
	typedef CHttpClientW		CHttpClient ;
#else
	/*! \brief	CHttpClient class (Generic type version) */
	typedef CHttpClientA		CHttpClient ;
#endif
///////////////////////////////////////// CHttpClientT /////////////////////////////////////////
}

#pragma warning (pop)	// restores the default
