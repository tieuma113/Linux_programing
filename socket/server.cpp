#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>

// 1. socket()
// 2. bind()
// 3. listen()
// 4. accept()
// 5. read()/write()

#define SOCKET_PATH "/tmp/unix_socket_demo"

int main(){
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    std::cout << "socket fd = " << server_fd << std::endl;
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return 1;
    }
    listen(server_fd, 5);
    
    std::cout << "Wait for connection ....." << std::endl;
    int client_fd = accept(server_fd, nullptr, nullptr);

    char buf[100] = {};
    read(client_fd, buf, sizeof(buf));
    std::cout << "Received: " << buf << std::endl;

    write(client_fd, "Hello from UNIX server", 23);
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}