#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <string.h>
#include "SocketState.h"
#include <fstream>
#include <sstream>

const int INITIALIZE_WINSOCK_ERROR = -1;
const int CREATING_SOCKET_ERROR = -2;
const int BIND_ERROR = -3;
const int LISTEN_ERROR = -4;
const int SELECT_ERROR = -5;
const int CONNECTION_ERROR = -6;
const int SEND_ERROR = -7;
const int GET_ERROR = -8;

int main() {

     SocketState* sockets = nullptr;
     sockets = new SocketState[MAX_SOCKETS];

     WSAData wsaData;

     if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
          std::cout << "Web Server: Error at WSAStartup()\n";
          return INITIALIZE_WINSOCK_ERROR;
     }

     SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

     if (INVALID_SOCKET == listenSocket) {
          std::cout << "Web Server: Error at socket(): " << WSAGetLastError() << std::endl;
          WSACleanup();
          return CREATING_SOCKET_ERROR;
     }

     sockaddr_in serverService;
     serverService.sin_family = AF_INET;
     serverService.sin_addr.s_addr = INADDR_ANY;
     serverService.sin_port = htons(8080);

     if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService))) {
          std::cout << "Web Server: Error at bind(): " << WSAGetLastError() << std::endl;
          closesocket(listenSocket);
          WSACleanup();
          return BIND_ERROR;
     }

     if (SOCKET_ERROR == listen(listenSocket, 5)) {
          std::cout << "Web Server: Error at listen(): " << WSAGetLastError() << std::endl;
          closesocket(listenSocket);
          WSACleanup();
          return LISTEN_ERROR;
     }
     SocketState::addSocket(sockets, listenSocket, LISTEN);

     while (true) {
          fd_set waitRecv;
          FD_ZERO(&waitRecv);
          for (int i = 0; i < MAX_SOCKETS; i++) {
               if ((sockets[i].getRecv() == LISTEN) || (sockets[i].getRecv() == RECEIVE))
                    FD_SET(sockets[i].getId(), &waitRecv);
          }

          fd_set waitSend;
          FD_ZERO(&waitSend);
          for (int i = 0; i < MAX_SOCKETS; i++) {
               if (sockets[i].getSend() == SEND)
                    FD_SET(sockets[i].getId(), &waitSend);
          }

          int nfd;
          nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
          if (nfd == SOCKET_ERROR) {
               std::cout << "Web Server: Error at select(): " << WSAGetLastError() << std::endl;
               WSACleanup();
               return SELECT_ERROR;
          }

          for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++) {
               if (FD_ISSET(sockets[i].getId(), &waitRecv)) {
                    nfd--;
                    switch (sockets[i].getRecv()) {
                    case LISTEN:
                         sockets[i].acceptConnection(sockets);
                         break;

                    case RECEIVE:
                         sockets[i].receiveMessage();
                         break;
                    }
               }
          }

          for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++) {
               if (FD_ISSET(sockets[i].getId(), &waitSend)) {
                    nfd--;
                    switch (sockets[i].getSend()) {
                    case SEND:
                         sockets[i].sendMessage();
                         break;
                    }
               }
          }
     }

     std::cout << "Time Server: Closing Connection.\n";
     closesocket(listenSocket);
     WSACleanup();
     return 0;
}