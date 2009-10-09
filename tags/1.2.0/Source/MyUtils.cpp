//	Copyright 2007-2009 Zenden. All Rights Reserved.
//
// This file is a part of Image Uploader application
// HomePage:    http://zenden.ws/imageuploader
//

#include "stdafx.h"
#include "myutils.h"

#define STRING MAX_PATH

int GetFontSize(int nFontHeight)
{
	return - MulDiv( nFontHeight, 72, GetDeviceCaps(::GetDC(0), LOGPIXELSY));
}

int GetFontHeight(int nFontSize)
{
	return - MulDiv(nFontSize, GetDeviceCaps(::GetDC(0), LOGPIXELSY), 72);
}

int GetFontSizeInTwips(int nFontSize)
{
   return MulDiv(nFontSize, 1440, 72);
}


bool ExtractStrFromList(
            LPCTSTR szString /* Source string */,
            int nIndex, /* Zero based item index */
            LPTSTR szBuffer /* Destination buffer */,
            LONG nSize, /* Length in characters of destionation buffer */
            LPCTSTR szDefString,
            TCHAR cSeparator /* Character to be separator in list */)
{
	int nStringLen = 0;
	LPCTSTR szSeparator,szPrevSep;

	szSeparator=szPrevSep=0;

	int i;
	int nStart,nLen,nNumOfItems;
	nStart=nLen=-1;
	nNumOfItems=1;

	if(!szString || !szBuffer) return false;

	nStringLen = lstrlen(szString);

	if(nStringLen<=0) goto lbl_copydef;

	while(*szString==_T(' ')) szBuffer++;

	if(nStringLen<=0) goto lbl_copydef;

	szPrevSep=szString;

	nStart=0;
	nLen=0;

	for(i=0;i<nStringLen+1;i++)
	{
		if(szString[i]==0)
		{
			if(nIndex<nNumOfItems)
				nLen=i-nStart;
			break;
		}

		else if(szString[i]==cSeparator)
		{
			nNumOfItems++;
			if( nNumOfItems-1 == nIndex)
			{
				nStart=i+1;

			}
			else if( nNumOfItems-2 == nIndex)
			{
				nLen=i-nStart;
				break;

			}
		}
	}

	if(nLen>nSize-1) nLen = nSize-1;

	if(nLen<=0) goto lbl_copydef;

	lstrcpyn(szBuffer, szString + nStart, nLen+1);

	goto lbl_allok;

lbl_copydef:
	if(szDefString) lstrcpy(szBuffer, szDefString);
	return false;

lbl_allok:
	return true;
}


bool FontToString(LPLOGFONT lFont, CString &Result)
{
	TCHAR  szBuffer[1024];
	if( !lFont || !szBuffer ) return false;

	int nPixelsPerInch;
	int nFontSize;
	HDC hScreenDC;
	bool bBold = false;
	bool bItalic = false;
	bool bUnderline = false;
	bool bStrikeOut = false;
	TCHAR szBold[2][2]={_T("\0"),_T("b")};
	TCHAR szItalic[2][2]={_T("\0"),_T("i")};
	TCHAR szUnderline[2][2]={_T("\0"),_T("u")};
	TCHAR szStrikeOut[2][2]={_T("\0"),_T("s")};

	hScreenDC = ::GetDC( NULL );

	if( !hScreenDC ) return false;

	nPixelsPerInch = GetDeviceCaps( ::GetDC(0), LOGPIXELSY );

	if( nPixelsPerInch <= 0 ) return false;

	nFontSize = - MulDiv( lFont->lfHeight, 72, nPixelsPerInch );

	if(lFont->lfWeight >= FW_BOLD) bBold = true;

	if(lFont->lfUnderline == (char)true) bUnderline = true;
	if(lFont->lfItalic) bItalic = true;
	if(lFont->lfStrikeOut == (char)true) bStrikeOut = true;

	wsprintf(szBuffer,_T("%s, %d, %s%s%s%s, %d"),lFont->lfFaceName,nFontSize,
				szBold[bBold],szItalic[bItalic],szUnderline[bUnderline],szStrikeOut[bStrikeOut],(int)lFont->lfCharSet);
	Result = szBuffer;
	return true;
}

