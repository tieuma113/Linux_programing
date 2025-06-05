#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

int main(){
    std::cout.setf(std::ios::unitbuf);
    std::string path = "/tmp/myfifo";
    int fd = open(path.c_str(), O_WRONLY | O_NONBLOCK);
    std::cout << "[WRITER] Sending message ....\n";
    std::string msg = "Hello FIFO\n\0"; 
    write(fd, msg.c_str(), msg.size());
    close(fd);
    return 0;
}