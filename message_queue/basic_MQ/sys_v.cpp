#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <unistd.h>
#include <iostream>

struct my_msgbuf{
    long m_type;
    char m_text[100];
};


int main(){
    key_t key = ftok("program_file", 65);
    int msgid = msgget(key, IPC_CREAT|0666);

    if (fork() == 0){
        my_msgbuf msg{1, "Hello from child\0"};
        msgsnd(msgid, &msg, sizeof(msg.m_text), 0);
        std::cout << "Child send message" << std::endl;
    }else{
        sleep(1);
        my_msgbuf msg;
        msgrcv(msgid, &msg, sizeof(msg.m_text), 1, 0);
        std::cout << "parent received message: " << msg.m_text << std::endl;
        msgctl(msgid, IPC_RMID, nullptr);
    }
}