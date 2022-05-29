#ifndef IU_CORE_SCRIPTAPI_WEBSERVER_H
#define IU_CORE_SCRIPTAPI_WEBSERVER_H

#pragma once 

#include "Core/Scripting/Squirrelnc.h"

namespace ScriptAPI {

class WebServerPrivateBase {
	public:
		virtual ~WebServerPrivateBase() {}
		virtual void stop() {}
};
		
class WebServerPrivate;

class WebServerRequest
{
	std::string method_, path_, queryString_, httpVersion_;
public:
	WebServerRequest();
	std::string method() const;
	std::string path() const;
	std::string queryString() const;
	std::string httpVersion() const;
};
class WebServer {
	public:
		WebServer();
		~WebServer();
		int bind(int port);
		void start();
		void resource(const std::string& path, const std::string& method, Sqrat::Function callBack, Sqrat::Object context);
		int port() const;
		void stop();
	protected:
		std::shared_ptr<WebServerPrivate> d_;
};

/* @cond PRIVATE */
void RegisterWebServerClass(Sqrat::SqratVM& vm);
/* @endcond */

}

#endif