#ifndef EX__3_REQUEST_H
#define EX__3_REQUEST_H

#include <string>
#include <vector>
#include <sstream>
#include <map>

class Request {
public:
     class HeaderItem {
     public:
          std::string& getName();

          std::string& getValue();

     private:
          std::string name;
          std::string value;
     };

private:

     std::string method;
     std::string uri;
     int versionMajor;
     int versionMinor;
     std::string content;
     bool keepAlive;
     std::vector<Request::HeaderItem> headers;

public:
     Request() : versionMajor(0), versionMinor(0), keepAlive(false) {}

     std::string inspect();

     std::vector<HeaderItem>& getHeaders();

     std::string& getMethod();

     std::string& getUri();

     int getVersionMajor() const;

     int getVersionMinor() const;

     std::string& getContent();

     bool isKeepAlive() const;

     void setKeepAlive(bool keepAlive);

     void setVersionMajor(int versionMajor);

     void setVersionMinor(int versionMinor);

};

#endif EX__3_REQUEST_H