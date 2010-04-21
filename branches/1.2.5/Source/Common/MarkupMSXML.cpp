// MarkupMSXML.cpp: implementation of the CMarkupMSXML class.
//
// Markup Release 11.0
// Copyright (C) 2009 First Objective Software, Inc. All rights reserved
// Go to www.firstobject.com for the latest CMarkup and EDOM documentation
// Use in commercial applications requires written permission
// This software is provided "as is", with no warranty.


#include "stdafx.h"


#include "MarkupMSXML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CMarkupMSXML::CMarkupMSXML()
{
	CoInitialize(NULL);
	m_nVer = 0;
	x_Create();
}

CMarkupMSXML::CMarkupMSXML( int nVer )
{
	CoInitialize(NULL);
	m_nVer = nVer;
	x_Create();
}

CMarkupMSXML::CMarkupMSXML( LPCTSTR szDoc, int nVer/*=0*/ )
{
	CoInitialize(NULL);
	m_nVer = nVer;
	if ( SUCCEEDED(x_Create()) )
		SetDoc( szDoc );
}

_bstr_t CMarkupMSXML::ToBSTR( LPCTSTR pszText )
{
#ifdef _UNICODE
	_bstr_t bstrText( pszText );
#else
	USES_CONVERSION;
	_bstr_t bstrText( A2BSTR(pszText), false );
#endif
	return bstrText;
}

CString CMarkupMSXML::FromBSTR( const _bstr_t& bstrText )
{
#ifdef _UNICODE
	CString csText( (LPCWSTR)bstrText );
#else
	USES_CONVERSION;
	CString csText( W2A(bstrText) );
#endif
	return csText;
}

_variant_t CMarkupMSXML::ToVARIANT( LPCTSTR pszText )
{
	_variant_t varText;
#ifdef _UNICODE
	varText.vt = VT_BSTR;
	varText.bstrVal = SysAllocString(pszText);
#else
	varText.SetString(pszText);
#endif
	return varText;
}

CString CMarkupMSXML::FromVARIANT( const _variant_t& varText )
{
	CString csText;
	if ( varText.vt == VT_BSTR )
		csText = FromBSTR( varText.bstrVal );
	return csText;
}


CMarkupMSXML::~CMarkupMSXML()
{
	// Release COM interfaces, in case CoUninitialize is being called the last time
	if ( m_pChild )
		m_pChild.Release();
	if ( m_pMain )
		m_pMain.Release();
	if ( m_pParent )
		m_pParent.Release();
	if ( m_pDOMDoc )
		m_pDOMDoc.Release();
	CoUninitialize();
}

bool CMarkupMSXML::x_Create()
{
	// Release any reference to a previous instance
	if ( m_pParent )
		m_pParent.Release();
	if ( m_pDOMDoc )
		m_pDOMDoc.Release();

	// Create new instance
	m_pDOMDoc = x_CreateInstance();
	if ( m_pDOMDoc )
	{
		m_pParent = m_pDOMDoc;
		return true;
	}
	return false;
}

MSXMLDOCPTR CMarkupMSXML::x_CreateInstance()
{
	// Create new instance
	MSXMLDOCPTR pDOMDoc;
#if defined(MARKUP_MSXML1)
	HRESULT hr = pDOMDoc.CreateInstance( "Microsoft.XMLDOM" );
	m_nVer = 1;
#else
	// IE7 team focused on MSXML 6.0 and MSXML 3.0 as most reliable
	struct __declspec(uuid("88d96a05-f192-11d4-a65f-0040963251e5")) D6;
	struct __declspec(uuid("f5078f32-c551-11d3-89b9-0000f81fe221")) D3;
	struct __declspec(uuid("88d969c0-f192-11d4-a65f-0040963251e5")) D4;
	struct __declspec(uuid("88d969e5-f192-11d4-a65f-0040963251e5")) D5;
	struct { const CLSID* clsid; int nVer; } aVer[] =
	{
		&__uuidof(D6), 6,
		&__uuidof(D3), 3,
		&__uuidof(D4), 4,
		&__uuidof(D5), 5,
		NULL, 0
	};
	HRESULT hr = -1;
	int nTry = 0;
	while ( aVer[nTry].nVer )
	{
		if ( m_nVer == 0 || m_nVer == aVer[nTry].nVer )
		{
			hr = pDOMDoc.CreateInstance( *aVer[nTry].clsid );
			if ( SUCCEEDED(hr) )
			{
				m_nVer = aVer[nTry].nVer;
				// pDOMDoc->put_validateOnParse( true );
				// pDOMDoc->PutpreserveWhiteSpace( true );
				if ( m_nVer == 6 )
					pDOMDoc->setProperty( _T("ProhibitDTD"), VARIANT_FALSE );

				break;
			}
		}
		++nTry;
	}
#endif
	if ( FAILED(hr) )
	{
		if ( hr == REGDB_E_CLASSNOTREG )
			AfxMessageBox( _T("MSXML not registered") );
		else
			AfxMessageBox( _T("Unable to create MSXML instance") );
	}
	return pDOMDoc;
}

