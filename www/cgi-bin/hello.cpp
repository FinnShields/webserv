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


int main(int argc, char** argv, char** envp) {
    (void)argc;
    (void)argv;

    std::string request_body = readFromFd(0);
    std::cout << "HTTP/1.1 200 OK\r\n";
    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << "<html><head><title>Hello, World!  I am CGI script</title></head><body>";
    //std::cout << "<h1>Hello, <br> World!</h1>";
    std::cout << "<h1>You send me http request, with the body:<br>" 
        << request_body << "<br><br>\n";
    std::cout << "My enviroment variables:<br>\n";
    int i = 0;
    while (envp[i]){
        std::cout << envp[i] << "<br>\n";
        ++i;
    }
    std::cout << "</h1>";
    std::cout << "</body></html>";
    //close(0);

    return 120;
}
