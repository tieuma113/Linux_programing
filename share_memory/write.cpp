#include <cstdio>
#include <cstring>
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <sys/ipc.h>
#include <sys/mman.h>   // mmap, munmap, PROT_*
#include <sys/stat.h>   // mode constants
#include <sys/types.h>  // off_t, size_t
#include <unistd.h>     // ftruncate, close
#include <iostream>
#include <stdlib.h>     // exit
#include <sys/shm.h>
#include <semaphore.h>

#define POSIX

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

int main(){
#ifdef SYS_V
//SYS_V
// Bước 1: Tạo hoặc lấy bộ nhớ chung
    int shm_id = shmget(ftok("./", 0), 4096, IPC_CREAT | 0666);
// Bước 2: Gắn (attach) vào không gian địa chỉ tiến trình
    void* shm_ptr = shmat(shm_id, nullptr, 0);
// Bước 3: Truy cập như mảng bình thường
    strcpy((char*) shm_ptr, "Ready");
    std::cout << "Writer wrote: READY" << std::endl;
// Bước 4: Gỡ (detach) khỏi tiến trình
    shmdt(shm_ptr);
// Bước 5: Xoá vùng nhớ khi không dùng nữa
#endif


//POSIX
#ifdef POSIX
    const char* SEM_NAME = "/sem_name";
    const char* SHM_NAME = "/my_shm";
    const size_t SHM_SIZE = 4096;
    sem_t* sem = sem_open(SEM_NAME,0);
    if (sem == SEM_FAILED) {
        perror("sem fail");
        return 1;
    }
    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    void* ptr = mmap(nullptr, SHM_SIZE, PROT_WRITE, MAP_SHARED, fd, 0); 
    if (ptr == MAP_FAILED) {
        perror("mmap fail");
        return 1;
    }
    sem_wait(sem);
    std::string msg = "READY";
    strcpy((char*) ptr, msg.c_str());
    std::cout << "Writer wrote: " << msg << std::endl;
    sem_post(sem);
    sem_close(sem);
    close(fd);
    munmap(ptr, 4096);
#endif
    return 0;
}