bool StringToFont(LPCTSTR szBuffer,LPLOGFONT lFont)
{
	TCHAR szFontName[LF_FACESIZE] = _T("Ms Sans Serif");

	TCHAR szFontSize[STRING];
	TCHAR szFormat[STRING];
	TCHAR szCharset[STRING];
	bool bBold=false;
	bool bItalic=false;
	bool bUnderline=false;
	bool bStrikeOut=false;
	int nFontSize=10;
	int nCharSet=ANSI_CHARSET;

	ExtractStrFromList(szBuffer,0,szFontName,sizeof(szFontName)/sizeof(TCHAR));
	if(ExtractStrFromList(szBuffer,1,szFontSize,sizeof(szFontSize)/sizeof(TCHAR)))
	{
		_stscanf(szFontSize,_T("%d"),&nFontSize);
	}

	ExtractStrFromList(szBuffer,2,szFormat,sizeof(szFontSize)/sizeof(TCHAR));

	if(_tcschr(szFormat, 'b')) bBold=true;
	if(_tcschr(szFormat, 'u')) bUnderline=true;
	if(_tcschr(szFormat, 'i')) bItalic=true;
	if(_tcschr(szFormat, 's')) bStrikeOut=true;

	if( ExtractStrFromList(szBuffer,3,szCharset,sizeof(szCharset)/sizeof(TCHAR)))
	{
		_stscanf(szCharset,_T("%d"),&nCharSet);
	}
	
	ZeroMemory(lFont,sizeof(LOGFONT));


	lstrcpy(lFont->lfFaceName,szFontName);
	lFont->lfHeight = -MulDiv(nFontSize, GetDeviceCaps(::GetDC(0), LOGPIXELSY), 72);


	lFont->lfItalic=bItalic;
	lFont->lfStrikeOut=bStrikeOut;
	lFont->lfWeight=bBold?FW_BOLD:FW_NORMAL;
	lFont->lfUnderline=bUnderline;
	lFont->lfCharSet=nCharSet;
	
	return true;
}

LPTSTR ExtractFilePath(LPCTSTR FileName, LPTSTR buf)
{  
	int i, len = lstrlen(FileName);
	for(i=len; i>=0; i--)
	{
		if(FileName[i] == _T('\\'))
			break;
	}
	lstrcpyn(buf, FileName, i+2);
	return buf;
}

