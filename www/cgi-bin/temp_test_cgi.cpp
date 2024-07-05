#include <iostream>
#include <unistd.h>

int main() {
    const char* pythonPath = "/usr/bin/python3";
    const char* scriptPath = "script.py";
    char* const argv[] = {
        const_cast<char*>(pythonPath),
        const_cast<char*>(scriptPath),
        nullptr
    };
    char* const envp[] = {nullptr};
    if (execve(pythonPath, argv, envp) == -1) {
        perror("execve");
        return 1;
    }
    return 0;
}

