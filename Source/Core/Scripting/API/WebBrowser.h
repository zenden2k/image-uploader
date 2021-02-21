/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include "Core/Scripting/Squirrelnc.h"
#include "HtmlDocument.h"

namespace ScriptAPI {;
class WebBrowserPrivate;
    /**
    Represents a web browser window. It is using Internet Explorer components on Windows, 
    but is not implemented on other systems. 
    Creating an instance: local webBrowser = CWebBrowser();
    @since 1.3.1.4270.
    */
    class CWebBrowser {
        public:
            CWebBrowser();
            ~CWebBrowser();
            bool navigateToUrl(const std::string& url);
            void setTitle(const std::string& title);
            /**
            Returns current url.
            */
            const std::string url();

            /**
            Returns current page title or url if title is empty.
            */
            const std::string title();

            /**
            Show the web browser window and wait until it is closed. 
            @return true (if the window was closed programatically), false - if the window was closed by user.
            */
            bool showModal();

            /**
            Mostly the same as showModal but the window may be hidden and does not block parent window.
            */
            bool exec();
            void show();
            void hide();

            /**
            Close the window. After call to this function, 
            showModal() or exec() stops execution and returns true.
            */
            void close();
            void setFocus();
                    
            /**
            Returns current page contents.
            */
            const std::string getDocumentContents();
            bool setHtml(const std::string& html);
            const std::string runJavaScript(const std::string& code);
            const std::string callJavaScriptFunction(const std::string& funcName, Sqrat::Object args);
            void setSilent(bool silent);
            void addTrustedSite(const std::string& domain);
            int getMajorVersion();

            /**
             * Arguments passed to callback:
            <b>data</b> - a table containing:
            @code
            {
                browser = WebBrowser,
                url = string
            }
            @endcode
             */
            void setOnUrlChangedCallback(Sqrat::Function callBack, Sqrat::Object context);

            /**
             * Arguments passed to callback:
                <b>data</b> - a table containing:
                @code
                {
                  browser = WebBrowser,
                  url = string,
                  statusCode = int
                }
                @endcode
             */
            void setOnNavigateErrorCallback(Sqrat::Function callBack, Sqrat::Object context);

            /**
             * Arguments passed to callback:
                <b>data</b> - a table containing:
                @code
                {
                  browser = WebBrowser,
                  url = string
                }
                @endcode
             */
            void setOnLoadFinishedCallback(Sqrat::Function callBack, Sqrat::Object context);
            
            friend class WebBrowserPrivate;
        protected:
            WebBrowserPrivate *d_;
    };

    /* @cond PRIVATE */
    void RegisterWebBrowserClass(Sqrat::SqratVM& vm);
    /* @endcond */
}

#endif
