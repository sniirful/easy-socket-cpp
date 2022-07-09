#include "sock.h"

//
// socket_entry
//

// private

bool sock::socket_entry::endsWith(std::string str, std::string substr) {
    for (int i = 0; i < substr.length(); i++) {
        if (str[str.length() - (i + 1)] != substr[substr.length() - (i + 1)])
            return false;
    }
    return true;
}

// public

sock::socket_entry::socket_entry(bool sec) {
    this->strict_error_checking = sec;
}

std::string sock::socket_entry::read_text(struct sock_data data, int buf_size) {
    std::string res = "";
    char* buffer = new char[buf_size];
    int read_bytes;
#ifdef __linux__
    read_bytes = read(data.sock, buffer, buf_size);
#endif
#ifdef _WIN32
    read_bytes = recv(data.ConnectionSocket, buffer, buf_size, 0);
#endif
    if (read_bytes < 0 && strict_error_checking) {
#ifdef __linux__
        throw errno;
#endif
#ifdef _WIN32
        throw WSAGetLastError();
#endif
    }
    for (int i = 0; i < read_bytes; i++) {
        res += buffer[i];
    }
    return res;
}
std::string sock::socket_entry::read_until(struct sock_data data, char c,
                                           bool include) {
    std::string res = "";
    for (;;) {
        std::string new_char = this->read_text(data, 1);
        if (new_char.length() <= 0) break;
        if (new_char[0] == c) {
            if (include) res += new_char;
            break;
        }
        res += new_char;
    }
    return res;
}
std::string sock::socket_entry::read_until(struct sock_data data, std::string s,
                                           bool include) {
    char last_c = s[s.length() - 1];
    std::string res = "";
    for (;;) {
        std::string temp = this->read_until(data, last_c, true);
        if (temp.length() <= 0) break;
        if (this->endsWith(temp, s)) {
            int len = temp.length();
            if (!include) len -= s.length();
            res += temp.substr(0, len);
            break;
        }
        res += temp;
    }
    return res;
}
std::string sock::socket_entry::read_line(struct sock_data data) {
    return this->read_until(data, '\n');
}

void sock::socket_entry::write_text(struct sock_data data, std::string text) {
    int written_bytes;
#ifdef __linux
    written_bytes = send(data.sock, text.c_str(), text.length(), 0);
#endif
#ifdef _WIN32
    written_bytes = send(data.ConnectionSocket, text.c_str(), text.length(), 0);
#endif
    if (written_bytes < 0 && strict_error_checking) {
#ifdef __linux__
        throw errno;
#endif
#ifdef _WIN32
        throw WSAGetLastError();
#endif
    }
}
void sock::socket_entry::write_line(struct sock_data data, std::string line) {
    this->write_text(data, line += '\n');
}

void sock::socket_entry::close_socket(struct sock_data data) {
#ifdef __linux__
    close(data.sock);
#endif
#ifdef _WIN32
    closesocket(data.ConnectionSocket);
    WSACleanup();
#endif
}

//
// server
//

// public

sock::server::server(bool sec) { strict_error_checking = sec; }

int sock::server::initialize(std::string port) {
#ifdef __linux__
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        return ERROR_SOCKET_DESCRIPTOR;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        return ERROR_SOCKET_OPTIONS;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(port.c_str()));

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        return ERROR_SOCKET_BIND;
    }
#endif
#ifdef _WIN32
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return ERROR_SOCKET_GENERIC;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
    if (iResult != 0) {
        return ERROR_SOCKET_GENERIC;
    }
    ListenSocket =
        socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        return ERROR_SOCKET_OPTIONS;
    }
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        return ERROR_SOCKET_BIND;
    }

    freeaddrinfo(result);
#endif
    return ERROR_SOCKET_OK;
}
void sock::server::listen_socket(
    std::function<void(struct sock_data data)> callback,
    std::function<void(int which)> onerror) {
#ifdef __linux__
    if (listen(server_fd, 3) < 0) {
        return onerror(ERROR_SOCKET_LISTEN);
    }
#endif
#ifdef _WIN32
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        return onerror(ERROR_SOCKET_LISTEN);
    }
#endif

    std::vector<std::thread> threads;
    struct sock_data d;
    for (;;) {
#ifdef __linux__
        if ((d.sock = accept(server_fd, (struct sockaddr*)&address,
                             (socklen_t*)&addrlen)) < 0) {
            return onerror(ERROR_SOCKET_ACCEPT);
        }
#endif
#ifdef _WIN32
        d.ConnectionSocket = accept(ListenSocket, NULL, NULL);
        if (d.ConnectionSocket == INVALID_SOCKET) {
            return onerror(ERROR_SOCKET_ACCEPT);
        }
#endif
        threads.push_back(std::thread([&]() {
            callback(d);
#ifdef __linux__
            close(d.sock);
#endif
#ifdef _WIN32
            closesocket(d.ConnectionSocket);
#endif
        }));
    }
    this->close_socket();
    for (std::thread& t : threads) t.join();
}

void sock::server::close_socket() {
#ifdef __linux__
    shutdown(server_fd, SHUT_RDWR);
#endif
#ifdef _WIN32
    closesocket(ListenSocket);
    WSACleanup();
#endif
}

//
// client
//

// public

int sock::client::initialize(std::string address, std::string port) {
#ifdef __linux__
    if ((d.sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return ERROR_SOCKET_CREATION;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port.c_str()));

    if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0) {
        return ERROR_SOCKET_ADDRESS;
    }
#endif
#ifdef _WIN32
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return ERROR_SOCKET_CREATION;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(address.c_str(), port.c_str(), &hints, &result);
    if (iResult != 0) {
        return ERROR_SOCKET_ADDRESS;
    }
#endif
    return ERROR_SOCKET_OK;
}
int sock::client::connect_socket() {
#ifdef __linux__
    if ((client_fd = connect(d.sock, (struct sockaddr*)&serv_addr,
                             sizeof(serv_addr))) < 0) {
        return ERROR_SOCKET_FAILED;
    }
#endif
#ifdef _WIN32
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        d.ConnectionSocket =
            socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (d.ConnectionSocket == INVALID_SOCKET) {
            return ERROR_SOCKET_FAILED;
        }

        iResult =
            connect(d.ConnectionSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            d.ConnectionSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (d.ConnectionSocket == INVALID_SOCKET) {
        return ERROR_SOCKET_FAILED;
    }
#endif
    return ERROR_SOCKET_OK;
}

std::string sock::client::read_text(int buf_size) {
    return socket_entry::read_text(d, buf_size);
}
std::string sock::client::read_until(char c, bool include) {
    return socket_entry::read_until(d, c, include);
}
std::string sock::client::read_until(std::string s, bool include) {
    return socket_entry::read_until(d, s, include);
}
std::string sock::client::read_line() { return socket_entry::read_line(d); }

void sock::client::write_text(std::string text) {
    socket_entry::write_text(d, text);
}
void sock::client::write_line(std::string line) {
    socket_entry::write_line(d, line);
}

void sock::client::close_socket() {
#ifdef __linux__
    socket_entry::close_socket({client_fd});
#endif
#ifdef _WIN32
    socket_entry::close_socket(d);
#endif
}
