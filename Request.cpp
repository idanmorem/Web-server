#include "Request.h"

std::string Request::inspect() {
     std::stringstream stream;
     stream << method << " " << uri << " HTTP/"
          << versionMajor << "." << versionMinor << "\n";

     for (std::vector<Request::HeaderItem>::iterator it = headers.begin();
          it != headers.end(); ++it) {
          stream << it->getName() << ": " << it->getValue() << "\n";
     }

     std::string data(content.begin(), content.end());
     stream << data << "\n";
     stream << "+ keep-alive: " << keepAlive << "\n";;
     return stream.str();
}

std::string& Request::HeaderItem::getName() {
     return name;
}

std::string& Request::HeaderItem::getValue() {
     return value;
}

std::vector<Request::HeaderItem>& Request::getHeaders() {
     return headers;
}

std::string& Request::getMethod() {
     return method;
}

std::string& Request::getUri() {
     return uri;
}

int Request::getVersionMajor() const {
     return versionMajor;
}

int Request::getVersionMinor() const {
     return versionMinor;
}

std::string& Request::getContent() {
     return content;
}

bool Request::isKeepAlive() const {
     return keepAlive;
}

void Request::setVersionMajor(int versionMajor) {
     Request::versionMajor = versionMajor;
}

void Request::setVersionMinor(int versionMinor) {
     Request::versionMinor = versionMinor;
}

void Request::setKeepAlive(bool keepAlive) {
     Request::keepAlive = keepAlive;
}
