#include <iostream>
#include <unistd.h>



std::string readFromFd(int fd) {
    std::string result;
    char buffer[1024];
    ssize_t size;
    while ((size = read(fd, buffer, sizeof(buffer))) > 0) {
         std::cerr << "From cgi script: size=" << size <<"\n";
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
    //std::cout << "";
    return 0;
}
