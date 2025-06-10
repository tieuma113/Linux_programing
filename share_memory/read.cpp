#include <cstdio>
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <sys/mman.h>   // mmap, munmap, PROT_*
#include <sys/stat.h>   // mode constants
#include <sys/types.h>  // off_t, size_t
#include <unistd.h>     // ftruncate, close
#include <iostream>
#include <stdlib.h>     // exit
#include <sys/shm.h>
#include <cstring>
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
    char buf[100] = {};
    strcpy(buf, (char*) shm_ptr);
    std::cout << "Reader got: " << buf << std::endl;
// Bước 4: Gỡ (detach) khỏi tiến trình
    shmdt(shm_ptr);
// Bước 5: Xoá vùng nhớ khi không dùng nữa
    shmctl(shm_id, IPC_RMID, nullptr);
#endif

//POSIX
#ifdef POSIX
    sem_t* sem = sem_open("/sem_name", 0);
    if(sem == SEM_FAILED){
        perror("sem open fail");
        return 1;
    }
    int fd = shm_open("/my_shm", O_RDONLY, 0666);
    void* ptr = mmap(0, 4096, PROT_READ, MAP_SHARED, fd, 0);
    sem_wait(sem); 
    char buf[100] = {};
    strcpy(buf,(char*) ptr);
    std::cout << "Reader got: " << buf << std::endl;
    sem_post(sem);
    sem_close(sem);
    close(fd);
    munmap(ptr, 4096);
#endif

    return 0;
}