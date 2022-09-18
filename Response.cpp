#include "Response.h"

std::string Response::inspect() {
     std::stringstream stream;
     stream << "HTTP/" << versionMajor << "." << versionMinor
          << " " << statusCode << " " << status << "\n";

     for (std::vector<Response::HeaderItem>::iterator it = headers.begin();
          it != headers.end(); ++it) {
          stream << it->getName() << ": " << it->getValue() << "\n";
     }
     stream << "\r\n";
     std::string data(content.begin(), content.end());
     stream << data << "\n";
     return stream.str();
}

std::string& Response::HeaderItem::getName() {
     return name;
}

std::string& Response::HeaderItem::getValue() {
     return value;
}

Response::HeaderItem::HeaderItem() {

}

Response::HeaderItem::HeaderItem(std::string i_name, std::string i_value) {
     name = i_name;
     value = i_value;
}

int Response::getVersionMajor() const {
     return versionMajor;
}

int Response::getVersionMinor() const {
     return versionMinor;
}

std::vector<Response::HeaderItem>& Response::getHeaders() {
     return headers;
}

std::string& Response::getContent() {
     return content;
}

bool Response::isKeepAlive() const {
     return keepAlive;
}

unsigned int Response::getStatusCode() const {
     return statusCode;
}

const std::string& Response::getStatus() const {
     return status;
}

void Response::setVersionMajor(int versionMajor) {
     Response::versionMajor = versionMajor;
}

void Response::setVersionMinor(int versionMinor) {
     Response::versionMinor = versionMinor;
}

void Response::setStatusCode(unsigned int statusCode) {
     Response::statusCode = statusCode;
}

void Response::setStatus(const std::string& status) {
     Response::status = status;
}

void Response::setKeepAlive(bool keepAlive) {
     Response::keepAlive = keepAlive;
}

void Response::setContent(const std::string& content) {
     Response::content = content;
}

void Response::addHeader(const Response::HeaderItem input)
{
     headers.push_back(input);
}
