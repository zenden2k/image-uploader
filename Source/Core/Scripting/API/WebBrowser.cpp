/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "WebBrowser.h"

#ifdef _WIN32
#include "WebBrowserPrivate_win.h"
#else
// Not implemented
#endif

namespace ScriptAPI {


CWebBrowser::CWebBrowser()
{
    d_ = new WebBrowserPrivate(this);
}

CWebBrowser::~CWebBrowser()
{
    delete d_;
}

bool CWebBrowser::navigateToUrl(const std::string& url)
{
    return d_->navigateToUrl(url);
}

void CWebBrowser::setOnUrlChangedCallback(Sqrat::Function callBack, Sqrat::Object context)
{
    d_->setOnUrlChangedCallback(callBack, context);
}

void CWebBrowser::setOnNavigateErrorCallback(Sqrat::Function callBack, Sqrat::Object context)
{
    d_->setOnNavigateErrorCallback(callBack, context);
}

void CWebBrowser::setOnLoadFinishedCallback(Sqrat::Function callBack, Sqrat::Object context)
{
    d_->setOnLoadFinishedCallback(callBack, context);
}


const std::string CWebBrowser::getDocumentContents()
{
    return d_->getDocumentContents();
}


bool CWebBrowser::setHtml(const std::string& html)
{
    return d_->setHtml(html);
}

const std::string CWebBrowser::runJavaScript(const std::string& code)
{
    return d_->runJavaScript(code);
}

const std::string CWebBrowser::callJavaScriptFunction(const std::string& funcName, Sqrat::Object args)
{
    return d_->callJavaScriptFunction(funcName, args);
}

void CWebBrowser::addTrustedSite(const std::string& domain)
{
    d_->addTrustedSite(domain);
}

int CWebBrowser::getMajorVersion()
{
    return d_->getMajorVersion();
}

bool CWebBrowser::showModal()
{
    return d_->showModal();
}

bool CWebBrowser::exec()
{
    return d_->exec();
}

void CWebBrowser::show()
{
    d_->show();
}

void CWebBrowser::hide()
{
    d_->hide();
}

void CWebBrowser::close()
{
    d_->close();
}

void CWebBrowser::setFocus()
{
    d_->setFocus();
}

void CWebBrowser::setTitle(const std::string& title)
{
    d_->setTitle(title);
}

const std::string CWebBrowser::url()
{
    return d_->url();
}

const std::string CWebBrowser::title()
{
    return d_->title();
}

void RegisterWebBrowserClass(Sqrat::SqratVM& vm) {
    using namespace Sqrat;
    vm.GetRootTable().Bind("CWebBrowser", Class<CWebBrowser>(vm.GetVM(), "CWebBrowser")
        .Func("navigateToUrl", &CWebBrowser::navigateToUrl)
        .Func("showModal", &CWebBrowser::showModal)
        .Func("setTitle", &CWebBrowser::setTitle)
        .Func("exec", &CWebBrowser::exec)
        .Func("show", &CWebBrowser::show)
        .Func("hide", &CWebBrowser::hide)
        .Func("close", &CWebBrowser::close)
        .Func("url", &CWebBrowser::url)
        .Func("title", &CWebBrowser::title)
        .Func("getDocumentContents", &CWebBrowser::getDocumentContents)
        .Func("setHtml", &CWebBrowser::setHtml)
        .Func("runJavaScript", &CWebBrowser::runJavaScript)
        .Func("addTrustedSite", &CWebBrowser::addTrustedSite)
        .Func("getMajorVersion", &CWebBrowser::getMajorVersion)
        .Func("callJavaScriptFunction", &CWebBrowser::callJavaScriptFunction)
        .Func("setOnUrlChangedCallback", &CWebBrowser::setOnUrlChangedCallback)
        .Func("setOnLoadFinishedCallback", &CWebBrowser::setOnLoadFinishedCallback)
        .Func("setOnNavigateErrorCallback", &CWebBrowser::setOnNavigateErrorCallback)
    );
}

}