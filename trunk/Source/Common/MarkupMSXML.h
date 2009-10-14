// MarkupMSXML.h: interface for the CMarkupMSXML class.
//
// Markup Release 11.0
// Copyright (C) 2009 First Objective Software, Inc. All rights reserved
// Go to www.firstobject.com for the latest CMarkup and EDOM documentation
// Use in commercial applications requires written permission
// This software is provided "as is", with no warranty.

#if !defined(_MARKUPMSXML_H_INCLUDED_)
#define _MARKUPMSXML_H_INCLUDED_
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#if _MSC_VER > 1000
#pragma once
#pragma warning(disable:4996) // suppress VS 2005 deprecated function warnings
#pragma warning(disable:4786)
#endif // _MSC_VER > 1000
#define  MARKUP_MSXML4
// MARKUP_MSXML1 compile for MSXML 1.0 interface, works with MSXML 1.0 only
// MARKUP_MSXML3 default, compile for MSXML 3.0 interface, works with MSXML 6.0 to 3.0
// MARKUP_MSXML4 compile for MSXML 4.0 interface, works with MSXML 6.0 to 3.0
//
#define AfxMessageBox(s) MessageBox(GetActiveWindow(),s,0,0) 
#if defined(MARKUP_MSXML1)
#import <msxml.dll>
#define MSXMLDOCPTR MSXML::IXMLDOMDocumentPtr
#define MSXMLNS MSXML
#else
#if defined(MARKUP_MSXML4)
#import <msxml4.dll> no_function_mapping
#else
#import <msxml3.dll> no_function_mapping
#endif
#define MSXMLDOCPTR MSXML2::IXMLDOMDocument2Ptr
#define MSXMLNS MSXML2
#endif

class CMarkupMSXML  
{
public:
	MSXMLDOCPTR m_pDOMDoc;

	CMarkupMSXML();
	CMarkupMSXML( LPCTSTR szDoc, int nVer = 0 );
	CMarkupMSXML( int nVer );
	virtual ~CMarkupMSXML();

	static _bstr_t ToBSTR( LPCTSTR pszText );
	static CString FromBSTR( const _bstr_t& bstrText );
	static _variant_t ToVARIANT( LPCTSTR pszText );
	static CString FromVARIANT( const _variant_t& varText );

