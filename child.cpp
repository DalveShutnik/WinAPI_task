#include <winsock2.h>
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

DWORD recv(DWORD& err, SOCKET rd, char* b, DWORD k)
{
  DWORD r = 0;
  while (r < k) {
    int h = ::recv(rd, b + r, k - r, 0);
    if (h == SOCKET_ERROR) {
      err = WSAGetLastError();
      break;
    }
    if (h == 0) break;
    r += h;
  }
  return r;
}

int main(int argc, char** argv) {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
    std::cerr << "WSAStartup failed" << std::endl;
    return 1;
  }
  SOCKET rd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(static_cast<u_short>(std::stoi(argv[1])));
  if (rd == INVALID_SOCKET ||
      connect(rd, (sockaddr*)&addr, sizeof(addr))) {
    std::cerr << WSAGetLastError() << std::endl;
    closesocket(rd);
    WSACleanup();
    return 1;
  }

  DWORD err = 0, k = 0;
  if (recv(err, rd, (char*)&k, sizeof(k)) != sizeof(k)) {
    std::cerr << err << '\n';
    closesocket(rd);
    WSACleanup();
    return 1;
  }
  std::string msg(k, '\0');
  if (recv(err, rd, msg.data(), k) != k) {
    std::cerr << err << '\n';
    closesocket(rd);
    WSACleanup();
    return 1;
  }
  closesocket(rd);
  WSACleanup();
  printf("%s\n", msg.c_str());
}
