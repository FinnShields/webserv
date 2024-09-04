#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>



std::string readFromFd(int fd) {
    std::string result;
    char buffer[4096];
    ssize_t size;
    while ((size = read(fd, buffer, sizeof(buffer))) > 0) {
        // std::cerr << "From cgi script: size=" << size <<"\n";
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

    //std::cerr << "-------argc: " << argc << std::endl;
    std::string request_body = readFromFd(0);
    //oss << "HTTP/1.1 200 OK\r\n";
    std::cout << "Content-Type: text/html\r\n";

    std::ostringstream oss;

    oss << "<html><head><title>Hello, World!  I am CGI script</title></head><body>";
    oss << "<h1>Hello, World! I am CGI script</h1><br><br>\n";
    if (request_body.empty())
        oss << "<h1>You send me http request, with empty body.<br>\n";
    else
        oss << "<h1>You send me http request, with the body:<br>" 
            << request_body << "<br><br>\n";
    oss << "My enviroment variables:<br>\n";
    int i = 0;
    while (envp[i]){
        oss << envp[i] << "<br>\n";
        ++i;
    }
    oss << "</h1>";
    oss << "</body></html>";
    std::string body = oss.str();
    //close(0);
   // std::cout << "Content-Length: " << body.size() << "\r\n\r\n";
    std::cout << "\r\n";

    std::cout << body;
    return 0;
}