//  Function doesn't allocate new string, it returns  pointer
//        to a part of source string
LPCTSTR myExtractFileName(LPCTSTR FileName)
{  
	int i,len = lstrlen(FileName);
	for(i=len; i>=0; i--)
	{
		if(FileName[i] == _T('\\'))
			break;
	}
	return FileName+i+1;
	
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

//  Function doesn't allocate new string, it returns  pointer
//        to a part of source string
LPCTSTR GetFileExt(LPCTSTR szFileName)
{
	if(!szFileName) return 0;
	int nLen = lstrlen(szFileName);
	
	LPCTSTR szReturn = szFileName+nLen;
	for( int i=nLen-1; i>=0; i-- )
	{
		if(szFileName[i] == '.')
		{
			szReturn = szFileName + i + 1;
			break;
		}
		else if(szFileName[i] == '\\') break;
	}
	return szReturn;
}

bool IsStrInList(LPCTSTR szExt,LPCTSTR szList)
{
	if(!szList || !szExt) return false;

	while ((*szList)!=0)
	{
		if(lstrcmpi(szExt,szList)==0) return true;
		szList += lstrlen(szList)+1;
	}
	return false;
}

bool GetOnlyFileName(LPCTSTR szFilename,LPTSTR szBuffer)
{
	if(!szFilename || !szBuffer) return false;

	lstrcpy(szBuffer,myExtractFileName(szFilename));
	int nLen = lstrlen(szBuffer);
	
	
	for( int i=nLen-1; i>=0; i-- )
	{
		if(szBuffer[i] == '.')
		{
			szBuffer[i]=0;
			break;
		}
	}
	return true;
}

bool IsImage(LPCTSTR szFileName)
{
	LPCTSTR szExt = GetFileExt(szFileName);
	if(lstrlen(szExt)<1) return false;
	return IsStrInList(szExt,_T("jpg\0jpeg\0png\0bmp\0gif\0tiff\0\0"));
}

bool IsVideoFile(LPCTSTR szFileName)
{
	LPCTSTR szExt = GetFileExt(szFileName);
	if(lstrlen(szExt)<1) return false;
	return IsStrInList(szExt,VIDEO_FORMATS);
}

bool MySaveImage(Image *img,LPTSTR szFilename,LPTSTR szBuffer,int Format,int Quality)
{
	if(Format==-1) Format=0;
	TCHAR szImgTypes[3][4]={_T("jpg"),_T("png"),_T("gif")};
	TCHAR szMimeTypes[3][12]={_T("image/jpeg"),_T("image/png"),_T("image/gif")};
	TCHAR szNameBuffer[MAX_PATH],szBuffer2[MAX_PATH],TempPath[256];
	GetOnlyFileName(szFilename,szNameBuffer);
	wsprintf(szBuffer2,_T("%s%s%d.%s"),IUTempFolder,szNameBuffer,(int)GetTickCount(),szImgTypes[Format]);
	
	CLSID clsidEncoder;
	EncoderParameters eps;
	eps.Count = 1;
	eps.Parameter[0].Guid = EncoderQuality;
	eps.Parameter[0].Type = EncoderParameterValueTypeLong;
	eps.Parameter[0].NumberOfValues = 1;
	eps.Parameter[0].Value = &Quality;

	if(GetEncoderClsid(szMimeTypes[Format], &clsidEncoder)!=-1)
	{
		if(Format == 0)
			img->Save(szBuffer2,&clsidEncoder,&eps);
		img->Save(szBuffer2,&clsidEncoder);
	}
	lstrcpy(szBuffer,szBuffer2);

	return true;
}

bool ReadSetting(LPTSTR szSettingName,int* Value,int DefaultValue,LPTSTR szString,LPTSTR szDefString)
{
	TCHAR szFileName[256],szPath[256];
	GetModuleFileName(0,szFileName,1023);
   ExtractFilePath(szFileName,szPath);
   wsprintf(szFileName,_T("%simgupload.ini"),szPath);



	TCHAR szBuffer1[128],szBuffer2[128];
	lstrcpy(szBuffer2,GetFileExt(szSettingName));
	GetOnlyFileName(szSettingName,szBuffer1);
	if(!szString)
	*Value = GetPrivateProfileInt(szBuffer1,szBuffer2, DefaultValue, szFileName);
	else
	 GetPrivateProfileString(szBuffer1,szBuffer2, szDefString,szString,256, szFileName);
	
	return true;
}

bool WriteSetting(LPCTSTR szSettingName,int Value,LPCTSTR szString)
{
	TCHAR szFileName[256],szPath[256];
	GetModuleFileName(0,szFileName,1023);
   ExtractFilePath(szFileName,szPath);
   wsprintf(szFileName,_T("%simgupload.ini"),szPath);

	TCHAR szBuffer1[128],szBuffer2[128];
	TCHAR szBuffer3[256];
	
	lstrcpy(szBuffer2,GetFileExt(szSettingName));
	GetOnlyFileName(szSettingName,szBuffer1);
	if(!szString)
	{
		wsprintf(szBuffer3,_T("%d"),Value);
		WritePrivateProfileString(szBuffer1,szBuffer2, szBuffer3, szFileName);
	}
	else
		
	WritePrivateProfileString(szBuffer1,szBuffer2, szString, szFileName);
	
	return true;
}

int GetSavingFormat(LPTSTR szFileName)
{
	if(!szFileName) return -1;
	LPCTSTR FileType = GetFileExt(szFileName);
	if(IsStrInList(FileType,_T("jpg\0jpeg\0\0")))
		return 0;
	else if(IsStrInList(FileType,_T("png\0\0")))
		return 1;
	else if(IsStrInList(FileType,_T("gif\0\0")))
		return 2;
	else return 0;
}

int MyGetFileSize(LPCTSTR FileName)
{
	HANDLE hFile = CreateFile(FileName, /*GENERIC_READ*/0, 0, 0, OPEN_EXISTING, 0, 0);
   if(!hFile) return -1;
	int fSize = GetFileSize (hFile, NULL); 
	CloseHandle(hFile);
	return fSize;
}

void DrawStrokedText(Graphics &gr, LPCTSTR Text,RectF Bounds,Font &font,Color &ColorText,Color &ColorStroke,int HorPos,int VertPos, int width)
{
	RectF OriginalTextRect, NewTextRect;
	FontFamily ff;
	font.GetFamily(&ff);
	gr.SetPageUnit(UnitPixel);
	gr.MeasureString(Text,-1,&font,PointF(0,0),&OriginalTextRect);

	Font NewFont(&ff,48,font.GetStyle(),UnitPixel);
	gr.MeasureString(Text,-1,&NewFont,RectF(0,0,5000,1600),&NewTextRect);
	OriginalTextRect.Height = OriginalTextRect.Height-OriginalTextRect.Y;
	float newwidth,newheight;
	newheight = OriginalTextRect.Height;
	newwidth=OriginalTextRect.Height/NewTextRect.Height*NewTextRect.Width;
	float k = 2*width*NewTextRect.Height/OriginalTextRect.Height;
	SolidBrush br(ColorText);
	Bitmap temp(NewTextRect.Width,NewTextRect.Height,&gr);

	Graphics gr_temp(&temp);
	StringFormat format;
	gr_temp.SetPageUnit(UnitPixel);
	GraphicsPath path;
	gr_temp.SetSmoothingMode( SmoothingModeHighQuality);
	path.AddString(Text, -1,&ff, (int)NewFont.GetStyle(), NewFont.GetSize(), Point(0,0), &format);

	Pen pen(ColorStroke,(float)k);
	pen.SetAlignment(PenAlignmentCenter);

	float x,y;
	gr_temp.DrawPath(&pen, &path);
	gr_temp.FillPath(&br, &path);
	gr.SetSmoothingMode( SmoothingModeHighQuality); 
	gr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );

	if(HorPos == 0)
		x = 2;
	else if(HorPos == 1)
		x = (Bounds.Width-newwidth)/2;
	else x=(Bounds.Width-newwidth)-2;

	if(VertPos==0)
		y=2;
	else if(VertPos==1)
		y=(Bounds.Height-newheight)/2;
	else y=(Bounds.Height-newheight)-2;
	 
	gr.DrawImage(&temp,(int)(Bounds.GetLeft()+x),(int)(Bounds.GetTop()+y),(int)(newwidth),(int)(newheight));
}

