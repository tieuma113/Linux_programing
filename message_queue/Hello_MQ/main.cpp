#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <wait.h>

#ifdef SYS_V
struct my_msgbuf{
    long m_type;
    char m_text[100];
};
#endif

#ifdef POSIX
constexpr const char* MQ_NAME = "/demo_queue";
constexpr std::size_t MAX_MSG_SIZE = 128;
#endif

int main(){
    #ifdef SYS_V
    key_t key = ftok("program_file", 14);
    int msgid = msgget(key, IPC_CREAT | 0666);

    int pid = fork();
    if(pid < 0){
        perror("Fail to fork");
        return 1;
    }
    else if(pid == 0){
        my_msgbuf msg{1, "Hello"};
        msgsnd(msgid, &msg, strlen(msg.m_text) + 1, 0);
        std::cout << "Child send message\n";
        _exit(1);
    }
    sleep(1);
    my_msgbuf msg;
    int n = msgrcv(msgid, &msg, sizeof(msg.m_text), 1, IPC_NOWAIT);
    if (n == -1){
        std::cout << "PID " << getpid() << std::endl;
        perror("Fail to receive message");
    }else {
        msg.m_text[n] = '\0';
        std::cout << "PID: " << getpid() << " Parent received: " << msg.m_text << std::endl;
    }
    waitpid(pid, nullptr, 0);
    msgctl(msgid, IPC_RMID, nullptr);
    #endif

    #ifdef POSIX
    mq_attr attr{};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    int mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if(mq == -1){perror("mq_open fail"); return 1;}

    int pid = fork();
    if (pid < 0) {
        perror("fork fail");
    }
    else if (pid == 0) {
        const char* text = "Hello";
        if(mq_send(mq, text, strlen(text) + 1, 5) == -1){
            perror("mq_send fail");
        }else {
            std::cout << "[CHILD] send message\n";
        }
        mq_close(mq);
        _exit(0);
    }
    sleep(1);
    char buf[MAX_MSG_SIZE];
    unsigned int prio;
    long n = mq_receive(mq, buf, sizeof(buf), &prio);
    if ( n == -1){
        perror("mq_receive fail");
    }else {
        buf[n] = '\0';
        std::cout << "[Parent] got bytes: " << n << " prio: " << prio << " message: " << buf << std::endl;
    }
    mq_close(mq);
    mq_unlink(MQ_NAME);
    #endif

    return 0;
}