#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/stat.h>
#include <vector>


// PHẦN I: PIPE (unnamed)
// Bài 1 – Giao tiếp đơn giản giữa cha và con (1 chiều)
// Mô tả: Process cha gửi chuỗi "Ping" đến con qua pipe. Con in ra chuỗi rồi kết thúc.
void ec1(){
    int pipe_fd[2] = {};
    if (pipe(pipe_fd) == -1) {
        std::cerr << "fail to open pipe" << std::endl;
        return;
    }
    int pid = fork();
    if (pid < 0) {
        std::cerr << "fork fail" << std::endl;
        return;
    }
    else if (pid == 0) {
        //child
        close(pipe_fd[1]);
        char buf[100];
        ssize_t n = read(pipe_fd[0], buf, sizeof(buf));
        if (n > 0){
            std::cout << "[Child] Received: " << buf;
        }
        else {
            std::cerr << "[Child] fail to read" << std::endl; 
        }
        close(pipe_fd[0]);
        _exit(1);
    }
    else{
        //parent
        close(pipe_fd[0]);
        const std::string buf = "Hello world\n";
        write(pipe_fd[1], buf.c_str(), buf.size());
        close(pipe_fd[1]);
        int status = 0;
        wait(&status);
        if(WIFEXITED(status)){
            std::cout << "Sub process return code " << WEXITSTATUS(status) << std::endl;
        }
    }
    return;
}

//  Bài 2 – Giao tiếp 2 chiều giữa cha và con (full-duplex)
// Yêu cầu:
// Tạo 2 pipe: pipe1 và pipe2
// Cha gửi "Ping" → con nhận và phản hồi "Pong"
// Cha in kết quả từ con
// Cẩn thận đóng đúng các đầu pipe để tránh treo!

#define CHILD_READ 0
#define CHILD_WRITE 1
#define PARENT_READ 1
#define PARENT_WRITE 0
void ec2(){
    int pipe_fd1[2];
    int pipe_fd2[2];
    if(pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1){
        std::cerr << "fail to open pipe" << std::endl;
        return;
    }
    std::vector<int> pipe_read;
    std::vector<int> pipe_write;
    pipe_read.push_back(pipe_fd1[0]);
    pipe_read.push_back(pipe_fd2[0]);
    pipe_write.push_back(pipe_fd1[1]);
    pipe_write.push_back(pipe_fd2[1]);
    int process_fd = fork();
    if(process_fd < 0){
        std::cerr << "fork fail" << std::endl;
        return;
    }
    else if (process_fd == 0) {
        //Child
        close(pipe_read[PARENT_READ]);
        close(pipe_write[PARENT_WRITE]);
        char buf[100] = {};
        ssize_t n = read(pipe_read[CHILD_READ], buf, sizeof(buf) - 1);
        if(n > 0){
            std::cout << "[Child] Received " << buf;
            const std::string reply = "Pong\n";
            write(pipe_write[CHILD_WRITE], reply.c_str(), reply.size());
        }
        close(pipe_read[CHILD_READ]);
        close(pipe_write[CHILD_WRITE]);
        _exit(1);
    }
    else{
        //Parent
        close(pipe_read[CHILD_READ]);
        close(pipe_write[CHILD_WRITE]);
        const std::string message = "Ping\n";
        char buf[128] = {};
        write(pipe_write[PARENT_WRITE], message.c_str(), message.size());
        ssize_t n = read(pipe_read[PARENT_READ], buf, sizeof(buf) - 1);
        if (n > 0){
            std::cout << "[Parent] Received: " << buf;
        }
        close(pipe_read[PARENT_READ]);
        close(pipe_write[PARENT_WRITE]);
        int status;
        wait(&status);
        if(WIFEXITED(status)){
            std::cout << "[Parent] Child exit code " << WEXITSTATUS(status) << std::endl;
        }
    }
}

// Bài 3 – Redirect stdout của ls vào process grep
// Mục tiêu: Luyện dup2() + execvp().
// Yêu cầu:
// Dùng pipe() để nối ls -l → grep .cpp (Tương tự với dùng "ls -l | grep ".cpp")
// Gợi ý: dùng fork(), rồi dup2(pipefd[1], STDOUT_FILENO) trong ls, dup2(pipefd[0], STDIN_FILENO) trong grep.
void ec3(){
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1){
        std::cerr << "pipe failed" << std::endl;
        return;
    }
    int pid1 = fork();
    if(pid1 < 0) {
        std::cerr << "pid1 fork fail" << std::endl;
        return;
    }
    else if(pid1 == 0){
        //Child1
        //ls -l
        //ghi stdout vao pipe
        std::cout << "[PID1] pid: " << getpid() << std::endl;
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);
        char* args[] = {(char*)"ls", (char*)"-l", nullptr};
        execvp(args[0], args);
        std::cerr << "exec ls fail" << std::endl;
        exit(1);
    }
    int pid2 = fork();
    if (pid2 < 0){
        std::cerr << "pid2 fork fail" << std::endl;
        return;
    }
    else if (pid2 == 0) {
        //Child2
        //grep .cpp
        //read stdin
        std::cout << "[PID2] pid: " << getpid() << std::endl;
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);

        char* args[] = {(char*)"grep", (char*)".cpp", nullptr};
        execvp(args[0], args);
        std::cerr << "exec grep fail" << std::endl;
        exit(1);
    }
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    return;
}

