#include "SocketState.h"

int SocketState::socketsCount = 0;

void SocketState::removeSocket() {
     receiving = EMPTY;
     sending = EMPTY;
     SocketState::socketsCount--;
}

void SocketState::acceptConnection(SocketState*& sockets) {
     SOCKET identifier = this->id;
     struct sockaddr_in from;        // Address of sending partner
     int fromLen = sizeof(from);

     SOCKET msgSocket = accept(identifier, (struct sockaddr*)&from, &fromLen);
     if (INVALID_SOCKET == msgSocket) {
          std::cout << "Web Server: Error at accept(): " << WSAGetLastError() << std::endl;
          return;
     }
     std::cout << "Web Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected."
          << std::endl;

     //
     // Set the socket to be in non-blocking mode.
     //
     unsigned long flag = 1;
     if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0) {
          std::cout << "Web Server: Error at ioctlsocket(): " << WSAGetLastError() << std::endl;
     }

     if (!addSocket(sockets, msgSocket, RECEIVE)) {
          std::cout << "\t\tToo many connections, dropped!\n";
          closesocket(id);
     }
     return;
}

void SocketState::receiveMessage() {
     SOCKET msgSocket = id;

     int length = len;
     int bytesRecv = recv(msgSocket, &buffer[length], sizeof(buffer) - length, 0);

     if (SOCKET_ERROR == bytesRecv) {
          std::cout << "Web Server: Error at receiving(): " << WSAGetLastError() << std::endl;
          closesocket(msgSocket);
          removeSocket();
          return;
     }
     if (bytesRecv == 0) {
          closesocket(msgSocket);
          removeSocket();
          return;
     }
     else {
          buffer[length + bytesRecv] = '\0'; //add the null-terminating to make it a string
          len += bytesRecv;

          HttpRequestParser::ParseResult parseRes = reqParser.parse(request, buffer, buffer + strlen(buffer));

          if (parseRes == HttpRequestParser::ParsingCompleted) {
               sending = SEND;
               memcpy(buffer, &buffer[bytesRecv], len - bytesRecv);
               len -= bytesRecv;
          }
          else {
               std::cerr << "Parsing failed" << std::endl;
          }
     }
}

void SocketState::sendMessage() {
     int bytesSent = 0;
     char sendBuff[255];
     SOCKET msgSocket = id;

     generateResponse();
     strcpy(sendBuff, response.inspect().c_str());
     bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
     if (SOCKET_ERROR == bytesSent) {
          std::cout << "Web Server: Error at sending(): " << WSAGetLastError() << std::endl;
          return;
     }

     sending = IDLE;

}

SOCKET SocketState::getId() const {
     return id;
}

void SocketState::setId(SOCKET id) {
     SocketState::id = id;
}

int SocketState::getRecv() const {
     return receiving;
}

void SocketState::setRecv(int recv) {
     SocketState::receiving = recv;
}

int SocketState::getSend() const {
     return sending;
}

void SocketState::setSend(int send) {
     SocketState::sending = send;
}

int SocketState::getSendSubType() const {
     return sendSubType;
}

void SocketState::setSendSubType(int sendSubType) {
     SocketState::sendSubType = sendSubType;
}

const char* SocketState::getBuffer() const {
     return buffer;
}

int SocketState::getLen() const {
     return len;
}

void SocketState::setLen(int len) {
     SocketState::len = len;
}

const std::string& SocketState::getUrl() const {
     return URL;
}

void SocketState::setUrl(const std::string& url) {
     URL = url;
}

const Request& SocketState::getRequest() const {
     return request;
}

void SocketState::setRequest(const Request& request) {
     SocketState::request = request;
}

const Response& SocketState::getResponse() const {
     return response;
}

void SocketState::setResponse(const Response& response) {
     SocketState::response = response;
}

bool SocketState::addSocket(SocketState*& sockets, SOCKET id, int what) {
     for (int i = 0; i < MAX_SOCKETS; i++) {
          if (sockets[i].getRecv() == EMPTY) {
               sockets[i].setId(id);
               sockets[i].setRecv(what);
               sockets[i].setSend(IDLE);
               sockets[i].setLen(0);
               SocketState::socketsCount++;
               return (true);
          }
     }
     return (false);
}

void SocketState::generateResponse() {
     response.setVersionMajor(1);
     response.setVersionMinor(1);
     if (strcmp(request.getMethod().c_str(), "GET") == 0) {
          generateGet();
     }
     else if (strcmp(request.getMethod().c_str(), "HEAD") == 0) {
          generateHead();
     }
     else if (strcmp(request.getMethod().c_str(), "POST") == 0) {
          generatePost();
     }
     else if (strcmp(request.getMethod().c_str(), "PUT") == 0) {
          generatePut();
     }
     else if (strcmp(request.getMethod().c_str(), "DELETE") == 0) {
          generateDelete();
     }
     else if (strcmp(request.getMethod().c_str(), "OPTIONS") == 0) {
          generateOptions();
     }
     else if (strcmp(request.getMethod().c_str(), "TRACE") == 0) {
          generateTrace();
     }
     else {
          generateBadRequest();
     }
}

int SocketState::exists(const char* fname) {
     FILE* file;
     if ((file = fopen(fname, "r"))) {
          fclose(file);
          return 1;
     }
     return 0;
}