void MakeLabelBold(HWND Label)
{
	HFONT Font = reinterpret_cast<HFONT>(SendMessage(Label, WM_GETFONT,0,0));  
	
	if(!Font) return;

	LOGFONT alf;

	if(!::GetObject(Font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT)) return;

	alf.lfWeight = FW_BOLD;

	HFONT NewFont = CreateFontIndirect(&alf);
	SendMessage(Label,WM_SETFONT,(WPARAM)NewFont,MAKELPARAM(false, 0));
	alf.lfHeight = -MulDiv(13, GetDeviceCaps(::GetDC(0), LOGPIXELSY), 72);
}

LPTSTR fgetline(LPTSTR buf,int num,FILE *f)
{
	LPTSTR Result;
	LPTSTR cur;
	Result=_fgetts(buf,num,f);
	int n=lstrlen(buf)-1;

	for(cur=buf+n;cur>=buf;cur--)
	{
		if(*cur==13||*cur==10||*cur==_T('\n'))
			*cur=0;
		else break;
	}
	return Result;
}

CString GetAppFolder()
{
	TCHAR szFileName[256],szPath[256];
	GetModuleFileName(0, szFileName, 1023);
	ExtractFilePath(szFileName, szPath);
	return szPath;
}

BOOL FileExists(LPCTSTR FileName)
{
	if(!FileName || GetFileAttributes(FileName)==-1) return FALSE;
	return TRUE;
}

void TrimString(LPTSTR Destination, LPCTSTR Source,  int MaxLen)
{
	int Len = lstrlen(Source);

	if(Len<=MaxLen) { lstrcpy(Destination, Source); return; }

	int ots = (MaxLen-3)/2;
	lstrcpyn(Destination, Source, ots);
	lstrcat(Destination, _T("..."));
	lstrcat(Destination, Source+Len-ots);
}

bool SelectDialogFilter(LPTSTR szBuffer, int nMaxSize, int nCount, LPCTSTR szName, LPCTSTR szFilter,...)
{
	*szBuffer = 0;
	LPCTSTR *pszName, *pszFilter;
	pszName = &szName;//(LPTSTR*)(&nCount+1);
	pszFilter = &szFilter; //(LPTSTR*)(&nCount+2);

	for(int i=0; i<nCount; i++)
	{
		int nLen = lstrlen(*pszName);
		lstrcpy(szBuffer, *pszName);
		szBuffer[nLen]=0;
		szBuffer+=nLen+1;

		nLen = lstrlen(*pszFilter);
		lstrcpy(szBuffer, *pszFilter);
		szBuffer[nLen]=0;
		szBuffer+=nLen+1;
		pszName+=2;
		pszFilter+=2;
	}
	*szBuffer=0;
	return true;
}

