#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>
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
    int thread = fork();
    if (thread < 0) {
        std::cerr << "fork fail" << std::endl;
        return;
    }
    else if (thread == 0) {
        //child
        close(pipe_fd[1]);
        char buf[100];
        read(pipe_fd[0], buf, sizeof(buf));
        std::cout << "Child received: " << buf << std::endl;
        close(pipe_fd[0]);
        _exit(1);
    }
    else{
        //parent
        close(pipe_fd[0]);
        const char* buf = "Hello world\n";
        write(pipe_fd[1], buf, strlen(buf) + 1);
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
        void* buf = malloc(sizeof(char) * 100);
        read(pipe_read[CHILD_READ], buf, sizeof(buf));
        std::cout << "[Child] Receive:" << (char*)buf << std::endl;
        memccpy(buf, "Pong\n", 0, 5);
        write(pipe_write[CHILD_WRITE], buf, strlen((char*)buf) +1);
        close(pipe_read[CHILD_READ]);
        close(pipe_write[CHILD_WRITE]);
        _exit(1);
    }
    else{
        //Parent
        close(pipe_read[CHILD_READ]);
        close(pipe_write[CHILD_WRITE]);
        void* buf = malloc(sizeof(char)* 100);
        memccpy(buf, "Ping\n", 0, 5);
        write(pipe_write[PARENT_WRITE], buf, strlen((char*)buf) + 1);
        read(pipe_read[PARENT_READ], buf, sizeof(buf));
        std::cout << "[Parent] Receive: " << (char*)buf << std::endl;
        close(pipe_read[PARENT_READ]);
        close(pipe_write[PARENT_WRITE]);
        int status;
        wait(&status);
        if(WIFEXITED(status)){
            std::cout << "[Parent] Child exit code " << WEXITSTATUS(status) << std::endl;
        }
        free(buf);
    }
}

// Bài 3 – Redirect stdout của ls vào process grep
// Mục tiêu: Luyện dup2() + execvp().
// Yêu cầu:
// Dùng pipe() để nối ls -l → grep .cpp
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
    return 0;
}