bool CMarkupMSXML::x_ParseError()
{
	MSXMLNS::IXMLDOMParseErrorPtr pDOMParseError;
	m_pDOMDoc->get_parseError( &pDOMParseError );
	m_strError = FromBSTR(pDOMParseError->Getreason());
	m_strError.TrimRight( _T("\r\n") );
	return false;
}

bool CMarkupMSXML::SetDoc( LPCTSTR szDoc )
{
	ResetPos();

	// If szDoc is empty, clear it
	if ( ! szDoc || ! szDoc[0] )
		return SUCCEEDED( x_Create() );

	VARIANT_BOOL bResult = m_pDOMDoc->loadXML( ToBSTR(szDoc) );
	if ( ! bResult )
		return x_ParseError();
	return true;
};

bool CMarkupMSXML::Load( LPCTSTR szFileName )
{
	VARIANT_BOOL bResult = m_pDOMDoc->load( ToVARIANT(szFileName) );
	ResetPos();
	if ( ! bResult )
		return x_ParseError();
	return true;
}

bool CMarkupMSXML::Save( LPCTSTR szFileName )
{
	HRESULT hr = m_pDOMDoc->save( ToVARIANT(szFileName) );
	if ( hr )
		return false;
	return true;
}

CString CMarkupMSXML::GetDoc() const
{
	return FromBSTR( m_pDOMDoc->xml );
};

bool CMarkupMSXML::FindElem( LPCTSTR szName )
{
	// Change current position only if found
	//
	MSXMLNS::IXMLDOMNodePtr pNode;
	pNode = x_FindElem( m_pParent, m_pMain, szName );
	if ( pNode )
	{
		m_pMain = pNode;
		m_pParent = m_pMain->GetparentNode();
		if ( m_pChild )
			m_pChild.Release();
		return true;
	}
	return false;
}

bool CMarkupMSXML::SetMainPosPtr( MSXMLNS::IXMLDOMNodePtr pMain )
{
	if ( m_pChild )
		m_pChild.Release();
	if ( pMain )
	{
		m_pMain = pMain;
		m_pParent = m_pMain->GetparentNode();
		return true;
	}
	m_pMain.Release();
	return false;
}

bool CMarkupMSXML::FindChildElem( LPCTSTR szName )
{
	// Change current child position only if found
	//
	// Shorthand: call this with no current main position
	// means find child under root element
	if ( ! ((bool)(m_pParent->GetparentNode())) && ! ((bool)m_pMain) )
		FindElem();
	if ( ! ((bool)m_pMain) )
		return false;

	MSXMLNS::IXMLDOMNodePtr pNode;
	pNode = x_FindElem( m_pMain, m_pChild, szName );
	if ( pNode )
	{
		m_pChild = pNode;
		m_pMain = m_pChild->GetparentNode();
		m_pParent = m_pMain->GetparentNode();
		return true;
	}

	return false;
}

bool CMarkupMSXML::IntoElem()
{
	if ( m_pMain )
	{
		m_pParent = m_pMain;
		if ( m_pChild )
		{
			m_pMain = m_pChild;
			m_pChild.Release();
		}
		else
			m_pMain.Release();
		return true;
	}
	return false;
}

bool CMarkupMSXML::OutOfElem()
{
	if ( (bool)(m_pParent->GetparentNode()) )
	{
		m_pChild = m_pMain;
		m_pMain = m_pParent;
		m_pParent = m_pMain->GetparentNode();
		return true;
	}
	return false;
}

CString CMarkupMSXML::GetAttribName( int n ) const
{
	CString strAttribName;
	if ( ! (bool)m_pMain )
		return strAttribName;

	// Is it within range?
	if ( n >= 0 && n < m_pMain->Getattributes()->Getlength() )
	{
		MSXMLNS::IXMLDOMNodePtr pAttrib = m_pMain->Getattributes()->item[n];
		strAttribName = FromBSTR( pAttrib->GetnodeName() );
	}
	return strAttribName;
}

