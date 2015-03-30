#ifndef IU_CORE_SCRIPTAPI_WEBBROWSER_H
#define IU_CORE_SCRIPTAPI_WEBBROWSER_H

#include <Core/Squirrelnc.h>

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
			void setOnUrlChangedCallback(SquirrelObject callBack, SquirrelObject context);
			void setOnNavigateErrorCallback(SquirrelObject callBack, SquirrelObject context);
			void setOnLoadFinishedCallback(SquirrelObject callBack, SquirrelObject context);
			const std::string getDocumentBody();
			bool setHtml(const std::string& html);
			bool injectJavaScript(const std::string& code);
			SquirrelObject callJavaScript(const std::string& funcName, SquirrelObject args);
			
			friend class WebBrowserPrivate;
		protected:
			WebBrowserPrivate *d_;
	};

	void RegisterWebBrowserClass();
}

#endif
