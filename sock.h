#pragma once

#ifdef __linux__
#include <arpa/inet.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#define ERROR_SOCKET_GENERIC -1
#define ERROR_SOCKET_OK 0

#define ERROR_SOCKET_DESCRIPTOR 1
#define ERROR_SOCKET_OPTIONS 2
#define ERROR_SOCKET_BIND 3
#define ERROR_SOCKET_LISTEN 4
#define ERROR_SOCKET_ACCEPT 5

#define ERROR_SOCKET_CREATION 10
#define ERROR_SOCKET_ADDRESS 11
#define ERROR_SOCKET_FAILED 12

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif

namespace sock {

struct sock_data {
#ifdef __linux__
    int sock;
#endif
#ifdef _WIN32
    SOCKET ConnectionSocket;
#endif
};

class socket_entry {
   private:
    bool endsWith(std::string str, std::string substr);

   protected:
    bool strict_error_checking;

   public:
    socket_entry(bool sec = false);

    std::string read_text(struct sock_data data, int buf_size);
    std::string read_until(struct sock_data data, char c, bool include = false);
    std::string read_until(struct sock_data data, std::string s,
                           bool include = false);
    std::string read_line(struct sock_data data);

    void write_text(struct sock_data data, std::string text);
    void write_line(struct sock_data data, std::string line);

    void close_socket(struct sock_data data);
};

class server : public socket_entry {
   private:
#ifdef __linux__
    int server_fd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
#endif
#ifdef _WIN32
    WSADATA wsaData;
    int iResult;
    SOCKET ListenSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL;
    struct addrinfo hints;
#endif

   public:
    server(bool sec = false);

    int initialize(std::string port);
    void listen_socket(
        std::function<void(struct sock_data data)> callback,
        std::function<void(int which)> onerror = [](int w) {});

    void close_socket();
};

class client : public socket_entry {
   private:
    struct sock_data d = {
#ifdef __linux__
        18
#endif
#ifdef _WIN32
        INVALID_SOCKET
#endif
    };
#ifdef __linux__
    int valread, client_fd;
    struct sockaddr_in serv_addr;
#endif
#ifdef _WIN32
    WSADATA wsaData;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int iResult;
#endif

   public:
    int initialize(std::string address, std::string port);
    int connect_socket();

    std::string read_text(int buf_size);
    std::string read_until(char c, bool include = false);
    std::string read_until(std::string s, bool include = false);
    std::string read_line();

    void write_text(std::string text);
    void write_line(std::string line);

    void close_socket();
};

}  // namespace sock