bool CMarkupMSXML::RemoveElem()
{
	if ( m_pMain )
	{
		MSXMLNS::IXMLDOMNodePtr pParent = m_pMain->GetparentNode();
		if ( pParent )
		{
			if ( m_pChild )
				m_pChild.Release();
			MSXMLNS::IXMLDOMNodePtr pPrev = m_pMain->GetpreviousSibling();
			pParent->removeChild( m_pMain );
			m_pMain = pPrev;
			return true;
		}
	}
	return false;
}

bool CMarkupMSXML::RemoveChildElem()
{
	if ( m_pChild )
	{
		MSXMLNS::IXMLDOMNodePtr pPrev = m_pChild->GetpreviousSibling();
		m_pMain->removeChild( m_pChild );
		m_pChild.Release();
		m_pChild = pPrev;
		return true;
	}
	return false;
}


int CMarkupMSXML::FindNode( int nType )
{
	MSXMLNS::IXMLDOMNodePtr pNext;
	if ( m_pMain )
		pNext = m_pMain->GetnextSibling();
	else
		pNext = m_pParent->GetfirstChild();
	while ( pNext )
	{
		int nTypeFound = x_GetNodeType( pNext );
		if ( (nType==0 && nTypeFound!=0) || (nTypeFound & nType) )
		{
			m_pMain = pNext;
			if ( m_pChild )
				m_pChild.Release();
			return nTypeFound;
		}
		pNext = pNext->GetnextSibling();
	}
	return 0;
}

