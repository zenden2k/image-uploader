#include "WebBrowser.h"
//using namespace ScriptAPI;


#ifdef _WIN32
#include "WebBrowserPrivate_win.h"
#else
// Not implemented
#endif


DECLARE_INSTANCE_TYPE_NAME_CUSTOM(ScriptAPI::WebBrowser, "WebBrowser");

namespace ScriptAPI {


WebBrowser::WebBrowser()
{
	d_ = new WebBrowserPrivate(this);
}

WebBrowser::~WebBrowser()
{
	delete d_;
}

bool WebBrowser::navigateToUrl(const std::string& url)
{
	return d_->navigateToUrl(url);
}

void WebBrowser::setOnUrlChangedCallback(SquirrelObject callBack, SquirrelObject context)
{
	d_->setOnUrlChangedCallback(callBack, context);
}

void WebBrowser::setOnNavigateErrorCallback(SquirrelObject callBack, SquirrelObject context)
{
	d_->setOnNavigateErrorCallback(callBack, context);
}

void WebBrowser::setOnLoadFinishedCallback(SquirrelObject callBack, SquirrelObject context)
{
	d_->setOnLoadFinishedCallback(callBack, context);
}

bool WebBrowser::openModal()
{
	return d_->openModal();
}

bool WebBrowser::exec()
{
	return d_->exec();
}

void WebBrowser::show()
{
	d_->show();
}

void WebBrowser::hide()
{
	d_->hide();
}

void WebBrowser::close()
{
	d_->close();
}

void WebBrowser::setTitle(const std::string& title)
{
	d_->setTitle(title);
}

const std::string WebBrowser::url()
{
	return d_->url();
}

const std::string WebBrowser::title()
{
	return d_->title();
}

void RegisterWebBrowserClass() {
	using namespace SqPlus;
	SQClassDef<WebBrowser>("WebBrowser")
		.func(&WebBrowser::navigateToUrl, "navigateToUrl")
		.func(&WebBrowser::openModal, "openModal")
		.func(&WebBrowser::setTitle, "setTitle")
		.func(&WebBrowser::exec, "exec")
		.func(&WebBrowser::show, "show")
		.func(&WebBrowser::hide, "hide")
		.func(&WebBrowser::close, "close")
		.func(&WebBrowser::url, "url")
		.func(&WebBrowser::url, "url")
		.func(&WebBrowser::setOnUrlChangedCallback, "setOnUrlChangedCallback")
		.func(&WebBrowser::setOnLoadFinishedCallback, "setOnLoadFinishedCallback")
		.func(&WebBrowser::setOnNavigateErrorCallback, "setOnNavigateErrorCallback");
}

}