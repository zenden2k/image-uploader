#ifndef IU_CORE_SCRIPTAPI_WEBBROWSER_H
#define IU_CORE_SCRIPTAPI_WEBBROWSER_H

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
			
			friend class WebBrowserPrivate;
		protected:
			WebBrowserPrivate *d_;
	};

	void RegisterWebBrowserClass();
}

#endif
