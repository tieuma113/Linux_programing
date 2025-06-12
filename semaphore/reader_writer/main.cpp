#include <csignal>
#include <cstring>
#include <exception>
#include <thread>
#include <unistd.h>
#include <cstdio>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

//      2. Readers – Writers Problem (Bài toán độc giả – tác giả)
//  Mục tiêu tổng quát
// Quản lý truy cập đồng thời vào tài nguyên dùng chung (shared resource) như:
// Shared memory
// File
// Database

//  Yêu cầu bắt buộc:
// Cho phép nhiều reader cùng đọc tài nguyên nếu không có writer
// Writer phải được quyền độc quyền: không reader nào được đọc khi writer đang ghi

//  Yêu cầu kỹ thuật cụ thể
// Yêu cầu	Diễn giải rõ
// 1. Đa reader đồng thời	                Cho phép N thread/process reader đọc cùng lúc
// 2. Writer phải độc quyền	                Không được có reader hoặc writer khác
// 3. Tránh race condition	                Dùng mutex, semaphore để bảo vệ biến đếm
// 4. Tránh deadlock	                    Phải có thứ tự lock chính xác
// 5. Có thể mở rộng ưu tiên	            Ưu tiên reader hoặc writer tùy bài toán

//  Các biến dùng trong hệ thống:
// Biến/Tài nguyên	                        Mục đích
// int reader_count	                        Số lượng reader đang đọc
// sem_t mutex	                            Bảo vệ truy cập reader_count
// sem_t resource	                        Điều phối quyền truy cập tài nguyên dùng chung
// (tùy chọn) sem_t wrt_priority	        Bảo vệ ưu tiên writer nếu có yêu cầu

#define NUM_READER 3
#define NUM_WRITER 2

struct SharedData{
    int readers;
    char buffer[128];
    sem_t sem_mutex; //Protect reader count
    sem_t sem_write; //Protect writer access
};

SharedData shared;

void create_reader(){
    std::cout << "[Reader] create reader" << std::endl;
    while (true) {
        sem_wait(&shared.sem_mutex);
        shared.readers++;
        if(shared.readers == 1) //First reader need to check writer
            sem_wait(&shared.sem_write);
        sem_post(&shared.sem_mutex);

        //Reading
        std::cout << "[Reader " << gettid() <<" ] Read: " <<shared.buffer << std::endl;
        sleep(5);

        sem_wait(&shared.sem_mutex);
        shared.readers--;
        if(shared.readers == 0)
            sem_post(&shared.sem_write); //Last reader unlock writer
        sem_post(&shared.sem_mutex);
        sleep(5);
    }
}

void create_writer(){
    std::cout << "[Writer] create writer" << std::endl;
    int count = 0;
    while (true) {
        sem_wait(&shared.sem_write);

        //Write section
        snprintf(shared.buffer, sizeof(shared.buffer), "Message %d from tid = %d", ++count, gettid());
        std::cout << "[Writer " << gettid() << "] Wrote: " << shared.buffer << std::endl;
        sleep(8);

        sem_post(&shared.sem_write);
        sleep(8);
    }
}

void create_resource(){
    shared.readers = 0;
    strcpy(shared.buffer, "Initial message");
    sem_init(&shared.sem_mutex, 0, 1);
    sem_init(&shared.sem_write, 0, 1);
}

void release_resource(){
    sem_destroy(&shared.sem_mutex);
    sem_destroy(&shared.sem_write);
}

void handle_siginit(int signum){
    std::cout << "\n[CTRL+C] Caught signal " << signum << ", cleaning up...\n";
    release_resource();
    exit(0);
}


int main(){
    signal(SIGINT, handle_siginit);
    create_resource();
    std::vector<std::thread> read_threads;
    std::vector<std::thread> write_threads;

    for(int i = 0; i < NUM_WRITER; ++i){
        write_threads.push_back(std::thread([]() {create_writer();}));
    }

    for(int i = 0; i < NUM_READER; ++i){
        read_threads.push_back(std::thread([](){create_reader();}));
    }

    for(int i = 0; i < NUM_WRITER; i++){
        if (write_threads[i].joinable()){
            write_threads[i].join();
        }
    }

    for(int i = 0; i < NUM_READER; i++){
        if (read_threads[i].joinable()){
            read_threads[i].join();
        }
    }

    release_resource();
}