// Bài 4 – FIFO giao tiếp giữa 2 tiến trình
// Yêu cầu:
// Tạo FIFO /tmp/myfifo
// Process A ghi "Hello FIFO" vào FIFO
// Process B đọc và in ra
// Có thể viết thành 2 chương trình riêng biệt: writer.cpp và reader.cpp
void ec4(){
    std::string path = "/tmp/myfifo";
    std::cout << "Make fifo" << std::endl;
    if (mkfifo(path.c_str(), 0666) == -1 && errno != EEXIST){
        std::cerr << "mkfifo fail" << std::endl;
        return;
    }
    int pipe_read[2];
    int pipe_write[2];
    std::cout << "Create read process" << std::endl;
    if(pipe(pipe_read) == -1|| pipe(pipe_write) == -1){
        std::cerr << "open pipe fail" << std::endl;
        return;
    }

    int pid1 = fork();
    if(pid1 < 0){
        std::cerr << "fork fail" << std::endl;
        return;
    }
    else if (pid1 == 0) {
        //read
        std::cout << "[PID1] pid: " << getpid() << std::endl;
        close(pipe_read[0]);
        dup2(pipe_read[1], STDOUT_FILENO);
        close(pipe_read[1]);
        char* args[] = {(char*)"./build/read", nullptr};
        sleep(1);
        execvp(args[0], args);
        std::cerr << "[READ] exec fail" << std::endl;
        _exit(1);
    }
    int pid2 = fork();
    std::cout << "Create process write" << std::endl;
    if(pid2 < 0){
        std::cerr << "fork fail" << std::endl;
        return;
    }
    else if (pid2 == 0) {
        //write
        std::cout << "[PID2] pid:" << getpid() << std::endl;
        close(pipe_write[0]);
        dup2(pipe_write[1], STDOUT_FILENO);
        close(pipe_write[1]);
        char* args[] = {(char*)"./build/write", nullptr};
        sleep(1);
        execvp(args[0], args);
        std::cerr << "[WRITE] exec fail" << std::endl;
        _exit(1);
    }
    close(pipe_read[1]);
    close(pipe_write[1]);

    char buf[256];
    fd_set readfds;
    int maxfd = std::max(pipe_read[0], pipe_write[0]) + 1;

    while(true){
        FD_ZERO(&readfds);
        FD_SET(pipe_read[0], &readfds);
        FD_SET(pipe_write[0], &readfds);

        int ready = select(maxfd, &readfds, nullptr, nullptr, nullptr);
        if(ready == -1){
            std::cerr << "select fail" << std::endl;
            break;
        }
        if(FD_ISSET(pipe_read[0], &readfds)){
            ssize_t n = read(pipe_read[0], buf, sizeof(buf) -1);
            if (n <= 0) break;
            buf[n] = '\0';
            std::cout << "[READER]: " << buf;
        }
        if(FD_ISSET(pipe_write[0], &readfds)){
            ssize_t n = read(pipe_write[0], buf, sizeof(buf) -1);
            if (n <= 0) break;
            buf[n] = '\0';
            std::cout << "[WRITE]: " << buf;
        }
    }
    close(pipe_read[0]);
    close(pipe_write[0]);


    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    unlink(path.c_str());
    return;
}

