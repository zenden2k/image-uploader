/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef SETTINGS_READ
	#define _XML_OPTION_VALUE(name,value) if (XML.FindElem(_CRT_WIDE(#name))) {/*XML.IntoElem();*/ XML.GetData(value);  /*ShowVar(value);ShowVar(_CRT_WIDE(#name));*/  /* XML.OutOfElem();*/}
	#define _XML_OPTION_ATTR(name,value) XML.GetAttrib(_CRT_WIDE(#name),value) 
	#define XML_NODE_START(name,value) if (XML.FindElem(_CRT_WIDE(#name))) {XML.IntoElem();
	#define XML_NODE_START_WITHATTRIB(name,value) if (XML.FindElem(_CRT_WIDE(#name))) {
	#define XML_NODE_STARTS(name,value) if (XML.FindElem(name)) {
	#define XML_NODE_START_VALUE(name,value) if (XML.FindElem(_CRT_WIDE(#name))) { XML.GetData(value);XML.IntoElem();
	#define XML_NODE_START_VALUE_WITHATTRIB(name,value) if (XML.FindElem(_CRT_WIDE(#name))) { XML.GetData(value);
	
	#define XML_NODE_END()  \
		XML.OutOfElem(); \
	}
	#define XML_OPTION_VALUE(name) _XML_OPTION_VALUE(name,name)

	#define XML_OPTION_MEMBER_VALUE(prefix, name) _XML_OPTION_VALUE(name, prefix##.name)

	/**  attributes */
	#define XML_OPTION_ATTR(name) _XML_OPTION_ATTR(name,name)

	//#define XML_OPTION_ATTR_STRING(name, size) _XML_OPTION_ATTR_STRING(name, name, size)

	#define XML_OPTION_MEMBER_ATTR(prefix, name) _XML_OPTION_ATTR(name, prefix##.name)
	//#define XML_OPTION_MEMBER_ATTR_STRING(prefix, name, size) _XML_OPTION_ATTR_STRING(name, prefix##.##name, size)

	#define COLOR_TO_PINT(c) (*(int*)&##c)
#else
	// SAVING MACROSES
		//#define _XML_OPTION_VALUE_STRING(name,value,size) XML.AddElement(_CRT_WIDE(#name),value) ; /*XML.OutOfElem();*/
	#define _XML_OPTION_VALUE(name,value) XML.AddElem(_CRT_WIDE(#name),value) ; /*XML.OutOfElem();*/
	
	//#define _XML_OPTION_ATTR_STRING(name,value,size) XML.AddElementAttr(_CRT_WIDE(#name),value) 
	#define _XML_OPTION_ATTR(name,value) XML.SetAttrib(_CRT_WIDE(#name),value) 

	#define XML_NODE_START(name,value) XML.AddElem(_CRT_WIDE(#name)); {XML.IntoElem();
	#define XML_NODE_START_WITHATTRIB(name,value) XML.AddElem(_CRT_WIDE(#name)); {

	#define XML_NODE_STARTS(name,value) XML.AddElem(name); {XML.IntoElem();
	#define XML_NODE_START_VALUE(name,value)  XML.AddElem(_CRT_WIDE(#name),value); { XML.IntoElem();
	#define XML_NODE_START_VALUE_WITHATTRIB(name,value)  XML.AddElem(_CRT_WIDE(#name),value); { 

	//#define XML_NODE_START_VALUE_STRING(name,value) XML.AddElement(_CRT_WIDE(#name),value,true); { 
	
	#define XML_NODE_END()  \
			XML.OutOfElem(); \
	}
	#define XML_OPTION_VALUE(name) _XML_OPTION_VALUE(name,name)
	#define XML_OPTION_VALUE_STRING(name, size) _XML_OPTION_VALUE_STRING(name, name, size)

	#define XML_OPTION_MEMBER_VALUE(prefix, name) _XML_OPTION_VALUE(name, prefix##.name)
	#define XML_OPTION_MEMBER_VALUE_STRING(prefix, name, size) _XML_OPTION_VALUE_STRING(name, prefix##.##name, size)

	/**  attributes */
	#define XML_OPTION_ATTR(name) _XML_OPTION_ATTR(name,name)
	#define XML_OPTION_ATTR_STRING(name, size) _XML_OPTION_ATTR_STRING(name, name, size)

	#define XML_OPTION_MEMBER_ATTR(prefix, name) _XML_OPTION_ATTR(name, prefix##.name)
	#define XML_OPTION_MEMBER_ATTR_STRING(prefix, name, size) _XML_OPTION_ATTR_STRING(name, prefix##.##name, size)

	#define COLOR_TO_PINT(c) (*(int*)&##c)
#endif



#ifdef SETTINGS_READ

