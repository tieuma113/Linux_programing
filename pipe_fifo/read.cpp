#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

int main(){
    std::cout.setf(std::ios::unitbuf);
    std::string path = "/tmp/myfifo";
    int fd = open(path.c_str(), O_RDONLY);
    char buf[100] = {};
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n > 0){
        std::cout << "[READER] received: " << buf;
    }
    close(fd);
}