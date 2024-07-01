#include <iostream>

int main() {
    // Output the HTTP headers
    std::cout << "Content-Type: text/html\r\n\r\n";

    // Output the HTML content
    std::cout << "<html><head><title>Hello, World!</title></head><body>";
    std::cout << "<h1>Hello, World!</h1>";
    std::cout << "</body></html>";

    return 0;
}