LPCTSTR  CopyToStartOfW(LPCTSTR szString,LPCTSTR szPattern,LPTSTR szBuffer,int nBufferSize)
{
	int nLen=0;
	if(!szString || !szPattern ||!szBuffer || nBufferSize<0) return FALSE;

	LPCTSTR szStart = (LPTSTR) _tcsstr(szString, szPattern);

	if(!szStart) 
	{
		nLen = lstrlen(szString);
		szStart = szString + nLen-1;
	}
	else nLen = szStart - szString;

	if(nLen > nBufferSize-1) nLen = nBufferSize-1;
	lstrcpyn(szBuffer, szString, nLen+1);
	szBuffer[nLen]=0;

	return szStart+1;
}

#define HOTKEY(modifier,key) ((((modifier)&0xff)<<8)|((key)&0xff)) 

// �������� ������ 
// ������� ���������: 
//  pwzShortCutFileName - ���� � ��� ������, ��������, "C:\\�������.lnk" 
//  ���� �� ������ ����, ����� ����� ������ � �����, ��������� � ��������� ���������. 
//  ����.: Windows ���� �� ��������� � ����� ���������� .lnk 
//  pszPathAndFileName  - ���� � ��� exe-�����, ��������, "C:\\Windows\\NotePad.Exe" 
//  pszWorkingDirectory - ������� �������, ��������, "C:\\Windows" 
//  pszArguments        - ��������� ��������� ������, ��������, "C:\\Doc\\Text.Txt" 
//  wHotKey             - ������� �������, ��������, ��� Ctrl+Alt+A     HOTKEY(HOTKEYF_ALT|HOTKEYF_CONTROL,'A') 
//  iCmdShow            - ��������� ���, ��������, SW_SHOWNORMAL 
//  pszIconFileName     - ���� � ��� �����, ����������� ������, ��������, "C:\\Windows\\NotePad.Exe" 
//  int iIconIndex      - ������ ������ � �����, ���������� � 0 
bool __fastcall CreateShortCut( 
							   LPCWSTR pwzShortCutFileName, 
							   LPCTSTR pszPathAndFileName, 
							   LPCTSTR pszWorkingDirectory, 
							   LPCTSTR pszArguments, 
							   WORD wHotKey, 
							   int iCmdShow, 
							   LPCTSTR pszIconFileName, 
							   int iIconIndex) 
{ 
					   IShellLink * pSL; 
	IPersistFile * pPF; 
	HRESULT hRes; 
	if( CoInitialize(NULL) != S_OK);
		//return false;
	// ��������� ���������� ���������� "�����" 
	hRes = CoCreateInstance(CLSID_ShellLink, 0,	CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pSL); 

	if( SUCCEEDED(hRes) ) 
	{ 
		hRes = pSL->SetPath(pszPathAndFileName); 
		if( SUCCEEDED(hRes) ) 
		{ 
			hRes = pSL->SetArguments(pszArguments); 
			//if( SUCCEEDED(hRes) ) 
			{ 
				hRes = pSL->SetWorkingDirectory(pszWorkingDirectory); 
				if( SUCCEEDED(hRes) ) 
				{ 
					hRes = pSL->SetIconLocation(pszIconFileName,iIconIndex); 
					if( SUCCEEDED(hRes) ) 
					{ 
					//	hRes = pSL->SetHotkey(wHotKey); 
					//	if( SUCCEEDED(hRes) ) 
						{ 
							hRes = pSL->SetShowCmd(iCmdShow); 
							if( SUCCEEDED(hRes) ) 
							{ 
								// ��������� ���������� ��������� ���������� 
								hRes = pSL->QueryInterface(IID_IPersistFile,(LPVOID *)&pPF); 
								if( SUCCEEDED(hRes) ) 
								{ 
									// ���������� ���������� ������ 
									hRes = pPF->Save(pwzShortCutFileName,TRUE); 
									pPF->Release(); 
								} 
							} 
						} 
					} 
				} 
			} 
		} 
		pSL->Release(); 
	} 
	return SUCCEEDED(hRes); 

}  

