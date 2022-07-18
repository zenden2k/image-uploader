#ifndef IU_CORE_SCRIPTAPI_WEBSERVER_H
#define IU_CORE_SCRIPTAPI_WEBSERVER_H

#pragma once 

#include <memory>
#include <string>
#include "Core/Scripting/Squirrelnc.h"

namespace ScriptAPI {
		
class WebServerPrivate;

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
