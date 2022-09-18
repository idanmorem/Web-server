#include "HTTPRequestParser.h"

bool HttpRequestParser::checkIfConnection(Request::HeaderItem& item) {
     return _stricmp(item.getName().c_str(), "Connection") == 0;  //_stricmp instead of strcasecmp
}

HttpRequestParser::ParseResult HttpRequestParser::parse(Request& req, const char* begin, const char* end) {
     return consume(req, begin, end);
}

HttpRequestParser::ParseResult HttpRequestParser::consume(Request& req, const char* begin, const char* end) {
     while (begin != end) {
          char input = *begin++;

          switch (state) {
          case RequestMethodStart:
               if (!isChar(input) || isControl(input) || isSpecial(input)) {
                    return ParsingError;
               }
               else {
                    state = RequestMethod;
                    req.getMethod().push_back(input);
               }
               break;
          case RequestMethod:
               if (input == ' ') {
                    state = RequestUriStart;
               }
               else if (!isChar(input) || isControl(input) || isSpecial(input)) {
                    return ParsingError;
               }
               else {
                    req.getMethod().push_back(input);
               }
               break;
          case RequestUriStart:
               if (isControl(input)) {
                    return ParsingError;
               }
               else {
                    state = RequestUri;
                    req.getUri().push_back(input);
               }
               break;
          case RequestUri:
               if (input == ' ') {
                    state = RequestHttpVersion_h;
               }
               else if (input == '\r') {
                    req.setVersionMajor(0);
                    req.setVersionMinor(9);

                    return ParsingCompleted;
               }
               else if (isControl(input)) {
                    return ParsingError;
               }
               else {
                    req.getUri().push_back(input);
               }
               break;
          case RequestHttpVersion_h:
               if (input == 'H') {
                    state = RequestHttpVersion_ht;
               }
               else {
                    return ParsingError;
               }
               break;
          case RequestHttpVersion_ht:
               if (input == 'T') {
                    state = RequestHttpVersion_htt;
               }
               else {
                    return ParsingError;
               }
               break;
          case RequestHttpVersion_htt:
               if (input == 'T') {
                    state = RequestHttpVersion_http;
               }
               else {
                    return ParsingError;
               }
               break;
          case RequestHttpVersion_http:
               if (input == 'P') {
                    state = RequestHttpVersion_slash;
               }
               else {
                    return ParsingError;
               }
               break;
          case RequestHttpVersion_slash:
               if (input == '/') {
                    req.setVersionMajor(0);
                    req.setVersionMinor(0);
                    state = RequestHttpVersion_majorStart;
               }
               else {
                    return ParsingError;
               }
               break;
          case RequestHttpVersion_majorStart:
               if (isDigit(input)) {
                    req.setVersionMajor(input - '0');
                    state = RequestHttpVersion_major;
               }
               else {
                    return ParsingError;
               }
               break;
          case RequestHttpVersion_major:
               if (input == '.') {
                    state = RequestHttpVersion_minorStart;
               }
               else if (isDigit(input)) {
                    req.setVersionMajor(req.getVersionMajor() * 10 + input - '0');
               }
               else {
                    return ParsingError;
               }
               break;
          case RequestHttpVersion_minorStart:
               if (isDigit(input)) {
                    req.setVersionMajor(input - '0');
                    state = RequestHttpVersion_minor;
               }
               else {
                    return ParsingError;
               }
               break;
          case RequestHttpVersion_minor:
               if (input == '\r') {
                    state = ResponseHttpVersion_newLine;
               }
               else if (isDigit(input)) {
                    req.setVersionMinor(req.getVersionMinor() * 10 + input - '0');
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_newLine:
               if (input == '\n') {
                    state = HeaderLineStart;
               }
               else {
                    return ParsingError;
               }
               break;
          case HeaderLineStart:
               if (input == '\r') {
                    state = ExpectingNewline_3;
               }
               else if (!req.getHeaders().empty() && (input == ' ' || input == '\t')) {
                    state = HeaderLws;
               }
               else if (!isChar(input) || isControl(input) || isSpecial(input)) {
                    return ParsingError;
               }
               else {
                    req.getHeaders().push_back(Request::HeaderItem());
                    req.getHeaders().back().getName().reserve(16);
                    req.getHeaders().back().getValue().reserve(16);
                    req.getHeaders().back().getName().push_back(input);
                    state = HeaderName;
               }
               break;
          case HeaderLws:
               if (input == '\r') {
                    state = ExpectingNewline_2;
               }
               else if (input == ' ' || input == '\t') {
               }
               else if (isControl(input)) {
                    return ParsingError;
               }
               else {
                    state = HeaderValue;
                    req.getHeaders().back().getValue().push_back(input);
               }
               break;
          case HeaderName:
               if (input == ':') {
                    state = SpaceBeforeHeaderValue;
               }
               else if (!isChar(input) || isControl(input) || isSpecial(input)) {
                    return ParsingError;
               }
               else {
                    req.getHeaders().back().getName().push_back(input);
               }
               break;
          case SpaceBeforeHeaderValue:
               if (input == ' ') {
                    state = HeaderValue;
               }
               else {
                    return ParsingError;
               }
               break;
          case HeaderValue:
               if (input == '\r') {
                    if (req.getMethod() == "POST" || req.getMethod() == "PUT") {
                         Request::HeaderItem& h = req.getHeaders().back();

                         if (_stricmp(h.getName().c_str(), "Content-Length") == 0) {  //_stricmp instead of strcasecmp
                              contentSize = atoi(h.getValue().c_str());
                              req.getContent().reserve(contentSize);
                         }
                         else if (_stricmp(h.getName().c_str(), "Transfer-Encoding") ==
                              0) {    //_stricmp instead of strcasecmp
                              if (_stricmp(h.getValue().c_str(), "chunked") == 0)  //_stricmp instead of strcasecmp
                                   chunked = true;
                         }
                    }
                    state = ExpectingNewline_2;
               }
               else if (isControl(input)) {
                    return ParsingError;
               }
               else {
                    req.getHeaders().back().getValue().push_back(input);
               }
               break;
          case ExpectingNewline_2:
               if (input == '\n') {
                    state = HeaderLineStart;
               }
               else {
                    return ParsingError;
               }
               break;
          case ExpectingNewline_3: {
               std::vector<Request::HeaderItem>::iterator it = std::find_if(req.getHeaders().begin(),
                    req.getHeaders().end(),
                    checkIfConnection);

               if (it != req.getHeaders().end()) {
                    if (_stricmp(it->getValue().c_str(), "Keep-Alive") == 0) { //_stricmp instead of strcasecmp
                         req.setKeepAlive(true);
                    }
                    else  // == Close
                    {
                         req.setKeepAlive(false);
                    }
               }
               else {
                    if (req.getVersionMajor() > 1 || (req.getVersionMajor() == 1 && req.getVersionMinor() == 1))
                         req.setKeepAlive(true);
               }

               if (chunked) {
                    state = ChunkSize;
               }
               else if (contentSize == 0) {
                    if (input == '\n')
                         return ParsingCompleted;
                    else
                         return ParsingError;
               }
               else {
                    state = Post;
               }
               break;
          }
          case Post:
               --contentSize;
               req.getContent().push_back(input);

               if (contentSize == 0) {
                    return ParsingCompleted;
               }
               break;
          case ChunkSize:
               if (isalnum(input)) {
                    chunkSizeStr.push_back(input);
               }
               else if (input == ';') {
                    state = ChunkExtensionName;
               }
               else if (input == '\r') {
                    state = ChunkSizeNewLine;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkExtensionName:
               if (isalnum(input) || input == ' ') {
                    // skip
               }
               else if (input == '=') {
                    state = ChunkExtensionValue;
               }
               else if (input == '\r') {
                    state = ChunkSizeNewLine;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkExtensionValue:
               if (isalnum(input) || input == ' ') {
                    // skip
               }
               else if (input == '\r') {
                    state = ChunkSizeNewLine;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkSizeNewLine:
               if (input == '\n') {
                    chunkSize = strtol(chunkSizeStr.c_str(), NULL, 16);
                    chunkSizeStr.clear();
                    req.getContent().reserve(req.getContent().size() + chunkSize);

                    if (chunkSize == 0)
                         state = ChunkSizeNewLine_2;
                    else
                         state = ChunkData;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkSizeNewLine_2:
               if (input == '\r') {
                    state = ChunkSizeNewLine_3;
               }
               else if (isalpha(input)) {
                    state = ChunkTrailerName;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkSizeNewLine_3:
               if (input == '\n') {
                    return ParsingCompleted;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkTrailerName:
               if (isalnum(input)) {
                    // skip
               }
               else if (input == ':') {
                    state = ChunkTrailerValue;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkTrailerValue:
               if (isalnum(input) || input == ' ') {
                    // skip
               }
               else if (input == '\r') {
                    state = ChunkSizeNewLine;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkData:
               req.getContent().push_back(input);

               if (--chunkSize == 0) {
                    state = ChunkDataNewLine_1;
               }
               break;
          case ChunkDataNewLine_1:
               if (input == '\r') {
                    state = ChunkDataNewLine_2;
               }
               else {
                    return ParsingError;
               }
               break;
          case ChunkDataNewLine_2:
               if (input == '\n') {
                    state = ChunkSize;
               }
               else {
                    return ParsingError;
               }
               break;
          default:
               return ParsingError;
          }
     }

     return ParsingIncompleted;
}
