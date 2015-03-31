#include "WebBrowser.h"
//using namespace ScriptAPI;


#ifdef _WIN32
#include "WebBrowserPrivate_win.h"
#else
// Not implemented
#endif


using namespace ScriptAPI;
DECLARE_INSTANCE_TYPE(CWebBrowser);
DECLARE_INSTANCE_TYPE(HtmlDocument);

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

void CWebBrowser::setOnUrlChangedCallback(SquirrelObject callBack, SquirrelObject context)
{
	d_->setOnUrlChangedCallback(callBack, context);
}

void CWebBrowser::setOnNavigateErrorCallback(SquirrelObject callBack, SquirrelObject context)
{
	d_->setOnNavigateErrorCallback(callBack, context);
}

void CWebBrowser::setOnLoadFinishedCallback(SquirrelObject callBack, SquirrelObject context)
{
	d_->setOnLoadFinishedCallback(callBack, context);
}

void CWebBrowser::setOnTimerCallback(int timerInterval, SquirrelObject callBack, SquirrelObject context)
{
	d_->setOnTimerCallback(timerInterval,  callBack, context);
}

void CWebBrowser::setOnFileInputFilledCallback(SquirrelObject callBack, SquirrelObject context)
{
	d_->setOnFileInputFilledCallback(callBack, context);
}

const std::string CWebBrowser::getDocumentContents()
{
	return d_->getDocumentContents();
}

ScriptAPI::HtmlDocument CWebBrowser::document()
{
	return d_->document();
}	

bool CWebBrowser::setHtml(const std::string& html)
{
	return d_->setHtml(html);
}

const std::string CWebBrowser::runJavaScript(const std::string& code)
{
	return d_->runJavaScript(code);
}

const std::string CWebBrowser::callJavaScriptFunction(const std::string& funcName, SquirrelObject args)
{
	return d_->callJavaScriptFunction(funcName, args);
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

void RegisterWebBrowserClass() {
	using namespace SqPlus;
	SQClassDef<CWebBrowser>("CWebBrowser")
		.func(&CWebBrowser::navigateToUrl, "navigateToUrl")
		.func(&CWebBrowser::showModal, "showModal")
		.func(&CWebBrowser::setTitle, "setTitle")
		.func(&CWebBrowser::exec, "exec")
		.func(&CWebBrowser::show, "show")
		.func(&CWebBrowser::hide, "hide")
		.func(&CWebBrowser::close, "close")
		.func(&CWebBrowser::url, "url")
		.func(&CWebBrowser::title, "title")
		.func(&CWebBrowser::getDocumentContents, "getDocumentContents")
		.func(&CWebBrowser::document, "document")
		.func(&CWebBrowser::setHtml, "setHtml")
		.func(&CWebBrowser::runJavaScript, "runJavaScript")
		.func(&CWebBrowser::callJavaScriptFunction, "callJavaScriptFunction")
		.func(&CWebBrowser::setOnUrlChangedCallback, "setOnUrlChangedCallback")
		.func(&CWebBrowser::setOnLoadFinishedCallback, "setOnLoadFinishedCallback")
		.func(&CWebBrowser::setOnTimerCallback, "setOnTimerCallback")
		.func(&CWebBrowser::setOnFileInputFilledCallback, "setOnFileInputFilledCallback")
		.func(&CWebBrowser::setOnNavigateErrorCallback, "setOnNavigateErrorCallback");
}

}