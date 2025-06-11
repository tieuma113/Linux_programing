#include <csignal>
#include <cstdio>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <vector>
#include "RingBuffer.h"

constexpr int LOOP_VALUE = 10000;
constexpr const char* SHM_NAME = "/shm_buffer";
constexpr const char* SEM_MUTEX = "/sem_mutex";
constexpr const char* SEM_FULL = "/sem_full";
constexpr const char* SEM_EMPTY = "/sem_empty";

std::vector<int> CHILD_PROCESES;
int shm_fd = -1;
void* shm_ptr = nullptr;
sem_t* sem_mutex = nullptr;
sem_t* sem_full = nullptr;
sem_t* sem_empty = nullptr;

// Gọi khi nhấn Ctrl+C
void handle_sigint(int signum) {
    std::cout << "\n[CTRL+C] Caught signal " << signum << ", cleaning up...\n";

    //Đóng process con
    for(auto it : CHILD_PROCESES){
        kill(it, SIGTERM);
    }
    // Đóng & unlink semaphore
    if (sem_mutex) { sem_close(sem_mutex); sem_unlink(SEM_MUTEX); }
    if (sem_full)  { sem_close(sem_full);  sem_unlink(SEM_FULL);  }
    if (sem_empty) { sem_close(sem_empty); sem_unlink(SEM_EMPTY); }

    // Gỡ ánh xạ shared memory
    if (shm_ptr) munmap(shm_ptr, sizeof(RingBuffer));
    if (shm_fd != -1) {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }

    std::exit(0);  // Thoát sạch
}

int create_producer(RingBuffer* buf, sem_t* sem_mutex, sem_t* sem_empty, sem_t* sem_full){
    int pid = fork();
    if(pid < 0){
        perror("fail to fork");
        return -1;
    }
    else if (pid == 0) {
        std::cout << "[PRODUCER] pid = " << getpid() << std::endl;
        for (int i = 0; i <= LOOP_VALUE; ++i) {
            sem_wait(sem_empty);
            sem_wait(sem_mutex);
            buf->push(i);
            std::cout << "[PRODUCER pid = " << getpid() << "] add value = " << i << std::endl;
            sem_post(sem_mutex);
            sem_post(sem_full);
        } 
        _exit(1);
    }
    CHILD_PROCESES.push_back(pid);
    return pid;
}

int create_consumer(RingBuffer* buf, sem_t* sem_mutex, sem_t* sem_empty, sem_t* sem_full){
    int pid = fork();
    if(pid < 0){
        perror("fail to fork");
        return -1;
    }
    else if (pid == 0) {
        std::cout << "[CONSUMER] pid = " << getpid() << std::endl;
        int value = 0;
        for (int i = 0; i <= 3 * LOOP_VALUE; ++i) {
            sem_wait(sem_full);
            sem_wait(sem_mutex);
            value = buf->pop();
            std::cout << "[CONSUMER pid = " << getpid() << "] get value = " << value << std::endl;
            sem_post(sem_mutex);
            sem_post(sem_empty);
        } 
        _exit(1);
    }
    CHILD_PROCESES.push_back(pid);
    return pid;
}

int main(){
    signal(SIGINT, handle_sigint); // Gán handler Ctrl+C
    //Tạo shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    //Khởi tạo size cho shared memory
    if (shm_fd == -1 || ftruncate(shm_fd, sizeof(RingBuffer))) {
        perror("create shared memory fail");
        return 1;
    }
    //Map shared memory
    void* shm_ptr = mmap(nullptr, sizeof(RingBuffer), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd,0);
    RingBuffer* buf = static_cast<RingBuffer*>(shm_ptr);
    new (buf) RingBuffer;
    //Tạo semaphore
    sem_t* sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t* sem_full = sem_open(SEM_FULL, O_CREAT, 0666, 0);
    sem_t* sem_empty = sem_open(SEM_EMPTY, O_CREAT, 0666, RingBuffer::BUF_SIZE);

    //Tạo process producer
    int producer1 = create_producer(buf, sem_mutex, sem_empty, sem_full);
    int producer2 = create_producer(buf, sem_mutex, sem_empty, sem_full);
    int producer3 = create_producer(buf, sem_mutex, sem_empty, sem_full);
    //Tạo process consumer
    int consumer = create_consumer(buf, sem_mutex, sem_empty, sem_full);

    //Wait process con
    waitpid(producer1, nullptr, 0);
    waitpid(producer2, nullptr, 0);
    waitpid(producer3, nullptr, 0);
    waitpid(consumer, nullptr, 0);
    
    //Đóng semaphore
    sem_close(sem_mutex);
    sem_close(sem_full);
    sem_close(sem_empty);
    //Unlink semaphore
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_EMPTY);
    //Đóng shared memory
    close(shm_fd);
    //Unmap shared memory
    munmap(shm_ptr, sizeof(RingBuffer));
    //Unlink shared memory
    shm_unlink(SHM_NAME);
    return 0;
}
