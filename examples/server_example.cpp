#include <iostream>

#include "../sock.h"

int main() {
    sock::server c;
    c.initialize("12345");
    c.listen_socket([&](struct sock::sock_data d) {
        c.write_line(d, "Hi client, I am a server.");
        c.read_line(d);
        c.write_line(d, "Oh no, you uncovered me!");
    }, [&](int which) {
        std::cout << "There was an error: " << which << std::endl;
    });
    c.close_socket();
}