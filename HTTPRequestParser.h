#ifndef EX__3_HTTPREQUESTPARSER_H
#define EX__3_HTTPREQUESTPARSER_H

#include <algorithm>

#include <string.h>
#include <stdlib.h>

#include "request.h"

class HttpRequestParser {

public:

     HttpRequestParser()
          : state(RequestMethodStart), contentSize(0),
          chunkSize(0), chunked(false) {
     }

     enum ParseResult {
          ParsingCompleted,
          ParsingIncompleted,
          ParsingError
     };

     // The current state of the parser.
     enum State {
          RequestMethodStart,
          RequestMethod,
          RequestUriStart,
          RequestUri,
          RequestHttpVersion_h,
          RequestHttpVersion_ht,
          RequestHttpVersion_htt,
          RequestHttpVersion_http,
          RequestHttpVersion_slash,
          RequestHttpVersion_majorStart,
          RequestHttpVersion_major,
          RequestHttpVersion_minorStart,
          RequestHttpVersion_minor,

          ResponseStatusStart,
          ResponseHttpVersion_ht,
          ResponseHttpVersion_htt,
          ResponseHttpVersion_http,
          ResponseHttpVersion_slash,
          ResponseHttpVersion_majorStart,
          ResponseHttpVersion_major,
          ResponseHttpVersion_minorStart,
          ResponseHttpVersion_minor,
          ResponseHttpVersion_spaceAfterVersion,
          ResponseHttpVersion_statusCodeStart,
          ResponseHttpVersion_spaceAfterStatusCode,
          ResponseHttpVersion_statusTextStart,
          ResponseHttpVersion_newLine,

          HeaderLineStart,
          HeaderLws,
          HeaderName,
          SpaceBeforeHeaderValue,
          HeaderValue,
          ExpectingNewline_2,
          ExpectingNewline_3,

          Post,
          ChunkSize,
          ChunkExtensionName,
          ChunkExtensionValue,
          ChunkSizeNewLine,
          ChunkSizeNewLine_2,
          ChunkSizeNewLine_3,
          ChunkTrailerName,
          ChunkTrailerValue,

          ChunkDataNewLine_1,
          ChunkDataNewLine_2,
          ChunkData,
     } state;

     ParseResult parse(Request& req, const char* begin, const char* end);

private:

     size_t contentSize;
     std::string chunkSizeStr;
     size_t chunkSize;
     bool chunked;


     static bool checkIfConnection(Request::HeaderItem& item);

     HttpRequestParser::ParseResult consume(Request& req, const char* begin, const char* end);

     // Check if a byte is an HTTP character.
     inline bool isChar(int c) {
          return c >= 0 && c <= 127;
     }

     // Check if a byte is an HTTP control character.
     inline bool isControl(int c) {
          return (c >= 0 && c <= 31) || (c == 127);
     }

     // Check if a byte is defined as an HTTP special character.
     inline bool isSpecial(int c) {
          switch (c) {
          case '(':
          case ')':
          case '<':
          case '>':
          case '@':
          case ',':
          case ';':
          case ':':
          case '\\':
          case '"':
          case '/':
          case '[':
          case ']':
          case '?':
          case '=':
          case '{':
          case '}':
          case ' ':
          case '\t':
               return true;
          default:
               return false;
          }
     }

     // Check if a byte is a digit.
     inline bool isDigit(int c) {
          return c >= '0' && c <= '9';
     }

};

#endif // EX__3_HTTPREQUESTPARSER_H
