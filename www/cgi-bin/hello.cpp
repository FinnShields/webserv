#include <iostream>
#include <unistd.h>



std::string readFromFd(int fd) {
    std::string result;
    char buffer[1024];
    ssize_t size;
    while ((size = read(fd, buffer, sizeof(buffer))) > 0) {
        result.append(buffer, size);
    }
    if (size < 0) {
        throw std::runtime_error("readFromFd: read error occured!");
    }
    return result;
}

int main() {

    //std::string request_body = "dummy body"
    // Output the HTTP headers
    std::cout << "---- CGI got msg-------\n";
    std::string request_body = readFromFd(0);
    std::cout << request_body << "\n";
    std::cout << "---- END of msg -------\n\n";
    std::cout << "---- CGI answer: -------\n";
    std::cout << "Content-Type: text/html\r\n\r\n";

    // Output the HTML content
    std::cout << "<html><head><title>Hello, World!</title></head><body>";
    std::cout << "<h1>Hello, World!</h1>";
    std::cout << "<h1>you send me msg=>" 
        << request_body << "<-  </h1>";
    std::cout << "</body></html>";
    //close(0);

    return 0;
}