std::string SocketState::formatUri(std::string path) {
     size_t found = path.find("C:");
     std::string res = path.substr(found);
     size_t foundLang = res.find("?lang");
     if (foundLang != std::string::npos) {
          std::string lang = res.substr(foundLang + 6);
          size_t foundName = res.find("index");
          res = res.substr(0, foundName + 5);
          res = res + "-" + lang + ".html";
     }
     return res;

}

void SocketState::generateGet() {
     char* fileName;
     fileName = &(request.getUri()[1]);
     strcpy(fileName, formatUri(request.getUri()).c_str());

     if (exists(fileName) == 1) {
          std::ifstream myFile(fileName);
          if (myFile.is_open()) {
               std::string body;
               std::string line;
               while (getline(myFile, line)) {
                    body += line;
               }
               myFile.close();
               response.setContent(body);
               response.setStatusCode(200);
               response.setStatus("OK");
               std::stringstream value;
               value << body.length();
               Response::HeaderItem contentLen("Content-Length", value.str());
               response.addHeader(contentLen);
               Response::HeaderItem contentType("Content-Type", "text/html");
               response.addHeader(contentType);
          }
     }
     else {
          generateFileNotFound();
     }
}

void SocketState::generateHead() {
     char* fileName;
     fileName = &(request.getUri()[1]);
     strcpy(fileName, formatUri(request.getUri()).c_str());

     if (exists(fileName) == 1) {
          std::ifstream myFile(fileName);
          if (myFile.is_open()) {
               std::string body;
               std::string line;
               while (getline(myFile, line)) {
                    body += line;
               }
               myFile.close();
               response.setStatusCode(200);
               response.setStatus("OK");
               std::stringstream value;
               value << body.length();
               Response::HeaderItem contentLen("Content-Length", value.str());
               response.addHeader(contentLen);
               Response::HeaderItem contentType("Content-Type", "text/html");
               response.addHeader(contentType);
          }
     }
     else {
          generateFileNotFound();
     }
}

void SocketState::generatePost() {
     std::cout << request.getContent() << std::endl;
     response.setStatusCode(200);
     response.setStatus("OK");
     Response::HeaderItem contentLen("Content-Length", "0");
     response.addHeader(contentLen);
     Response::HeaderItem contentType("Content-Type", "text/html");
     response.addHeader(contentType);
     response.setContent("");
}

void SocketState::generatePut() {
     char* fileName;
     fileName = &(request.getUri()[1]);
     strcpy(fileName, formatUri(request.getUri()).c_str());

     if (exists(fileName) == 1) {
          std::ofstream ofs;
          ofs.open(fileName, std::ofstream::out | std::ofstream::trunc);
          ofs.close();
          std::fstream myFile(fileName);
          if (myFile.is_open()) {
               myFile.clear();
               myFile << request.getContent();
               myFile.close();
               response.setStatusCode(200);
               response.setStatus("OK");
               Response::HeaderItem contentLen("Content-Length", "0");
               response.addHeader(contentLen);
               Response::HeaderItem contentType("Content-Type", "text/html");
               response.addHeader(contentType);
          }
     }
     else {
          generateFileNotFound();
     }
}

void SocketState::generateDelete() {
     char* fileName;
     fileName = &(request.getUri()[1]);
     strcpy(fileName, formatUri(request.getUri()).c_str());

     if (exists(fileName) == 1) {
          if (remove(fileName) == 0) {
               std::string body = "The file has been successfully deleted";
               std::stringstream value;
               value << body.length();
               response.setStatusCode(200);
               response.setStatus("OK");
               Response::HeaderItem contentLen("Content-Length", value.str());
               response.addHeader(contentLen);
               Response::HeaderItem contentType("Content-Type", "text/html");
               response.setContent(body);
          }
     }
     else {
          generateFileNotFound();
     }
}

void SocketState::generateOptions() {
     response.setStatusCode(200);
     response.setStatus("OK");
     Response::HeaderItem contentLen("Content-Length", "0");
     response.addHeader(contentLen);
     Response::HeaderItem contentType("Content-Type", "text/html");
     response.addHeader(contentType);
     Response::HeaderItem allow("Allow", "GET, HEAD, POST, PUT, DELETE, OPTIONS, TRACE");
     response.addHeader(allow);
}

void SocketState::generateTrace() {
     response.setStatusCode(200);
     response.setStatus("OK");
     response.setContent(request.getContent());
     for (Request::HeaderItem item : request.getHeaders()) {
          Response::HeaderItem itemToAdd(item.getName(), item.getValue());
          response.addHeader(itemToAdd);
     }
}

void SocketState::generateFileNotFound() {
     response.setStatusCode(404);
     response.setStatus("File not found");
     std::string fail = "The file you were looking for couldn't be found";
     response.setContent(fail);
     std::stringstream value;
     value << fail.length();
     Response::HeaderItem contentLen("Content-Length", value.str());
     response.addHeader(contentLen);
     Response::HeaderItem contentType("Content-Type", "text/html");
     response.addHeader(contentType);
}

void SocketState::generateBadRequest() {
     response.setStatusCode(400);
     response.setStatus("Bad Request");
     std::string fail = "The server did not understand the request";
     response.setContent(fail);
     std::stringstream value;
     value << fail.length();
     Response::HeaderItem contentLen("Content-Length", value.str());
     response.addHeader(contentLen);
     Response::HeaderItem contentType("Content-Type", "text/html");
     response.addHeader(contentType);
}

