#include <cstddef>
#include <cstdio>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

constexpr const char* MQ_NAME = "/demo_queue";
constexpr std::size_t MAX_MSG_SIZE = 128;

int main(){
    struct mq_attr attr {};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if(mq == -1) {perror("mq_open fail"); return 1;}

    if(fork() == 0){
        const char* text = "Hello work from child via POSIX MQ";
        if(mq_send(mq, text, std::strlen(text) + 1, 5) == -1){
            perror("mq_send");
        }else {
            std::cout << "[CHILD] send message\n";
        }
        mq_close(mq);
        _exit(0);
    }
    char buf[MAX_MSG_SIZE];
    unsigned int prio;
    ssize_t n = mq_receive(mq, buf, sizeof(buf), &prio);
    if(n == -1){
        perror("mq_receive");
    }else {
        std::cout << "[PARENT] got (" << n << "bytes, prio " << prio << "): " << buf <<'\n';
    }
    mq_close(mq);
    mq_unlink(MQ_NAME);
    return 0;
}