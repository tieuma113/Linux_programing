#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <semaphore.h>
#include <sys/mman.h>   // mmap, munmap, PROT_*
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/stat.h>   // mode constants
#include <sys/types.h>  // off_t, size_t
#include <sys/wait.h>
#include <unistd.h>     // ftruncate, close
#include <string>       // strcpy, strlen
#include <iostream>
#include <stdlib.h>     // exit


// Viết 2 file C++ riêng biệt:
//      1. File writer.cpp
// Tạo shared memory tên /mybuffer
// Set size = 4096 byte
// Ghi chuỗi "READY" vào vùng nhớ
// In ra: "Writer wrote: READY"
//      2. File reader.cpp
// Mở lại shared memory /mybuffer
// Đọc chuỗi từ shared memory
// In ra: "Reader got: READY"
//      3. Cleanup
// Chỉ reader mới gọi shm_unlink("/mybuffer")
// Cả hai process đều phải munmap và close
void ec1(){
    const char* SEM_NAME = "/sem_name";
    const char* SHM_NAME = "/my_shm";
    const size_t SHM_SIZE = 4096;
    sem_t* sem = sem_open(SEM_NAME, O_CREAT| O_EXCL, 0666, 1);
    if(sem == SEM_FAILED){
        if (errno == EEXIST) {
            sem = sem_open(SEM_NAME, 0);
        }
        else{
            perror("sem fail");
            return;
        }
    }
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if(fd == -1 || ftruncate(fd, 4096)){
        perror("shm setup fail");
        return;
    }
    int pipe_read[2];
    int pipe_write[2];
    if(pipe(pipe_read) == -1 || pipe(pipe_write) == -1){
        perror("pipe failed");
        return;
    }
    int pid1 = fork();
    if(pid1 < 0){
        perror("pid1 failed");
        return;
    }
    else if(pid1 == 0){
        //write
        std::cout << "[WRITE] pid " << getpid() << std::endl;
        close(pipe_write[0]);
        dup2(pipe_write[1], STDOUT_FILENO);
        close(pipe_write[1]);
        char* args[] = {(char*) "./build/write", nullptr};
        execvp(args[0], args);
        perror("exec fail");
        _exit(1);
    }
    sleep(1);
    int pid2 = fork();
    if(pid2 < 0){
        perror("pid2 failed");
        return;
    }
    else if (pid2 == 0) {
        //read
        std::cout << "[READ] pid " << getpid() << std::endl;
        close(pipe_read[0]);
        dup2(pipe_read[1], STDOUT_FILENO);
        close(pipe_read[1]);
        char* args[] = {(char*) "./build/read", nullptr};
        execvp(args[0], args);
        perror("exec fail");
        _exit(1);
    }
    close(pipe_write[1]);
    close(pipe_read[1]);
    char buf[256] = {};
    fd_set readfds;
    int maxfd = std::max(pipe_read[0], pipe_write[0]) + 1;
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(pipe_read[0], &readfds);
        FD_SET(pipe_write[0], &readfds);

        int ready = select(maxfd, &readfds, nullptr, nullptr, nullptr);
        if(ready == -1){
            perror("select fail");
            break;
        }
        if(FD_ISSET(pipe_read[0], &readfds)){
            ssize_t n = read(pipe_read[0], buf, sizeof(buf) - 1);
            if (n <= 0) break;
            buf[n] = '\0';
            std::cout << "[READER] " << buf;
        }
        if(FD_ISSET(pipe_write[0], &readfds)){
            ssize_t n = read(pipe_write[0], buf, sizeof(buf) -1);
            if (n <= 0) break;
            buf[n] = '\0';
            std::cout << "[WRITER ] " << buf;
        }
    }

    sem_close(sem);
    sem_unlink(SEM_NAME);
    close(fd);
    shm_unlink(SHM_NAME);
    close(pipe_write[0]);
    close(pipe_read[0]);
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    return;
}


int main(){
#ifdef EC1
    ec1();
#endif
}