#ifndef IU_CORE_WEBSERVER_H
#define IU_CORE_WEBSERVER_H

#include "Core/Utils/CoreTypes.h"

class WebServerPrivate;

class WebServer {
public:
    WebServer();
    ~WebServer();
    bool start();
    WebServerPrivate* d_ptr;
    Q_DECLARE_PRIVATE(WebServer);
};
#endif