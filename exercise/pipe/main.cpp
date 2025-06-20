#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <vector>


//Mô phỏng lại ls -l

// int main(){
//     int pipe_fd[2];
//     if (pipe(pipe_fd) == -1){
//         perror("pipe fail");
//     }
//     int pid1 = fork();
//     if(pid1 < 0){
//         perror("fork fail");
//     }else if (pid1 == 0) {
//         if(close(pipe_fd[0]) == -1){
//             perror("close fail");
//             exit(1);
//         }
//         if(dup2(pipe_fd[1], STDOUT_FILENO) == -1){
//             perror("dup2 fail");
//             exit(1);
//         }
//         if(close(pipe_fd[1]) == -1){
//             perror("close fail");
//             exit(1);
//         }
//         char* args[] = {(char*)"ls", (char*)"-l", nullptr};
//         execvp(args[0], args);
//         perror("exec fail");
//         exit(1);
//     }

//     if(close(pipe_fd[1]) == -1){
//         perror("close fail");
//     }
//     char buf[64];
//     long n = 0;
//     while ((n = read(pipe_fd[0], buf, sizeof(buf))) > 0) {
//         std::cout.write(buf, n);
//     }
//     if(n == -1){
//         perror("read fail");
//     }
//     if(close(pipe_fd[0]) == -1){
//         perror("close fail");
//     }
//     waitpid(pid1, nullptr, 0);
//     return 0;
// }

// Mô phỏng ps -e -o comm | sort | uniq -c | sort -nr

std::vector<int> CHILD_PROCESS_PID;
std::vector<int*> PIPES;
int spawn(const char* cmd[], int in_fd, int out_fd) {
    int pid = fork();
    if(pid < 0){
        perror("fork fail");
        return -1;
    }
    else if (pid == 0){
        for (int* i : PIPES){
            if(i[0] != in_fd){
                close(i[0]);
            }
            if(i[1] != out_fd){
                close(i[1]);
            }
        }
        if (in_fd != STDIN_FILENO && in_fd != -1) dup2(in_fd, STDIN_FILENO);
        if (out_fd != STDOUT_FILENO && out_fd != -1) dup2(out_fd, STDOUT_FILENO);
        execvp(cmd[0], const_cast<char**>(cmd));
        _exit(127);
    }
    CHILD_PROCESS_PID.push_back(pid);
    return pid;
}

void create_pipe(int* pipe_fd){
    if(pipe2(pipe_fd,O_CLOEXEC) == -1){
        perror("pipe fail");
        return;
    }
    PIPES.push_back(pipe_fd);
}

void kill_process(){
    for (int &i : CHILD_PROCESS_PID){
        kill(i, SIGKILL);
    }
}

int main(){
    int pipe_fd[3][2];
    for(int* i : pipe_fd){
        create_pipe(i);
    }
    const char* cmd[4][5] = {{(char*)"ps", (char*)"-e", (char*)"-o", (char*)"comm", nullptr},
                         {(char*)"sort", nullptr, nullptr, nullptr, nullptr},
                         {(char*)"uniq", (char*)"-c", nullptr, nullptr, nullptr}, 
                         {(char*)"sort", (char*)"-nr", nullptr, nullptr, nullptr}};
    spawn(cmd[0], -1, pipe_fd[0][1]);
    close(pipe_fd[0][1]);
    spawn(cmd[1], pipe_fd[0][0], pipe_fd[1][1]);
    close(pipe_fd[0][0]);
    close(pipe_fd[1][1]);
    spawn(cmd[2], pipe_fd[1][0], pipe_fd[2][1]);
    close(pipe_fd[1][0]);
    close(pipe_fd[2][1]);
    spawn(cmd[3], pipe_fd[2][0], -1);
    close(pipe_fd[2][0]);
    

    for (int &i : CHILD_PROCESS_PID){
        waitpid(i, nullptr, 0);
    }
    return 0;
}