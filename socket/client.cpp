#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>

// 1. socket()
// 2. connect()
// 3. read()/write()

#define SOCKET_PATH "/tmp/unix_socket_demo"

int main(){
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un server_addr{};
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to connect to server: " << strerror(errno) << std::endl;
        close(client_fd);
        return 1;
    }
    send(client_fd, "Hello", 6, 0);
    char buf[100];
    read(client_fd, buf, sizeof(buf));
    std::cout << "Received: " << buf << std::endl;
    close(client_fd);
    return 0;
}