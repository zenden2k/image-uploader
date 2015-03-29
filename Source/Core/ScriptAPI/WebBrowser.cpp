#include "WebBrowser.h"
//using namespace ScriptAPI;


#ifdef _WIN32
#include "WebBrowserPrivate_win.h"
#else
// Not implemented
#endif


using namespace ScriptAPI;
DECLARE_INSTANCE_TYPE(CWebBrowser);

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

bool CWebBrowser::openModal()
{
	return d_->openModal();
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
		.func(&CWebBrowser::openModal, "openModal")
		.func(&CWebBrowser::setTitle, "setTitle")
		.func(&CWebBrowser::exec, "exec")
		.func(&CWebBrowser::show, "show")
		.func(&CWebBrowser::hide, "hide")
		.func(&CWebBrowser::close, "close")
		.func(&CWebBrowser::url, "url")
		.func(&CWebBrowser::title, "title")
		.func(&CWebBrowser::setOnUrlChangedCallback, "setOnUrlChangedCallback")
		.func(&CWebBrowser::setOnLoadFinishedCallback, "setOnLoadFinishedCallback")
		.func(&CWebBrowser::setOnNavigateErrorCallback, "setOnNavigateErrorCallback");
}

}