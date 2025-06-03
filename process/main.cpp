#include <cstdio>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>

// #define EC1
// #define EC2
// #define EC3
// #define EC4
// #define EC5


// BÀI TẬP 1: Tạo Process Cha-Con và in PID
// Mục tiêu:
// Sử dụng fork(), getpid(), getppid()

// Yêu cầu:
// Tạo một process con bằng fork()
// In ra thông tin sau trong cả process cha và con:
// PID của chính nó
// PID của cha nó (PPID)
// Dùng sleep(2) trong process con để có thể thấy được rõ sự tách biệt.
#ifdef EC1
void ec1(){
    int pid = fork();

    if (pid < 0) {
        std::cerr << "Fork fail" << std::endl;
        return;
    }

    else if (pid == 0){
        //Child
        std::cout << "[Child] PID: " << getpid() << " PPID " << getppid() << std::endl;
        sleep(2);
        _exit(1);
    }
    else{
        //parent
        int status = 0;
        waitpid(pid, &status, 0);
        std::cout << "[Parent] PID " << getpid() << " PPID " << getppid() << std::endl;
        if (WIFEXITED(status)){
            std::cout << "[Parent] Child exited with code " << WEXITSTATUS(status) << std::endl;
        }
    }
}
#endif

// BÀI TẬP 2: Dùng exec() để thay thế chương trình
// Mục tiêu:
// Sử dụng fork() + exec()

// Yêu cầu:
// Process cha gọi fork()
// Trong process con, dùng execlp() để chạy lệnh ls -l /
// Nếu exec thất bại, in thông báo lỗi
#ifdef EC2
void ec2(){
    int pid = fork();

    if (pid < 0){
        std::cerr << "Fork fail" << std::endl; 
        return;
    }
    else if (pid == 0){
        std::cout << "[Child] PID = " << getpid() << " PPID = " << getppid() << std::endl;
        execlp("ls", "ls", "-l", nullptr);
        std::cerr << "[Child] exe fail" << std::endl;
        _exit(1);
    }
    else {
        std::cout << "[Parent] PID = " << getpid() << std::endl;
        std::cout << "[Parent] pid = " << pid << std::endl;
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)){
            std::cout << "[Parent] child exit with status " << WEXITSTATUS(status) << std::endl; 
        }
    }
}
#endif

// BÀI TẬP 3: Thu hoạch process con bằng wait()
// Mục tiêu:
// Tránh zombie

// Yêu cầu:
// Cha tạo process con
// Con sleep(3) rồi exit(42)
// Cha dùng wait() để chờ con và in ra mã thoát (exit code) bằng WEXITSTATUS(status)
#ifdef EC3
void ec3(){
    int pid = fork();

    if (pid < 0) {
        std::cerr << "Fork fail" << std::endl;
        return;
    }else if (pid == 0) {
        //Child process
        std::cout << "[Child] PID = " << getpid() << " PPID = " << getppid() << std::endl;
        sleep(3);
        exit(42);
    }else {
        //Parent process
        std::cout << "[Parent] PID = " << getpid() << std::endl;
        int status = 0;
        wait(&status);
        if (WIFEXITED(status)){
            std::cout << "[Parent] child exit with status code " << WEXITSTATUS(status) << std::endl;
        }
    }
    return;
}
#endif

// BÀI TẬP 4: So sánh exit() và _exit()
// Mục tiêu:
// Thấy được sự khác biệt về flush buffer

// Yêu cầu:
// Trong process con, ghi printf("hello world\n");
// Sau đó gọi:
// Lần 1: exit(0)
// Lần 2: _exit(0)
// Quan sát khác biệt trong output
#ifdef EC4
void ec4(){
    int pid = fork();

    if (pid < 0){
        std::cerr << "Fork fail " << std::endl;
        return;
    }
    else if (pid == 0){
        std::cout << "[Child] pid = " << getpid() << " ppid = " << getppid() << std::endl;
        printf("Hello world");
        sleep(2);
        exit(1);
    }
    else {
        std::cout << "[Parent] pid = " << getpid() << std::endl;
        int status = 0;
        wait(&status);
        if (WIFEXITED(status)){
            std::cout << "[Parent] child exit with status code " << WEXITSTATUS(status) << std::endl;
        }
    }
}
#endif

// BÀI TẬP 5: Tạo nhiều process con
// Mục tiêu:
// Hiểu được cách cha quản lý nhiều con

// Yêu cầu:
// Process cha tạo ra 3 process con
// Mỗi con in ra PID của nó và sleep(x) với x là thời gian khác nhau (ví dụ: 1s, 2s, 3s)
// Cha dùng wait() trong vòng lặp để thu hoạch từng con
#ifdef EC5
void ec5(){
    const int NUM_CHILD = 3;
    for (int i = 0; i < NUM_CHILD; i++){
        int pid = fork();
        if(pid < 0){
            std::cerr << "Fork fail " << std::endl;
            exit(1);
        }
        else if (pid == 0) {
            std::cout << "[Child " << i << "] PID = " << getpid()
                      << "\nPPID " << getppid()
                      << "sleeping " << (i+1) << std::endl;
            sleep(i+1);
            exit(i+1); 
        }
    }
    for (int i = 0; i < NUM_CHILD; i++){
        int status = 0;
        int pid = wait(&status);
        if(WIFEXITED(status)){
            std::cout << "[Parent] collect child PID = " << pid
                      << ", exit code " << WEXITSTATUS(status) << std::endl;
        }
    }
    return;
}
#endif

int main() {
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
    return 0;
}