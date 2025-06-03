#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <future>
#include <utility>
#include <vector>

// #define EC1
// Bài tập 1: In số bằng 2 thread
// Mục tiêu: luyện std::thread, truyền tham số
// Yêu cầu:
// Tạo 2 thread:
// Thread 1 in số từ 1 → 5
// Thread 2 in số từ 6 → 10
// Kết thúc chương trình khi cả hai thread xong.
// Gợi ý dùng: std::thread, join
void ec1(){
    std::thread t1([](){
        for (int i = 1; i <=500; i++){
            std::cout << i << std::endl;
        }
    });
    std::thread t2([](){
        for (int i = 501; i <= 1000; i++){
            std::cout << i << std::endl;
        }
    });
    if (t1.joinable()) {
        t1.join();
    }
    if (t2.joinable()) {
        t2.join();
    }
}


// Bài tập 2: Tăng biến toàn cục từ nhiều thread
// Mục tiêu: Hiểu và tránh race condition
// Yêu cầu:
// Có 1 biến int counter = 0
// Tạo 10 thread, mỗi thread tăng counter 1000 lần
// In kết quả cuối cùng
// Yêu cầu mở rộng:
// Chạy thử không dùng mutex → xem sai số
// Sau đó dùng std::mutex → kết quả đúng
// Gợi ý dùng: std::thread, std::mutex, std::lock_guard

void ec2(){
    int counter = 0;
    int NUMER_THREAD = 100;
    std::mutex m;
    std::vector<std::thread> thread_pool;
    for (int i =0; i < NUMER_THREAD; i++){
        std::thread t([&counter](){
            for(int i = 0; i < 1000;  i++){
                counter++;
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
        thread_pool.push_back(std::move(t));
    }
    for (auto it = thread_pool.begin(); it != thread_pool.end(); it++){
        if(it->joinable()){
            it->join();
        }
    }
    std::cout << counter << std::endl;

    thread_pool.clear();
    counter = 0;
    for (int i =0; i < NUMER_THREAD; i++){
        std::thread t([&counter, &m](){
            for(int i = 0; i < 1000;  i++){
                std::lock_guard<std::mutex> lock(m);
                counter++;
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
        thread_pool.push_back(std::move(t));
    }
    for (auto it = thread_pool.begin(); it != thread_pool.end(); it++){
        if(it->joinable()){
            it->join();
        }
    }
    std::cout << counter << std::endl;
}


// Bài tập 3: Producer - Consumer với condition_variable
// Mục tiêu: Đồng bộ 2 thread với condition_variable
// Yêu cầu:
// Producer: đợi 2s rồi đặt ready = true, gọi notify_one
// Consumer: chờ ready == true rồi in ra "Data is ready"
// Gợi ý dùng: std::condition_variable, std::unique_lock, lambda

void ec3(){
    std::mutex mtx;
    std::condition_variable con;
    bool ready = false;

    std::thread consumer([&mtx, &con, &ready](){
        std::unique_lock<std::mutex> lock(mtx);
        std::cout << "consumer start" << std::endl;
        con.wait(lock, [&ready]{return ready;});
        std::cout << "consumer continue ..." << std::endl;
    });

    std::thread producer([&mtx, &con, &ready](){
        std::cout << "producer start " << std::endl;
        std::lock_guard<std::mutex> lock(mtx);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        ready = true;
        std::cout << "producer done" << std::endl;
        con.notify_one();
    });
    consumer.join();
    producer.join();
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
}