bool CMarkupMSXML::RemoveNode()
{
	if ( m_pMain )
	{
		MSXMLNS::IXMLDOMNodePtr pParent = m_pMain->GetparentNode();
		if ( pParent )
		{
			if ( m_pChild )
				m_pChild.Release();
			MSXMLNS::IXMLDOMNodePtr pPrev = m_pMain->GetpreviousSibling();
			pParent->removeChild( m_pMain );
			m_pMain = pPrev;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////


MSXMLNS::IXMLDOMNodePtr CMarkupMSXML::x_FindElem( MSXMLNS::IXMLDOMNodePtr pParent, MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szPath )
{
	// If szPath is NULL or empty, go to next sibling element
	// Otherwise go to next sibling element with matching path
	//
	if ( pNode )
		pNode = pNode->GetnextSibling();
	else
		pNode = pParent->GetfirstChild();

	while ( pNode )
	{
		if ( pNode->nodeType == MSXMLNS::NODE_ELEMENT )
		{
			// Compare tag name unless szPath is not specified
			if ( szPath == NULL || !szPath[0] || x_GetTagName(pNode) == szPath )
				break;
		}
		pNode = pNode->GetnextSibling();
	}
	return pNode;

}

CString CMarkupMSXML::x_GetPath( MSXMLNS::IXMLDOMNodePtr pNode ) const
{
	CString strPath;
	while ( ((bool)pNode) && pNode->GetnodeType() != MSXMLNS::NODE_DOCUMENT )
	{
		CString strTagName = x_GetTagName( pNode );
		MSXMLNS::IXMLDOMNodePtr pPrevNode = pNode->GetpreviousSibling();
		int nCount = 1;
		while ( pPrevNode )
		{
			if ( x_GetTagName( pPrevNode ) == strTagName )
				++nCount;
			pPrevNode = pPrevNode->GetpreviousSibling();
		}
		if ( nCount > 1 )
		{
			_TCHAR szPred[25];
			_stprintf( szPred, _T("[%d]"), nCount );
			strPath = _T("/") + strTagName + szPred + strPath;
		}
		else
			strPath = _T("/") + strTagName + strPath;

		pNode = pNode->GetparentNode();
	}
	return strPath;
}

CString CMarkupMSXML::x_GetTagName( MSXMLNS::IXMLDOMNodePtr pNode ) const
{
	CString strTagName;
	if ( pNode )
		strTagName = FromBSTR( pNode->GetnodeName() );
	return strTagName;
}

CString CMarkupMSXML::x_GetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib ) const
{
	CString strAttrib;
	if ( pNode )
	{
		MSXMLNS::IXMLDOMNodePtr pAttrib;
		HRESULT hr = pNode->Getattributes()->raw_getNamedItem( ToBSTR(szAttrib), &pAttrib );
		if ( SUCCEEDED(hr) && ((bool)pAttrib) )
			strAttrib = FromVARIANT(pAttrib->GetnodeValue());
	}
	return strAttrib;
}

void CMarkupMSXML::x_Insert( MSXMLNS::IXMLDOMNodePtr pParent, MSXMLNS::IXMLDOMNodePtr pNext, MSXMLNS::IXMLDOMNodePtr pNew )
{
	if ( pNext )
	{
		VARIANT varRef;
		VariantInit( &varRef );
		varRef.vt = VT_DISPATCH;
		varRef.pdispVal = pNext.GetInterfacePtr();
		pParent->insertBefore( pNew, varRef );
	}
	else
		pParent->appendChild( pNew );
}

bool CMarkupMSXML::x_AddElem( LPCTSTR szName, int nValue, bool bInsert, bool bAddChild )
{
	// Convert integer to string
	_TCHAR szVal[25];
	_stprintf( szVal, _T("%d"), nValue );
	return x_AddElem( szName, szVal, bInsert, bAddChild );
}

bool CMarkupMSXML::x_AddElem( LPCTSTR szName, LPCTSTR szData, bool bInsert, bool bAddChild )
{
	MSXMLNS::IXMLDOMNodePtr pNext, pParent;
	if ( bAddChild )
	{
		if ( ! (bool)m_pMain )
			return false;
		pParent = m_pMain;
		pNext = m_pChild;
	}
	else
	{
		if ( m_pChild )
			m_pChild.Release();
		pParent = m_pParent;
		pNext = m_pMain;
	}
	if ( bInsert )
	{
		if ( ! ((bool)pNext) )
			pNext = pParent->GetfirstChild();
	}
	else
	{
		if ( pNext )
			pNext = pNext->GetnextSibling();
	}

	MSXMLNS::IXMLDOMElementPtr pNew;
	if ( m_strDefaultNamespace.IsEmpty() )
		pNew = m_pDOMDoc->createElement( ToBSTR(szName) );
	else
		pNew = m_pDOMDoc->createNode( _variant_t((short)MSXMLNS::NODE_ELEMENT),
			ToBSTR(szName), ToBSTR(m_strDefaultNamespace) );

	x_Insert( pParent, pNext, pNew );
	if ( szData && szData[0] )
	{
		MSXMLNS::IXMLDOMNodePtr pText = m_pDOMDoc->createTextNode( ToBSTR(szData) );
		pNew->appendChild( pText );
	}

	if ( bAddChild )
		m_pChild = pNew;
	else
		m_pMain = pNew;
	return true;
}

CString CMarkupMSXML::x_GetSubDoc( MSXMLNS::IXMLDOMNodePtr pNode ) const
{
	if ( (bool)pNode )
		return FromBSTR(pNode->xml);
	return _T("");
}

bool CMarkupMSXML::x_AddSubDoc( LPCTSTR szSubDoc, bool bInsert, bool bAddChild )
{
	MSXMLNS::IXMLDOMNodePtr pNext, pParent;
	if ( bAddChild )
	{
		// Add a subdocument under main position, before or after child
		if ( ! (bool)m_pMain )
			return false;
		pParent = m_pMain;
		pNext = m_pChild;
	}
	else
	{
		// Add a subdocument under parent position, before or after main
		if ( ! (bool)m_pParent )
			return false;
		pParent = m_pParent;
		pNext = m_pMain;
	}
	if ( bInsert )
	{
		if ( ! ((bool)pNext) )
			pNext = pParent->GetfirstChild();
	}
	else
	{
		if ( pNext )
			pNext = pNext->GetnextSibling();
	}

	MSXMLDOCPTR pSubDoc = x_CreateInstance();
#if ! defined(_UNICODE) && ! defined(MARKUP_MSXML1)
	VARIANT_BOOL bResult = pSubDoc->loadXML( szSubDoc );
#else
	VARIANT_BOOL bResult = pSubDoc->loadXML( ToBSTR(szSubDoc) );
#endif
	if ( ! bResult )
		return false;
	MSXMLNS::IXMLDOMElementPtr pNew = pSubDoc->GetdocumentElement();
	x_Insert( pParent, pNext, pNew );

	if ( bAddChild )
	{
		m_pChild = pNew;
	}
	else
	{
		m_pMain = pNew;
		if ( m_pChild )
			m_pChild.Release();
	}
	return true;
}

bool CMarkupMSXML::x_SetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib, int nValue )
{
	_TCHAR szVal[25];
	_itot( nValue, szVal, 10 );
	return x_SetAttrib( pNode, szAttrib, szVal );
}

bool CMarkupMSXML::x_SetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib, LPCTSTR szValue )
{
	if ( ! ((bool)pNode) )
		return false;

	MSXMLNS::IXMLDOMNamedNodeMapPtr pAttribs = pNode->Getattributes();
	MSXMLNS::IXMLDOMAttributePtr pAttr = m_pDOMDoc->createAttribute( ToBSTR(szAttrib) );
	if ( pAttr )
	{
		pAttr->put_value( ToVARIANT(szValue) );
		pAttribs->setNamedItem( pAttr );
		return true;
	}
	return false;
}


