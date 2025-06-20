#include <ctime>
#include <pwd.h>
#include <cstdio>
#include <iomanip>
#include <ios>
#include <iostream>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <grp.h>

//Đọc nội dung từ file được truyền qua argv[1], ghi ra stdout dùng open(), read(), write().
void ec1(std::string file_name){
    int file_fd = open(file_name.c_str(), O_RDONLY);
    if (file_fd < 0){
        perror("open fail");
        return;
    }
    unsigned long n;
    char buf[1028];
    while ((n = read(file_fd, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, n);
    }
    std::cout << std::endl;
    if(close(file_fd) == -1){
        perror("close fail");
    }
    return;
}

// EC2. So sánh hai file nhị phân
// Mở hai file, so sánh byte từng phần, in vị trí đầu tiên khác biệt.
void ec2(std::string file1, std::string file2){
    int file1_fd = open(file1.c_str(), O_RDONLY);
    int file2_fd = open(file2.c_str(), O_RDONLY);
    if (file1_fd < 0 || file2_fd < 0) {
        perror("fail to open");
        return;
    }
    std::vector<char> buf1(1024);
    std::vector<char> buf2(1024);
    long pos = 0;
    long n1, n2;
    bool dif = false;

    while ((n1 = read(file1_fd, buf1.data(), buf1.size())) > 0 && (n2 = read(file2_fd, buf2.data(), buf2.size())) > 0) {
        if (n1 > n2) {
            std::cout << "file1 is longer than file2\n";
            dif = true;
            break;
        }else if (n1 < n2) {
            std::cout << "file2 is longer than file1\n";
            dif = true;
            break;
        }
        for(int i = 0; i < n1; i++){
            if(buf1[i] != buf2[i]){
                std::cout << "Dif in byte " << pos + i 
                          << "\nfile1 = 0x" << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char) buf1[i]
                          << "\nfile2 = 0x" << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char) buf2[i] << std::endl;
                dif = true;
                break;
            }
        }
        pos+= n1;
        if (dif == true) {
            break;
        }
    }
    if (dif == false) {
        std::cout << "They are the same\n";
    }
    close(file1_fd);
    close(file2_fd);
}

// EC4. Viết chương trình clone ls -l (thu gọn)
// Dùng stat() để in mode, size, mtime.
// Thêm kiểm tra symbolic link với readlink() nếu có.
void read_file_info(std::string file_name){
    struct stat buf;
    std::vector<int> flags = {S_IRUSR, S_IWUSR, S_IXUSR,
                              S_IRGRP, S_IWGRP, S_IXGRP,
                              S_IROTH, S_IWOTH, S_IXOTH};
    std::string flag_string = "rwxrwxrwx";
    if(lstat(file_name.c_str(), &buf) == -1){
        perror("stat fail");
        return;
    }
    std::stringstream tr;
    switch (buf.st_mode & __S_IFMT) {
        case __S_IFDIR:
            tr << "d";
            break;
        case __S_IFCHR:
            tr << "c";
            break;
        case __S_IFBLK:
            tr << "b";
            break;
        case __S_IFREG:
            tr << "-";
            break;
        case __S_IFIFO:
            tr << "p";
            break;
        case __S_IFLNK:
            tr << "l";
            break;
        case __S_IFSOCK:
            tr << "s";
            break;
        default:
            tr << "?";
    }
    for (unsigned long i = 0; i < flags.size(); ++i){
        (buf.st_mode & flags[i]) ? 0 : flag_string[i] = '-';
    }

    tr  << flag_string << " " << std::setw(2) << buf.st_nlink << " " 
        << std::setw(5) << getpwuid(buf.st_uid)->pw_name << " " 
        << std::setw(5) << getgrgid(buf.st_gid)->gr_name << " " 
        << std::setw(5) << buf.st_size << " "
        << ctime(&buf.st_mtim.tv_sec) << " " 
        << file_name;
        
    if (S_ISLNK(buf.st_mode)) {
        char target[1024];
        int n = 0;
        n = readlink(file_name.c_str(), target, sizeof(target) - 1);
        target[n] = '\0';
        tr << " -> " << target;
    }

    std::cout << tr.str() << std::endl;
    return;
}

void ec4(){
    DIR* dir = opendir("./");
    if (dir == nullptr) {
        perror("opendir fail");
        return;
    }
    dirent* entry;
    std::vector<std::string> files;
    while ((entry = readdir(dir)) != nullptr) {
        files.push_back(entry->d_name);
    }
    closedir(dir);
    for (std::string &i: files){
        if(i == "." || i == ".."){
            continue;
        }
        read_file_info(i);
    }
    return;
}

// EC5. In thông tin /proc/self/maps
// Đọc file này và giải thích mối liên hệ với vùng nhớ process.
void ec5(){
    int map_fd = open("/proc/self/maps",O_RDONLY);
    if(map_fd  < 0){
        perror("open fail");
        return;
    }
    char buf[256];
    int n = 0;
    while ((n = read(map_fd, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, n);
    }
    if (n == -1){
        perror("read fail");
    }
    close(map_fd);
}

int main(int argc, char* argv[]){
    std::vector<std::string> v_argv;
    if (argc > 1){
        for (int i = 1; i < argc; i++){
            v_argv.push_back(std::string(argv[i]));
        }
    }
#ifdef EC1
    ec1(v_argv[0]);
#endif

#ifdef EC2
    if(argc == 3){
        ec2(v_argv[0], v_argv[1]);
    }
#endif

#ifdef EC4
    ec4();
#endif

#ifdef EC5
    ec5();
#endif
}