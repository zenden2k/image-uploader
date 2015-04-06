/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#ifndef IU_CORE_SCRIPTAPI_WEBBROWSER_H
#define IU_CORE_SCRIPTAPI_WEBBROWSER_H

#pragma once 

#include <Core/Squirrelnc.h>
#include "HtmlDocument.h"

namespace ScriptAPI {
	class WebBrowserPrivate;
	class CWebBrowser {
		public:
			CWebBrowser();
			~CWebBrowser();
			bool navigateToUrl(const std::string& url);
			void setTitle(const std::string& title);
			const std::string url();
			const std::string title();
			bool showModal();
			bool exec();
			void show();
			void hide();
			void close();
			void setFocus();
			void setOnUrlChangedCallback(SquirrelObject callBack, SquirrelObject context);
			void setOnNavigateErrorCallback(SquirrelObject callBack, SquirrelObject context);
			void setOnLoadFinishedCallback(SquirrelObject callBack, SquirrelObject context);
			void setOnTimerCallback(int timerInterval, SquirrelObject callBack, SquirrelObject context);
			void setOnFileInputFilledCallback(SquirrelObject callBack, SquirrelObject context);
			const std::string getDocumentContents();
			HtmlDocument document();
			bool setHtml(const std::string& html);
			const std::string runJavaScript(const std::string& code);
			const std::string callJavaScriptFunction(const std::string& funcName, SquirrelObject args);
			void setSilent(bool silent);
			void addTrustedSite(const std::string& domain);
			int getMajorVersion();
			
			friend class WebBrowserPrivate;
		protected:
			WebBrowserPrivate *d_;
	};

	void RegisterWebBrowserClass();
}

#endif
