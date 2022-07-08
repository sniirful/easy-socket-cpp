#include <iostream>

#include "../sock.h"

int main() {
    sock::client c;
    c.initialize("127.0.0.1", "12345");
    c.connect_socket();
    
    std::string s = c.read_line();
    std::cout << "s: " << s << std::endl;

    std::string sending = "I know that you said " + s;
    c.write_line(sending);
    std::cout << "c: " << sending << std::endl;

    std::string last = c.read_line();
    std::cout << "s: " << last << std::endl;

    c.close_socket();
}