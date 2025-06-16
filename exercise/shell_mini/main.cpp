#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

void run_shell() {
    std::string line;
    while (true) {
        std::getline(std::cin, line);
        if (line.empty()){
            continue;
        }
        if (line == "exit") {
            break;
        }
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token){
            tokens.push_back(token);
        }
        std::vector<char*> argv;
        for (auto& it: tokens){
            argv.push_back(it.data());
        }
        argv.push_back(nullptr);
        int pid = fork();
        if(pid < 0){
            perror("fork fail");
        }else if (pid == 0) {
            execvp(argv[0], argv.data());
            perror("execvp fail");
            _exit(1);
        }
        waitpid(pid, nullptr, 0);
    }
    return;
}

int main() {
    run_shell();
    return 0;
}