#undef PixelFormat8bppIndexed 
#define PixelFormat8bppIndexed (3 | ( 8 << 8) | PixelFormatIndexed | PixelFormatGDI)


void FillRectGradient(HDC hdc, RECT FillRect, COLORREF start, COLORREF finish, bool Horizontal)
{
	RECT rectFill;          
	float fStep;            //The size of each band in pixels
	HBRUSH hBrush;
	int i;  // Loop index

	float r, g, b;
	int n = 256;
	FillRect.bottom--;
	COLORREF c;
  
	if(!Horizontal)
		fStep = (float)(FillRect.bottom - FillRect.top) / 256;
	else 
		fStep = (float)(FillRect.right - FillRect.left) / 256;

	if( fStep < 1)
	{
		fStep = 1;
		if(!Horizontal)
			n = FillRect.bottom - FillRect.top;
		else 
			n = (FillRect.right - FillRect.left);
	}

	r = (float)(GetRValue(finish)-GetRValue(start))/(n-1);

	g = (float)(GetGValue(finish)-GetGValue(start))/(n-1);

	b = (float)(GetBValue(finish)-GetBValue(start))/(n-1);
	
	//������ ����������
	for (i = 0; i < n; i++) 
	{
		//������������ �� ����, ��� �� - �������������� ��� ������������ ��������
		if(!Horizontal)
			SetRect(&rectFill, FillRect.left, (int)(i * fStep)+FillRect.top,
							FillRect.right+1, (int)FillRect.top+(i+1) * fStep); 
		else 
			SetRect(&rectFill, (int)FillRect.left+(i * fStep), FillRect.top,
						(int)FillRect.left+((i+1) * fStep), FillRect.bottom+1); 
		if(i == n-1)
			c = finish;
		else
			c = RGB((int)GetRValue(start)+(r*i/**zR*/),(int)GetGValue(start)+(g*i/*zG*/),(int)GetBValue(start)+(b*i/**zB*/));
	   
		hBrush=CreateSolidBrush(c);
	  
		::FillRect(hdc, &rectFill, hBrush);

		DeleteObject(hBrush);
  }
}

CString DisplayError(int idCode)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,
						idCode, 0, (LPTSTR) &lpMsgBuf, 0, NULL);
	CString res = (LPCTSTR)lpMsgBuf;
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return res;
}


/* MakeFontBold
	MakeFontUnderLine

	-----------------------
	These functions create bold/underlined fonts based on given font
*/
HFONT MakeFontBold(HFONT font)
{
	if(!font) return 0;

	LOGFONT alf;

	bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

	if(!ok) return 0;

	alf.lfWeight = FW_BOLD;

	HFONT NewFont = CreateFontIndirect(&alf);
	return NewFont;
}

HFONT MakeFontUnderLine(HFONT font)
{
	if(!font) return 0;

	LOGFONT alf;

	bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

	if(!ok) return 0;

	alf.lfUnderline = 1;
	HFONT NewFont = CreateFontIndirect(&alf);

	return NewFont;
}


LPTSTR MoveToEndOfW(LPTSTR szString,LPTSTR szPattern)
{
	int nLen;

	if(!szString || !szPattern) return szString;

	nLen = wcslen(szPattern);
	if(!nLen) return szString;
	
	LPTSTR szStart = (LPTSTR) wcsstr(szString, szPattern);

	if(!szStart) return szString;
	else szString = szStart+nLen;

	return szString;
}

#ifdef DEBUG

void ShowX(LPCTSTR str,int line,int n)
{
	TCHAR buf[MAX_PATH];
	wsprintf(buf,_T("Str %d : %s = %d"),line,str,n);
	::MessageBox(0,buf,0,0);
}
void ShowX(LPCTSTR str,int line,float n)
{
	TCHAR buf[MAX_PATH];
	wsprintf(buf,_T("Str %d : %s = %f"),line,str,n);
	::MessageBox(0,buf,0,0);
}

void ShowX(LPCTSTR str,int line,LPCTSTR n)
{
	TCHAR buf[MAX_PATH];
	wsprintf(buf,_T("Str %d : %s = %s"),line,str,n);
	::MessageBox(0,buf,0,0);
}
#endif


void EnableNextN(HWND Control ,int n, bool Enable)
{
	for(int i=0;i< n; i++)
	{
		Control = GetNextWindow(Control, GW_HWNDNEXT);
		EnableWindow(Control, Enable);
	}
}