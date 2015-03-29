#ifndef IU_CORE_SCRIPTAPI_WEBBROWSER_H
#define IU_CORE_SCRIPTAPI_WEBBROWSER_H

#include <Core/Squirrelnc.h>

namespace ScriptAPI {
	class WebBrowserPrivate;
	class WebBrowser {
		public:
			WebBrowser();
			~WebBrowser();
			bool navigateToUrl(const std::string& url);
			void setTitle(const std::string& title);
			const std::string url();
			const std::string title();
			bool openModal();
			bool exec();
			void show();
			void hide();
			void close();
			void setOnUrlChangedCallback(SquirrelObject callBack, SquirrelObject context);
			void setOnNavigateErrorCallback(SquirrelObject callBack, SquirrelObject context);
			void setOnLoadFinishedCallback(SquirrelObject callBack, SquirrelObject context);
			
			friend class WebBrowserPrivate;
		protected:
			WebBrowserPrivate *d_;
	};

	void RegisterWebBrowserClass();
}

#endif