bool CSettings::MacroLoadSettings(CMyXml &XML)
{
#else
 	bool CSettings::MacroSaveSettings(CMyXml &XML)
	{
#endif
	TCHAR szFont[1024];
CString Font;

	XML_NODE_START(ImageUploader);
	XML_NODE_START(Settings);
	
	XML_NODE_START(General);
		XML_OPTION_MEMBER_VALUE(Settings,Language);
		
		
		XML_OPTION_VALUE(ExplorerContextMenu);
		XML_OPTION_VALUE(ExplorerVideoContextMenu);
		XML_OPTION_VALUE(ExplorerCascadedMenu);
		#ifndef IU_SHELLEXT
		XML_OPTION_VALUE(ConfirmOnExit);
		XML_OPTION_VALUE(SendToContextMenu);
		XML_OPTION_VALUE(ParseSubDirs);
		XML_OPTION_VALUE(ImageEditorPath);
		XML_OPTION_VALUE(ShowTrayIcon);
		XML_OPTION_VALUE(AutoCopyToClipboard);
		XML_OPTION_VALUE(AutoShowLog);
		XML_OPTION_VALUE(ImagesFolder);
		XML_OPTION_VALUE(VideoFolder);
		CString HotkeysStr;
		#ifndef  SETTINGS_READ
			 HotkeysStr= Settings.Hotkeys.toString();
				//FontToString(&LogoSettings.Font,Font);			
		#endif
			
		XML_OPTION_VALUE(HotkeysStr);

			#ifdef  SETTINGS_READ
				Settings.Hotkeys.DeSerialize(HotkeysStr);
				//StringToFont(Font, &LogoSettings.Font);
			#endif
		#endif
	XML_NODE_END();
		#ifndef IU_SHELLEXT
	XML_NODE_START(Screenshot);
		XML_OPTION_MEMBER_VALUE(ScreenshotSettings,Delay);
		XML_OPTION_MEMBER_VALUE(ScreenshotSettings,Format);
		XML_OPTION_MEMBER_VALUE(ScreenshotSettings,Quality);
		_XML_OPTION_ATTR(brushColor, COLOR_TO_PINT(ScreenshotSettings.brushColor));
	XML_NODE_END();
	
	XML_NODE_START(Image);
		XML_OPTION_MEMBER_VALUE(ImageSettings, Quality);
		XML_OPTION_MEMBER_VALUE(ImageSettings, Format);
		XML_OPTION_MEMBER_VALUE(ImageSettings, KeepAsIs);
		XML_OPTION_MEMBER_VALUE(ImageSettings, NewWidth);
		XML_OPTION_MEMBER_VALUE(ImageSettings, NewHeight);
		XML_OPTION_MEMBER_VALUE(ImageSettings, AddLogo);
		XML_OPTION_MEMBER_VALUE(ImageSettings, AddText);

		XML_NODE_START_VALUE_WITHATTRIB(Logo, LogoSettings.FileName);
			XML_OPTION_MEMBER_ATTR(LogoSettings,LogoPosition);
			XML_OPTION_MEMBER_ATTR(LogoSettings, LogoBlend);
			XML.IntoElem();
		XML_NODE_END();
				
			
		XML_NODE_START_VALUE_WITHATTRIB(Text, LogoSettings.Text);
			XML_OPTION_MEMBER_ATTR(LogoSettings, TextPosition);
			_XML_OPTION_ATTR(TextColor, COLOR_TO_PINT(LogoSettings.TextColor));

			
			#ifndef  SETTINGS_READ
				FontToString(&LogoSettings.Font,Font);			
			#endif
			
			XML_OPTION_ATTR(Font);

			#ifdef  SETTINGS_READ
				StringToFont(Font, &LogoSettings.Font);
			#endif
				XML.IntoElem();

		XML_NODE_END();
	XML_NODE_END();


	
		XML_NODE_START(Thumbnails);
			XML_OPTION_MEMBER_VALUE(ThumbSettings, CreateThumbs);
			XML_OPTION_MEMBER_VALUE(ThumbSettings, ThumbWidth);
			_XML_OPTION_VALUE(FrameColor, COLOR_TO_PINT(ThumbSettings.FrameColor));
			_XML_OPTION_VALUE(StrokeColor, COLOR_TO_PINT(LogoSettings.StrokeColor));
			_XML_OPTION_VALUE(ThumbColor1, COLOR_TO_PINT(ThumbSettings.ThumbColor1));
			_XML_OPTION_VALUE(ThumbColor2, COLOR_TO_PINT(ThumbSettings.ThumbColor2));
				XML_OPTION_MEMBER_VALUE(ThumbSettings, UseServerThumbs);
				XML_OPTION_MEMBER_VALUE(ThumbSettings, UseThumbTemplate);

			XML_NODE_START_VALUE_WITHATTRIB(Text, ThumbSettings.Text);
			_XML_OPTION_ATTR(Color, COLOR_TO_PINT(ThumbSettings.ThumbTextColor));

			#ifndef  SETTINGS_READ
				FontToString(&ThumbSettings.ThumbFont,Font);
			
			#endif
			
			XML_OPTION_ATTR(Font);

			#ifdef  SETTINGS_READ
				StringToFont(Font, &ThumbSettings.ThumbFont);
			#endif
				XML_OPTION_MEMBER_ATTR(ThumbSettings,TextOverThumb);
				XML_OPTION_MEMBER_ATTR(ThumbSettings, ThumbAlpha);
				XML.IntoElem();

				XML_NODE_END(); /* end of Thumbnails */
		XML_NODE_END();

	XML_NODE_START(VideoGrabber);
		XML_OPTION_MEMBER_VALUE(VideoSettings, Columns);
		XML_OPTION_MEMBER_VALUE(VideoSettings, TileWidth);
		XML_OPTION_MEMBER_VALUE(VideoSettings, GapWidth);
		XML_OPTION_MEMBER_VALUE(VideoSettings, GapHeight);
		XML_OPTION_MEMBER_VALUE(VideoSettings, NumOfFrames);
		XML_OPTION_MEMBER_VALUE(VideoSettings,JPEGQuality);
	XML_NODE_END(); /* end of VideoGrabber */

	XML_NODE_START(TrayIcon);
		XML_OPTION_MEMBER_VALUE(TrayIconSettings, LeftDoubleClickCommand);
		XML_OPTION_MEMBER_VALUE(TrayIconSettings, LeftClickCommand);
		XML_OPTION_MEMBER_VALUE(TrayIconSettings, RightClickCommand);
		XML_OPTION_MEMBER_VALUE(TrayIconSettings, MiddleClickCommand);
		XML_OPTION_MEMBER_VALUE(TrayIconSettings, DontLaunchCopy);
	XML_NODE_END(); /* end of VideoGrabber */
	
		
	XML_NODE_START(Uploading);
		XML_OPTION_VALUE(ServerName);
		XML_OPTION_VALUE(FileServerName);
		XML_OPTION_VALUE(QuickUpload);
		XML_OPTION_VALUE(QuickServerName);
		XML_OPTION_VALUE(CodeLang);
		XML_OPTION_VALUE(ThumbsPerLine);
		XML_OPTION_VALUE(UploadBufferSize);
		XML_OPTION_VALUE(UseTxtTemplate);
		XML_OPTION_VALUE(CodeType);
		XML_OPTION_VALUE(FileRetryLimit);
		XML_OPTION_VALUE(ShowUploadErrorDialog);
		XML_OPTION_VALUE(ActionRetryLimit);
		XML_NODE_START_WITHATTRIB(Proxy);	
			XML_OPTION_MEMBER_ATTR(ConnectionSettings,UseProxy);
			XML_OPTION_MEMBER_ATTR(ConnectionSettings, NeedsAuth);
			XML.IntoElem();
			XML_OPTION_MEMBER_VALUE(ConnectionSettings,ServerAddress);
			XML_OPTION_MEMBER_VALUE(ConnectionSettings, ProxyPort);
			XML_OPTION_MEMBER_VALUE(ConnectionSettings, ProxyType);
			XML_OPTION_MEMBER_VALUE(ConnectionSettings, ProxyUser);
			CString buf;

			#ifndef SETTINGS_READ
			EncodeString(ConnectionSettings.ProxyPassword, buf );
			#endif
			_XML_OPTION_VALUE(Password,buf);

			#ifdef SETTINGS_READ
			DecodeString( buf, ConnectionSettings.ProxyPassword);
			#endif
		XML_NODE_END(); 

	XML_NODE_END();

	#ifdef SETTINGS_READ
		XML_NODE_START(ServersParams);
		while(XML.FindElem(_T("Server")))
		{
			CString v;
		(XML.GetAttrib(_T("Name"),v));
	
	XML.GetAttrib(_T("Auth"),AuthParams[v].UseIeCookies);
	CString Buffer;

	XML.GetAttrib(_T("Login"),Buffer);
	DecodeString(Buffer, AuthParams[v].Login);
	
	XML.GetAttrib(_T("Password"),Buffer);
	DecodeString(Buffer, AuthParams[v].Password);
	}
XML_NODE_END();
#else
	XML_NODE_START(ServersParams);
		std::map<CString, LoginInfo>::iterator i;
		for(i=AuthParams.begin(); i!=AuthParams.end(); i++)		
		{
			XML.IntoElem();
			
			if(XML.AddElem(_T("Server")))
			{
				(XML.SetAttrib(_T("Name"),i->first));
				XML.SetAttrib(_T("Auth"),i->second.UseIeCookies);
				CString Buffer;
				EncodeString(i->second.Login,Buffer );
				XML.SetAttrib(_T("Login"),Buffer);
				EncodeString( i->second.Password, Buffer);
				XML.SetAttrib(_T("Password"),Buffer);
				XML.OutOfElem();
			}
		}
	XML_NODE_END();
#endif
	#endif
	XML_NODE_END(); // end of settings
	XML_NODE_END(); // end of image uploader

	return true;
}
