#ifndef EX__3_RESPONSE_H
#define EX__3_RESPONSE_H

#include <string>
#include <vector>
#include <sstream>

class Response {

public:
     class HeaderItem {
     private:
     public:
          HeaderItem();
          HeaderItem(std::string i_name, std::string i_value);

          std::string& getName();
          std::string& getValue();

     private:
          std::string name;
          std::string value;
     };

private:
     int versionMajor;
     int versionMinor;
     std::vector<Response::HeaderItem> headers;
     std::string content;
     bool keepAlive;
     unsigned int statusCode;
     std::string status;

public:
     Response() : versionMajor(0), versionMinor(0), keepAlive(false), statusCode(0) {}

     void setStatusCode(unsigned int statusCode);
     void setVersionMajor(int versionMajor);
     void setVersionMinor(int versionMinor);
     void setContent(const std::string& content);
     void setKeepAlive(bool keepAlive);
     void setStatus(const std::string& status);

     int getVersionMajor() const;
     int getVersionMinor() const;
     std::vector<HeaderItem>& getHeaders();
     std::string& getContent();
     bool isKeepAlive() const;
     unsigned int getStatusCode() const;
     const std::string& getStatus() const;

     std::string inspect();
     void addHeader(Response::HeaderItem input);
};


#endif EX__3_RESPONSE_H

