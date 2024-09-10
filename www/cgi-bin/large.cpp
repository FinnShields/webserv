/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   large.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 14:19:44 by apimikov          #+#    #+#             */
/*   Updated: 2024/09/09 14:38:35 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <cstdlib>
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

int main() {
    std::string request_body = readFromFd(0);
    // Set the content type to HTML
    std::cout << "Content-Type: text/html\r\n";
    //const size_t SIZE = 1 * 1024 * 1024;
    const size_t SIZE = 1 * 1024;// + request_body.size();
    std::cout << "Content-Length: " << std::to_string(SIZE) <<"\r\n"; 
    std::cout << "\r\n";

    // Generate a large body of data
    std::string buffer(SIZE, 'A');

    // Write the body to the output
 //   std::cout.write(buffer.data(), buffer.size());
    std::cout << buffer << request_body;
    std::cout << "\r\n";
    
    return 0;
}