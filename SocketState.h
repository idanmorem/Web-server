#ifndef EX__3_SOCKETSTATE_H
#define EX__3_SOCKETSTATE_H
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include "Response.h"
#include "Request.h"
#include "HTTPRequestParser.h"
#include "HTTPResponseParser.h"
#include "URLParser.h"
#include <sys/stat.h>
#include <stdbool.h>

const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;

class SocketState {
private:
     SOCKET id = NULL;            // Socket handle
     int receiving = EMPTY;               // Receiving?
     int sending = EMPTY;            // Sending?
     int sendSubType = EMPTY;    // Sending sub-type
     char buffer[1024];
     int len = 0;
     std::string URL;
     Request request;
     HttpRequestParser reqParser;
     Response response;
     HttpResponseParser resParser;
     UrlParser urlParser;
     static int socketsCount;

public:
     SocketState() {}
     SOCKET getId() const;
     void setId(SOCKET id);
     int getRecv() const;
     void setRecv(int recv);
     int getSend() const;
     void setSend(int send);
     int getSendSubType() const;
     void setSendSubType(int sendSubType);
     const char* getBuffer() const;
     int getLen() const;
     void setLen(int len);
     void generateResponse();
     const std::string& getUrl() const;
     void setUrl(const std::string& url);
     const Request& getRequest() const;
     void setRequest(const Request& request);
     const Response& getResponse() const;
     void setResponse(const Response& response);
     void removeSocket();
     void acceptConnection(SocketState*& sockets);
     void receiveMessage();
     void sendMessage();
     static bool addSocket(SocketState*& sockets, SOCKET id, int what);
     int exists(const char* fname);
     std::string formatUri(std::string path);

     void generateGet();

     void generateHead();

     void generatePost();

     void generatePut();

     void generateDelete();

     void generateOptions();

     void generateTrace();

     void generateFileNotFound();

     void generateBadRequest();
};

#endif //EX__3_SOCKETSTATE_H