int CMarkupMSXML::x_GetNodeType( MSXMLNS::IXMLDOMNodePtr pNode ) const
{
	// Convert DOM node type to EDOM node type
	if ( ((bool)pNode) )
	{
		switch ( pNode->GetnodeType() )
		{
		case MSXMLNS::NODE_ELEMENT:
			return MNT_ELEMENT;
		case MSXMLNS::NODE_TEXT:
			{
				// Determine if whitespace
				MSXMLNS::IXMLDOMTextPtr pText = pNode;
				CString strTextData = FromBSTR(pText->Getdata());
				if ( strTextData.SpanIncluding( _T(" \t\r\n") ).GetLength() == strTextData.GetLength() )
					return MNT_WHITESPACE;
				return MNT_TEXT;
			}
		case MSXMLNS::NODE_CDATA_SECTION:
			return MNT_CDATA_SECTION;
		case MSXMLNS::NODE_PROCESSING_INSTRUCTION:
			return MNT_PROCESSING_INSTRUCTION;
		case MSXMLNS::NODE_COMMENT:
			return MNT_COMMENT;
		case MSXMLNS::NODE_DOCUMENT_TYPE:
			MNT_DOCUMENT_TYPE;
		}
	}
	return 0;
}

bool CMarkupMSXML::x_AddNode( int nNodeType, LPCTSTR szText, bool bInsert )
{
	// Create
	MSXMLNS::IXMLDOMNodePtr pNew;
	switch ( nNodeType )
	{
	case MNT_ELEMENT:
		{
			MSXMLNS::IXMLDOMElementPtr pElement;
			pElement = m_pDOMDoc->createElement( ToBSTR(szText) );
			pNew = pElement;
		}
		break;
	case MNT_TEXT:
	case MNT_WHITESPACE:
		pNew = m_pDOMDoc->createTextNode( ToBSTR(szText) );
		break;
	case MNT_CDATA_SECTION:
		{
			MSXMLNS::IXMLDOMCDATASectionPtr pCDATASection;
			pCDATASection = m_pDOMDoc->createCDATASection( ToBSTR(szText) );
			pNew = pCDATASection;
		}
		break;
	case MNT_PROCESSING_INSTRUCTION:
		{
			// Extract target
			CString strTarget( szText ), strData;
			int nSpace = strTarget.FindOneOf( _T(" \t\r\n") );
			if ( nSpace != -1 )
			{
				strData = strTarget.Mid( nSpace + 1 );
				strTarget = strTarget.Left( nSpace );
			}
			MSXMLNS::IXMLDOMProcessingInstructionPtr pPI;
			pPI = m_pDOMDoc->createProcessingInstruction( ToBSTR(strTarget), ToBSTR(strData) );
			pNew = pPI;
		}
		break;
	case MNT_COMMENT:
		pNew = m_pDOMDoc->createComment( ToBSTR(szText) );
		break;
	case MNT_DOCUMENT_TYPE:
		m_strError = "XMLDOM: Document Type is Read-Only";
		return false;
	}

	// Insert
	MSXMLNS::IXMLDOMNodePtr pNext, pParent;
	if ( m_pChild )
		m_pChild.Release();
	pParent = m_pParent;
	pNext = m_pMain;
	if ( bInsert )
	{
		if ( ! ((bool)pNext) )
			pNext = pParent->GetfirstChild();
	}
	else
	{
		if ( pNext )
			pNext = pNext->GetnextSibling();
	}
	x_Insert( pParent, pNext, pNew );

	m_pMain = pNew;
	return true;
}