// Bài 5 – Blocking behavior của FIFO
// Mô tả:
// Chạy process reader trước, mở FIFO ở O_RDONLY, rồi sleep vài giây
// Sau đó mới chạy writer
// Quan sát xem reader có bị block không → giải thích vì sao
void ec5(){
    std::string path = "/tmp/myfifo";
    std::cout << "Make fifo" << std::endl;
    if(mkfifo(path.c_str(), 0666) == -1 && errno != EEXIST){
        std::cerr << "make fifo fail" << std::endl;
        return;
    }
    //make output pipe
    int pipe_read[2];
    int pipe_write[2];
    if(pipe(pipe_read) == -1 || pipe(pipe_write) == -1){
        std::cout << "Fail to create pipe" << std::endl;
    }
    int pid1 = fork();
    if (pid1 < 0){
        std::cerr << "Fail to open reader process" << std::endl;
        return;
    }else if(pid1 == 0){
        //reader process
        //redirect output stream
        close(pipe_read[0]);
        close(pipe_write[0]);
        close(pipe_write[1]);
        dup2(pipe_read[1], STDOUT_FILENO);
        close(pipe_read[1]);
        char* args[] = {(char*)"./build/read", nullptr};
        std::cout << "[READER] TRY OPEN" << std::endl;
        execvp(args[0], args);
        std::cerr << "Fail to open read binary" << std::endl;
        _exit(1);
    }

    int pid2 = fork();
    if(pid2 < 0){
        std::cerr << "Fail to open write process" << std::endl;
        return;
    }
    else if (pid2 == 0) {
        close(pipe_read[0]);
        close(pipe_read[1]);
        close(pipe_write[0]);
        dup2(pipe_write[1], STDOUT_FILENO);
        close(pipe_write[1]);
        sleep(10);
        char* args[] = {(char*)"./build/write", nullptr};
        std::cout << "[WRITER] TRY OPEN" << std::endl;
        execvp(args[0], args);
        std::cerr << "fail to open write binary" << std::endl;
        _exit(1);
    }

    //main process
    close(pipe_read[1]);
    close(pipe_write[1]);
    char buf[256];
    fd_set readfds;
    int maxfd = std::max(pipe_read[0], pipe_write[0]) + 1;
    while (true)
    {    
        FD_ZERO(&readfds);
        FD_SET(pipe_read[0], &readfds);
        FD_SET(pipe_write[0], &readfds);

        int ready = select(maxfd, &readfds, nullptr, nullptr, nullptr);
        if (ready == -1){
            std::cerr << "select fail" << std::endl;
            break;
        }
        if(FD_ISSET(pipe_read[0], &readfds)){
            ssize_t n = read(pipe_read[0], buf, sizeof(buf) - 1);
            std::cout << "[READ] n = " << n << std::endl;
            if (n <= 0) break;
            buf[n] = '\0';
            std::cout << buf;
        }
        if(FD_ISSET(pipe_write[0], &readfds)){
            ssize_t n = read(pipe_write[0], buf, sizeof(buf) - 1);
            std::cout << "[WRITE] n = " << n << std::endl;
            if(n <= 0) break;
            buf[n] = '\0';
            std::cout << buf;
        }
    }
    close(pipe_read[0]);
    close(pipe_write[0]);

    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    unlink(path.c_str());
    return;
}

//  Bài 6 – Nhiều tiến trình ghi vào FIFO
// Mục tiêu: Quan sát race condition.
// Yêu cầu:
//  Tạo FIFO
//  3 tiến trình ghi "Hello from A/B/C" cùng lúc
//  Tiến trình đọc in toàn bộ dữ liệu nhận được
// Câu hỏi phụ: Nếu không đồng bộ, dữ liệu bị trộn không?
void ec6(){
    std::string path = "/tmp/myfifo";
    if(mkfifo(path.c_str(), 0666) == -1 && errno != EEXIST){
        std::cerr << "Fail to create fifo" << std::endl;
    }
    std::cout << "Create process 1" << std::endl;
    int pid1 = fork();
    if(pid1 < 0){
        std::cerr << "fork faiil" << std::endl;
        return;
    }
    else if (pid1 == 0) {
        std::cout << "Success creat process 1" << std::endl;
        int fd = open(path.c_str(), O_WRONLY | O_NONBLOCK);
        std::string msg = "[PROCESS1] Hello from A";
        sleep(3);
        write(fd, msg.c_str(), msg.size());
        close(fd);
        _exit(1);
    }
    int pid2 = fork();
    if(pid2 < 0){
        std::cerr << "fork fail" << std::endl;
        return;
    }
    else if (pid2 == 0) {
        std::cout << "Success create process 2" << std::endl;
        int fd = open(path.c_str(), O_WRONLY | O_NONBLOCK);
        std::string msg = "[PROCESS2] Hello from B";
        sleep(3);
        write(fd, msg.c_str(), msg.size());
        close(fd);
        _exit(1);
    }
    int pid3 = fork();
    if(pid3 < 0){
        std::cerr << "fork fail" << std::endl;
        return;
    }
    else if (pid3 == 0) {
        std::cout << "Success create process 3" << std::endl;
        int fd = open(path.c_str(), O_WRONLY | O_NONBLOCK);
        std::string msg = "[PROCESS3] Hello from C";
        sleep(3);
        write(fd, msg.c_str(), msg.size());
        close(fd);
        _exit(1);
    }
    //parent
    int fd = open(path.c_str(), O_RDONLY);
    char buf[100];
    ssize_t n = read(fd, buf, sizeof(buf));
    if(n>0){
        std::cout << "received: " << buf;
    }
    close(fd);
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    waitpid(pid3, nullptr, 0);
    unlink(path.c_str());
}

int main(){
    #ifdef EC1
    ec1();
    #endif

    #ifdef EC2
    ec2();
    #endif

    #ifdef EC3
    ec3();
    #endif

    #ifdef EC4
    ec4();
    #endif

    #ifdef EC5
    ec5();
    #endif

    #ifdef EC6
    ec6();
    #endif
    return 0;
}