	// Navigate
	bool Load( LPCTSTR szFileName );
	bool SetDoc( LPCTSTR szDoc );
	bool FindElem( LPCTSTR szName=NULL );
	bool FindChildElem( LPCTSTR szName=NULL );
	bool IntoElem();
	bool OutOfElem();
	void ResetChildPos() { if ( m_pChild ) m_pChild.Release(); };
	void ResetMainPos() { ResetChildPos(); if ( m_pMain ) m_pMain.Release(); };
	void ResetPos() { ResetMainPos(); m_pParent = m_pDOMDoc; };
	bool SetMainPosPtr( MSXMLNS::IXMLDOMNodePtr pMain );
	CString GetTagName() const { return x_GetTagName(m_pMain); };
	CString GetChildTagName() const { return x_GetTagName(m_pChild); };
	CString GetData() const { return x_GetData( m_pMain ); };
	CString GetChildData() const { return x_GetData(m_pChild); };
	CString GetAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib( m_pMain, szAttrib ); };
	CString GetChildAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib( m_pChild, szAttrib ); };
	CString GetAttribName( int n ) const;
	int FindNode( int nType=0 );
	int GetNodeType() { return x_GetNodeType( m_pMain ); };
	CString GetError() const { return m_strError; };
	int GetVersionCreated() const { return m_nVer; };

	enum MarkupNodeType
	{
		MNT_ELEMENT					= 1,  // 0x01
		MNT_TEXT					= 2,  // 0x02
		MNT_WHITESPACE				= 4,  // 0x04
		MNT_CDATA_SECTION			= 8,  // 0x08
		MNT_PROCESSING_INSTRUCTION	= 16, // 0x10
		MNT_COMMENT					= 32, // 0x20
		MNT_DOCUMENT_TYPE			= 64, // 0x40
		MNT_EXCLUDE_WHITESPACE		= 123,// 0x7b
	};

	// Create
	bool Save( LPCTSTR szFileName );
	CString GetDoc() const;
	bool AddElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,false,false); };
	bool InsertElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,true,false); };
	bool AddChildElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,false,true); };
	bool InsertChildElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,true,true); };
	bool AddElem( LPCTSTR szName, int nValue ) { return x_AddElem(szName,nValue,false,false); };
	bool InsertElem( LPCTSTR szName, int nValue ) { return x_AddElem(szName,nValue,true,false); };
	bool AddChildElem( LPCTSTR szName, int nValue ) { return x_AddElem(szName,nValue,false,true); };
	bool InsertChildElem( LPCTSTR szName, int nValue ) { return x_AddElem(szName,nValue,true,true); };
	bool AddAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_pMain,szAttrib,szValue); };
	bool AddChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_pChild,szAttrib,szValue); };
	bool AddAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_pMain,szAttrib,nValue); };
	bool AddChildAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_pChild,szAttrib,nValue); };
	bool AddSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,false,false); };
	bool InsertSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,true,false); };
	CString GetSubDoc() const { return x_GetSubDoc(m_pMain); };
	bool AddChildSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,false,true); };
	bool InsertChildSubDoc( LPCTSTR szSubDoc ) { return x_AddSubDoc(szSubDoc,true,true); };
	CString GetChildSubDoc() const { return x_GetSubDoc(m_pChild); };
	void SetDefaultNamespace( LPCTSTR szNamespace ) { m_strDefaultNamespace = szNamespace?szNamespace:_T(""); };
	bool AddNode( int nType, LPCTSTR szText ) { return x_AddNode(nType,szText,false); };
	bool InsertNode( int nType, LPCTSTR szText ) { return x_AddNode(nType,szText,true); };

	// Modify
	bool RemoveElem();
	bool RemoveChildElem();
	bool RemoveNode();
	bool SetAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_pMain,szAttrib,szValue); };
	bool SetChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_pChild,szAttrib,szValue); };
	bool SetAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_pMain,szAttrib,nValue); };
	bool SetChildAttrib( LPCTSTR szAttrib, int nValue ) { return x_SetAttrib(m_pChild,szAttrib,nValue); };
	bool SetData( LPCTSTR szData, int nCDATA=0 ) { return x_SetData(m_pMain,szData,nCDATA); };
	bool SetChildData( LPCTSTR szData, int nCDATA=0 ) { return x_SetData(m_pChild,szData,nCDATA); };

protected:
	MSXMLNS::IXMLDOMNodePtr m_pParent;
	MSXMLNS::IXMLDOMNodePtr m_pMain;
	MSXMLNS::IXMLDOMNodePtr m_pChild;
	CString m_strError;
	CString m_strDefaultNamespace;
	int m_nVer;

	bool x_Create();
	MSXMLDOCPTR x_CreateInstance();
	bool x_ParseError();
	MSXMLNS::IXMLDOMNodePtr x_FindElem( MSXMLNS::IXMLDOMNodePtr pParent, MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szPath );
	CString x_GetPath( MSXMLNS::IXMLDOMNodePtr pNode ) const;
	CString x_GetTagName( MSXMLNS::IXMLDOMNodePtr pNode ) const;
	CString x_GetData( MSXMLNS::IXMLDOMNodePtr pNode ) const;
	CString x_GetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib ) const;
	void CMarkupMSXML::x_Insert( MSXMLNS::IXMLDOMNodePtr pParent, MSXMLNS::IXMLDOMNodePtr pNext, MSXMLNS::IXMLDOMNodePtr pNew );
	bool x_AddElem( LPCTSTR szName, LPCTSTR szData, bool bInsert, bool bAddChild );
	bool x_AddElem( LPCTSTR szName, int nValue, bool bInsert, bool bAddChild );
	CString x_GetSubDoc( MSXMLNS::IXMLDOMNodePtr pNode ) const;
	bool x_AddSubDoc( LPCTSTR szSubDoc, bool bInsert, bool bAddChild );
	bool x_SetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib, LPCTSTR szValue );
	bool x_SetAttrib( MSXMLNS::IXMLDOMNodePtr pNode, LPCTSTR szAttrib, int nValue );
	bool x_CreateNode( CString& strNode, int nNodeType, LPCTSTR szText );
	bool x_AddNode( int nNodeType, LPCTSTR szText, bool bInsert );
	int x_GetNodeType( MSXMLNS::IXMLDOMNodePtr pNode ) const;
	bool x_SetData( MSXMLNS::IXMLDOMNodePtr& pNode, LPCTSTR szData, int nCDATA );
};

#endif // !defined(_MARKUPMSXML_H_INCLUDED_)