CString CMarkupMSXML::x_GetData( MSXMLNS::IXMLDOMNodePtr pNode ) const
{
	CString strData;
	if ( ! ((bool)pNode) )
		return strData;

	int nNodeType = x_GetNodeType( pNode );
	if ( nNodeType != MNT_ELEMENT )
	{
		switch ( nNodeType )
		{
		case MNT_PROCESSING_INSTRUCTION:
			{
				MSXMLNS::IXMLDOMProcessingInstructionPtr pPI = pNode;
				CString strTarget = FromBSTR(pPI->Gettarget());
				strData.Format( _T("%s %s"), strTarget, FromBSTR(pPI->Getdata()) );
			}
			break;
		case MNT_COMMENT:
			{
				MSXMLNS::IXMLDOMCommentPtr pComment = pNode;
				strData = FromBSTR(pComment->Getdata());
			}
			break;
		case MNT_CDATA_SECTION:
			{
				MSXMLNS::IXMLDOMCDATASectionPtr pCDATASection = pNode;
				strData = FromBSTR(pCDATASection->Getdata());
			}
			break;
		case MNT_TEXT:
		case MNT_WHITESPACE:
			{
				MSXMLNS::IXMLDOMTextPtr pText = pNode;
				strData = FromBSTR(pText->Getdata());
			}
			break;
		}
		return strData;
	}

	if ( (bool)(pNode->GetfirstChild()) )
		strData = FromBSTR(pNode->Gettext());
	return strData;
}

bool CMarkupMSXML::x_SetData( MSXMLNS::IXMLDOMNodePtr& pNode, LPCTSTR szData, int nCDATA )
{
	if ( ! ((bool)pNode) )
		return false;

	int nNodeType = x_GetNodeType( pNode );
	if ( nNodeType != MNT_ELEMENT )
	{
		switch ( nNodeType )
		{
		case MNT_PROCESSING_INSTRUCTION:
			{
				MSXMLNS::IXMLDOMProcessingInstructionPtr pPI = pNode;
				CString strExistingTarget = FromBSTR(pPI->Gettarget());

				// Extract target from specified target
				CString strTarget( szData ), strData;
				int nSpace = strTarget.FindOneOf( _T(" \t\r\n") );
				if ( nSpace != -1 )
				{
					strData = strTarget.Mid( nSpace + 1 );
					strTarget = strTarget.Left( nSpace );
				}
				if ( strTarget != strExistingTarget )
				{
					m_strError = "Specified Target does not match current node";
					return false;
				}
				pPI->Putdata( ToBSTR(strData) );
			}
			break;
		case MNT_COMMENT:
			{
				MSXMLNS::IXMLDOMCommentPtr pComment = pNode;
				pComment->Putdata( ToBSTR(szData) );
			}
			break;
		case MNT_CDATA_SECTION:
			{
				MSXMLNS::IXMLDOMCDATASectionPtr pCDATASection = pNode;
				pCDATASection->Putdata( ToBSTR(szData) );
			}
			break;
		case MNT_TEXT:
		case MNT_WHITESPACE:
			{
				MSXMLNS::IXMLDOMTextPtr pText = pNode;
				pText->Putdata( ToBSTR(szData) );
			}
			break;
		}
		return true;
	}

	if ( ! ((bool)pNode) )
		return false;

	// Return false if child element
	MSXMLNS::IXMLDOMNodePtr pChild = pNode->GetfirstChild();
	MSXMLNS::IXMLDOMNodePtr pChildNext = pChild;
	while ( (bool)pChildNext )
	{
		if ( pChildNext->nodeType == MSXMLNS::NODE_ELEMENT )
			return false;
		pChildNext = pChildNext->GetnextSibling();
	}

	// Remove any child nodes
	pChildNext = pChild;
	while ( (bool)pChildNext )
	{
		MSXMLNS::IXMLDOMNodePtr pChildDel = pChildNext;
		pChildNext = pChildNext->GetnextSibling();
		pNode->removeChild( pChildDel );
	}

	if ( nCDATA != 0 )
	{
		MSXMLNS::IXMLDOMCDATASectionPtr pData;
		LPCTSTR pszNextStart = szData;
		LPCTSTR pszEnd = _tcsstr( szData, _T("]]>") );
		while ( pszEnd )
		{
			CString csSegment( pszNextStart, (int)(pszEnd - pszNextStart) + 2 );
			pData = m_pDOMDoc->createCDATASection( ToBSTR(csSegment) );
			pNode->appendChild( pData );
			pszNextStart = pszEnd + 2;
			pszEnd = _tcsstr( pszNextStart, _T("]]>") );
		}
		pData = m_pDOMDoc->createCDATASection( ToBSTR(pszNextStart) );
		pNode->appendChild( pData );
	}
	else
	{
		MSXMLNS::IXMLDOMNodePtr pText = m_pDOMDoc->createTextNode( ToBSTR(szData) );
		pNode->appendChild( pText );
	}

	return true;
}
