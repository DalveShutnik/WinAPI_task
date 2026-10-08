// parent.cpp
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

const u_short PORT = 50000;

DWORD send(DWORD& err, SOCKET wr, const char* b, DWORD k)
{
  DWORD r = 0;
  while (r < k) {
    int h = ::send(wr, b + r, k - r, 0);
    if (h == SOCKET_ERROR) {
      err = WSAGetLastError();
      break;
    }
    r += h;
  }
  return r;
}

int main(int argc, char** argv) {
  std::string msg;
  std::getline(std::cin, msg);

  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
    std::cerr << "WSAStartup failed" << std::endl;
    return 1;
  }
  SOCKET srv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(PORT);
  if (srv == INVALID_SOCKET ||
      bind(srv, (sockaddr*)&addr, sizeof(addr)) ||
      listen(srv, 1)) {
    std::cerr << WSAGetLastError() << std::endl;
    closesocket(srv);
    WSACleanup();
    return 1;
  }

  PROCESS_INFORMATION pi = {};
  STARTUPINFOA si = { .cb = sizeof(si) };
  std::string cmd = std::string(argv[1]) + ' ' + std::to_string(PORT);
  if (!CreateProcessA(argv[1], cmd.data(), NULL, NULL, FALSE,
                      NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
    std::cerr << GetLastError() << std::endl;
    closesocket(srv);
    WSACleanup();
    return 1;
  }

  SOCKET wr = accept(srv, NULL, NULL);
  closesocket(srv);
  if (wr == INVALID_SOCKET) {
    std::cerr << WSAGetLastError() << std::endl;
    WSACleanup();
    return 1;
  }
  DWORD err = 0, k = static_cast<DWORD>(msg.size());
  if (send(err, wr, (const char*)&k, sizeof(k)) != sizeof(k) ||
      send(err, wr, msg.data(), k) != k) {
    std::cerr << err << '\n';
    closesocket(wr);
    WSACleanup();
    return 1;
  }
  closesocket(wr);

  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
  WSACleanup();
}
