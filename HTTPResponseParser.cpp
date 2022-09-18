#include "HTTPResponseParser.h"

HttpResponseParser::ParseResult HttpResponseParser::parse(Response& resp, char* begin, char* end) {
     return consume(resp, begin, end);
}

bool HttpResponseParser::checkIfConnection(Response::HeaderItem& item) {
     return _stricmp(item.getName().c_str(), "Connection") == 0;  //_stricmp instead of strcasecmp
}

HttpResponseParser::ParseResult HttpResponseParser::consume(Response& resp, const char* begin, const char* end) {
     while (begin != end) {
          char input = *begin++;

          switch (state) {
          case ResponseStatusStart:
               if (input != 'H') {
                    return ParsingError;
               }
               else {
                    state = ResponseHttpVersion_ht;
               }
               break;
          case ResponseHttpVersion_ht:
               if (input == 'T') {
                    state = ResponseHttpVersion_htt;
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_htt:
               if (input == 'T') {
                    state = ResponseHttpVersion_http;
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_http:
               if (input == 'P') {
                    state = ResponseHttpVersion_slash;
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_slash:
               if (input == '/') {
                    resp.setVersionMajor(0);
                    resp.setVersionMinor(0);
                    state = ResponseHttpVersion_majorStart;
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_majorStart:
               if (isDigit(input)) {
                    resp.setVersionMajor(input - '0');
                    state = ResponseHttpVersion_major;
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_major:
               if (input == '.') {
                    state = ResponseHttpVersion_minorStart;
               }
               else if (isDigit(input)) {
                    resp.setVersionMajor(resp.getVersionMajor() * 10 + input - '0');
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_minorStart:
               if (isDigit(input)) {
                    resp.setVersionMinor(input - '0');
                    state = ResponseHttpVersion_minor;
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_minor:
               if (input == ' ') {
                    state = ResponseHttpVersion_statusCodeStart;
                    resp.setStatusCode(0);
               }
               else if (isDigit(input)) {
                    resp.setVersionMinor(resp.getVersionMinor() * 10 + input - '0');
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_statusCodeStart:
               if (isDigit(input)) {
                    resp.setStatusCode(input - '0');
                    state = ResponseHttpVersion_statusCode;
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_statusCode:
               if (isDigit(input)) {
                    resp.setStatusCode(resp.getStatusCode() * 10 + input - '0');
               }
               else {
                    if (resp.getStatusCode() < 100 || resp.getStatusCode() > 999) {
                         return ParsingError;
                    }
                    else if (input == ' ') {
                         state = ResponseHttpVersion_statusTextStart;
                    }
                    else {
                         return ParsingError;
                    }
               }
               break;
          case ResponseHttpVersion_statusTextStart:
               if (isChar(input)) {
                    resp.setStatus(resp.getStatus() + input);
                    state = ResponseHttpVersion_statusText;
               }
               else {
                    return ParsingError;
               }
               break;
          case ResponseHttpVersion_statusText:
               if (input == '\r') {
                    state = ResponseHttpVersion_newLine;
               }
               else if (isChar(input)) {
                    resp.setStatus(resp.getStatus() + input);
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
               else if (!resp.getHeaders().empty() && (input == ' ' || input == '\t')) {
                    state = HeaderLws;
               }
               else if (!isChar(input) || isControl(input) || isSpecial(input)) {
                    return ParsingError;
               }
               else {
                    resp.getHeaders().push_back(Response::HeaderItem());
                    resp.getHeaders().back().getName().reserve(16);
                    resp.getHeaders().back().getValue().reserve(16);
                    resp.getHeaders().back().getName().push_back(input);
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
                    resp.getHeaders().back().getValue().push_back(input);
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
                    resp.getHeaders().back().getName().push_back(input);
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
                    Response::HeaderItem& h = resp.getHeaders().back();

                    if (_stricmp(h.getName().c_str(), "Content-Length") == 0) {  //_stricmp instead of strcasecmp
                         contentSize = atoi(h.getValue().c_str());
                         resp.getContent().reserve(contentSize);
                    }
                    else if (_stricmp(h.getName().c_str(), "Transfer-Encoding") ==
                         0) {    //_stricmp instead of strcasecmp
                         if (_stricmp(h.getValue().c_str(), "chunked") == 0)  //_stricmp instead of strcasecmp
                              chunked = true;
                    }
                    state = ExpectingNewline_2;
               }
               else if (isControl(input)) {
                    return ParsingError;
               }
               else {
                    resp.getHeaders().back().getValue().push_back(input);
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
               std::vector<Response::HeaderItem>::iterator it = std::find_if(resp.getHeaders().begin(),
                    resp.getHeaders().end(),
                    checkIfConnection);

               if (it != resp.getHeaders().end()) {
                    if (_stricmp(it->getValue().c_str(), "Keep-Alive") == 0) {   //_stricmp instead of strcasecmp
                         resp.setKeepAlive(true);
                    }
                    else  // == Close
                    {
                         resp.setKeepAlive(false);
                    }
               }
               else {
                    if (resp.getVersionMajor() > 1 || (resp.getVersionMajor() == 1 && resp.getVersionMinor() == 1))
                         resp.setKeepAlive(true);
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
               resp.getContent().push_back(input);

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
                    resp.getContent().reserve(resp.getContent().size() + chunkSize);

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
               resp.getContent().push_back(input);

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
