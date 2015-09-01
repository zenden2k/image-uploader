#include "WebServer.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define DOCUMENT_ROOT "."
#define PORT "8081"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"
#include "CivetServer.h"
bool exitNow = false;

class ExampleHandler : public CivetHandler {
public:
    bool handleGet(CivetServer *server, struct mg_connection *conn) {
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        mg_printf(conn, "<html><body>\r\n");
        mg_printf(conn, "<h2>Image Uploader Web API</h2>\r\n");
        mg_printf(conn, "</body></html>\r\n");
        return true;
    }
};

class WebServerUploadHandler : public CivetHandler {
public:

    bool handleGet(CivetServer* server, mg_connection* conn) override {
        /* Show HTML form. */
        /* See http://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.1 */
        static const char *html_form =
            "<html><body>Upload example."
            ""
            /* enctype="multipart/form-data" */
            "<form method=\"POST\" action=\"\" "
            "  enctype=\"multipart/form-data\">"
            "<input type=\"file\" name=\"file\" /> <br/>"
            "<input type=\"file\" name=\"file2\" /> <br/>"
            "<input type=\"text\" name=\"test\" value=\"bunny\" /> <br/>"
            "<input type=\"submit\" value=\"Upload\" />"
            "</form>"
            ""
            "</body></html>";

        mg_printf(conn, "HTTP/1.0 200 OK\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: text/html\r\n\r\n%s",
            (int)strlen(html_form), html_form);
        return true;
    }

    bool handlePost(CivetServer* server, mg_connection* conn) override {
        std::string res;
        server->getParam(conn, "test", res);
        mg_printf(conn, "%s", "HTTP/1.0 200 OK\r\n\r\n");
        mg_upload(conn, "d:/");
       
        return true;
    }
};
/*
class ExitHandler : public CivetHandler {
public:
    bool handleGet(CivetServer *server, struct mg_connection *conn) {
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
        mg_printf(conn, "Bye!\n");
        exitNow = true;
        return true;
    }
};

class AHandler : public CivetHandler {
private:
    bool handleAll(const char * method, CivetServer *server, struct mg_connection *conn) {
        std::string s = "";
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        mg_printf(conn, "<html><body>");
        mg_printf(conn, "<h2>This is the A handler for \"%s\" !</h2>", method);
        if (CivetServer::getParam(conn, "param", s)) {
            mg_printf(conn, "<p>param set to %s</p>", s.c_str());
        } else {
            mg_printf(conn, "<p>param not set</p>");
        }
        mg_printf(conn, "</body></html>\n");
        return true;
    }
public:
    bool handleGet(CivetServer *server, struct mg_connection *conn) {
        return handleAll("GET", server, conn);
    }
    bool handlePost(CivetServer *server, struct mg_connection *conn) {
        return handleAll("POST", server, conn);
    }
};

class ABHandler : public CivetHandler {
public:
    bool handleGet(CivetServer *server, struct mg_connection *conn) {
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        mg_printf(conn, "<html><body>");
        mg_printf(conn, "<h2>This is the AB handler!!!</h2>");
        mg_printf(conn, "</body></html>\n");
        return true;
    }
};

class FooHandler : public CivetHandler {
public:
    bool handleGet(CivetServer *server, struct mg_connection *conn) {
        /* Handler may access the request info using mg_get_request_info 
        const struct mg_request_info * req_info = mg_get_request_info(conn);

        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        mg_printf(conn, "<html><body>");
        mg_printf(conn, "<h2>Image Uploader HTTP API!!!</h2>");
        mg_printf(conn, "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>",
            req_info->request_method, req_info->uri, req_info->http_version);
        mg_printf(conn, "</body></html>\n");
        return true;
    }
};
*/
class WebServerPrivate {
    private:
        std::unique_ptr<CivetServer> server_;
        std::unique_ptr<ExampleHandler> exampleHandler_;
        std::unique_ptr<WebServerUploadHandler> uploadHandler_;
        int port_;
public:
    WebServerPrivate() {
        port_ = 8081;
    }
    bool start() {
        if (!server_) {
            try {
                char portBuffer[20];
                sprintf(portBuffer, "127.0.0.1:%d", port_);
                const char * options[] = {
                    "document_root", DOCUMENT_ROOT,
                    "num_threads", "3",
                    "listening_ports", portBuffer, 0
                };

                server_.reset(new CivetServer(options));

                exampleHandler_.reset(new ExampleHandler);
                server_->addHandler("/", exampleHandler_.get());
                uploadHandler_.reset(new WebServerUploadHandler);
                server_->addHandler("/api/UploadScreenshot", uploadHandler_.get());

                /*ExitHandler h_exit;
                server.addHandler(EXIT_URI, h_exit);

                AHandler h_a;
                server.addHandler("/a", h_a);

                ABHandler h_ab;
                server.addHandler("/a/b", h_ab);

                FooHandler h_foo;
                server.addHandler("**.foo$", h_foo);

                printf("Browse files at http://localhost:%s/\n", PORT);
                printf("Run example at http://localhost:%s%s\n", PORT, EXAMPLE_URI);
                printf("Exit at http://localhost:%s%s\n", PORT, EXIT_URI);

                while (!exitNow) {
                #ifdef _WIN32
                Sleep(1000);
                #else
                sleep(1);
                #endif
                }*/
            } catch (std::exception& ex) {
                LOG(ERROR) << ex.what();
            }
        }
        return true;
    }
};

WebServer::WebServer() : d_ptr(new WebServerPrivate) {
}

WebServer::~WebServer() {
    delete d_ptr;
}

bool WebServer::start() {
    Q_D(WebServer);
    return d->start();
}