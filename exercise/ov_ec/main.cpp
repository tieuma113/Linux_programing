#include <algorithm>
#include <cstdio>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <pwd.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <wait.h>

#define READ_PIPE 0
#define WRITE_PIPE 1

// Đọc nội dung từ file được truyền qua argv[1], ghi ra stdout dùng open(),
// read(), write().
void ec1(std::string file_name) {
  int file_fd = open(file_name.c_str(), O_RDONLY);
  if (file_fd < 0) {
    perror("open fail");
    return;
  }
  unsigned long n;
  char buf[1028];
  while ((n = read(file_fd, buf, sizeof(buf))) > 0) {
    write(STDOUT_FILENO, buf, n);
  }
  std::cout << std::endl;
  if (close(file_fd) == -1) {
    perror("close fail");
  }
  return;
}

// EC2. So sánh hai file nhị phân
// Mở hai file, so sánh byte từng phần, in vị trí đầu tiên khác biệt.
void ec2(std::string file1, std::string file2) {
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

  while ((n1 = read(file1_fd, buf1.data(), buf1.size())) > 0 &&
         (n2 = read(file2_fd, buf2.data(), buf2.size())) > 0) {
    if (n1 > n2) {
      std::cout << "file1 is longer than file2\n";
      dif = true;
      break;
    } else if (n1 < n2) {
      std::cout << "file2 is longer than file1\n";
      dif = true;
      break;
    }
    for (int i = 0; i < n1; i++) {
      if (buf1[i] != buf2[i]) {
        std::cout << "Dif in byte " << pos + i << "\nfile1 = 0x" << std::hex
                  << std::setw(2) << std::setfill('0')
                  << (unsigned int)(unsigned char)buf1[i] << "\nfile2 = 0x"
                  << std::hex << std::setw(2) << std::setfill('0')
                  << (unsigned int)(unsigned char)buf2[i] << std::endl;
        dif = true;
        break;
      }
    }
    pos += n1;
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
void read_file_info(std::string file_name) {
  struct stat buf;
  std::vector<int> flags = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                            S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
  std::string flag_string = "rwxrwxrwx";
  if (lstat(file_name.c_str(), &buf) == -1) {
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
  for (unsigned long i = 0; i < flags.size(); ++i) {
    (buf.st_mode & flags[i]) ? 0 : flag_string[i] = '-';
  }

  tr << flag_string << " " << std::setw(2) << buf.st_nlink << " "
     << std::setw(5) << getpwuid(buf.st_uid)->pw_name << " " << std::setw(5)
     << getgrgid(buf.st_gid)->gr_name << " " << std::setw(5) << buf.st_size
     << " " << ctime(&buf.st_mtim.tv_sec) << " " << file_name;

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

void ec4() {
  DIR *dir = opendir("./");
  if (dir == nullptr) {
    perror("opendir fail");
    return;
  }
  dirent *entry;
  std::vector<std::string> files;
  while ((entry = readdir(dir)) != nullptr) {
    files.push_back(entry->d_name);
  }
  closedir(dir);
  for (std::string &i : files) {
    if (i == "." || i == "..") {
      continue;
    }
    read_file_info(i);
  }
  return;
}

// EC5. In thông tin /proc/self/maps
// Đọc file này và giải thích mối liên hệ với vùng nhớ process.
void ec5() {
  int map_fd = open("/proc/self/maps", O_RDONLY);
  if (map_fd < 0) {
    perror("open fail");
    return;
  }
  char buf[256];
  int n = 0;
  while ((n = read(map_fd, buf, sizeof(buf))) > 0) {
    write(STDOUT_FILENO, buf, n);
  }
  if (n == -1) {
    perror("read fail");
  }
  close(map_fd);
}

// EC_EPOLL_1: Dùng pipe() và epoll để giám sát 2 pipe con gửi message cho cha
// Yêu cầu:
// Process cha tạo 2 pipe và 2 process con
// Con gửi message "Hi from child 1" và "Hi from child 2" sau sleep
// Cha dùng epoll_wait() để in ra đúng thông điệp theo thứ tự ai gửi trước
class Resource_guard {
private:
  std::vector<int> fd_list;
  std::vector<int> pid_list;

public:
  Resource_guard() = default;

  Resource_guard(const std::vector<int> &fd_list,
                 const std::vector<int> &pid_list)
      : fd_list(fd_list), pid_list(pid_list) {}

  ~Resource_guard() {
    std::cout << "[DESTRUCTOR] pid " << getpid() << " called destructor\n";
    cleanup();
  }

  void cleanup() {
    std::cout << "[" << getpid() << "]Release resource\n";
    if (fd_list.size() > 0) {
      for (auto &i : fd_list) {
        std::cout << "[" << getpid() << "] CLose fd " << i << "\n";
        if (i != -1 && close(i) == -1) {
          perror("close fail");
        }
        std::cout << "[PID " << getpid() << "] remove fd " << i << std::endl;
      }
    }

    if (pid_list.size() > 0) {
      for (auto &i : pid_list) {
        std::cout << "release pid " << i << std::endl;
        if (waitpid(i, nullptr, 0) == -1) {
          perror("waitpid fail");
        }
        std::cout << "[PID " << getpid() << "] remove pid " << i << std::endl;
      }
    }
  }

  bool add_fd(const int &fd) {
    for (auto &i : fd_list) {
      if (i == fd) {
        std::cerr << "FD already exist\n";
        return false;
      }
    }
    fd_list.push_back(fd);
    return true;
  }

  bool add_pid(const int &pid) {
    for (auto &i : pid_list) {
      if (i == pid) {
        std::cerr << "PID already exist\n";
        return false;
      }
    }
    pid_list.push_back(pid);
    return true;
  }

  void remove_fd(const int &fd) {
    auto it = std::find(fd_list.begin(), fd_list.end(), fd);
    if (it != fd_list.end()) {
      if (*it != -1 && close(*it) == -1) {
        perror("close fail");
      }
      fd_list.erase(it);
      std::cout << "[PID << " << getpid() << "] erase fd " << fd << std::endl;
    }
  }

  void remove_pid(const int &pid) {
    auto it = std::find(pid_list.begin(), pid_list.end(), pid);
    if (it != pid_list.end()) {
      if (*it != -1 && waitpid(*it, nullptr, 0) == -1) {
        perror("waitpid fail");
      }
      pid_list.erase(it);
      std::cout << "[PID << " << getpid() << "] wait pid " << pid << std::endl;
    }
  }

  const std::vector<int> &get_fd_list() const { return fd_list; }
  const std::vector<int> &get_pid_list() const { return pid_list; }
  void set_fd_list(const std::vector<int> &fd_list) { this->fd_list = fd_list; }
  void set_pid_list(const std::vector<int> &pid_list) {
    this->pid_list = pid_list;
  }
};

void ec6() {
  std::cout << "[PARENT] pid = " << getpid() << std::endl;
  Resource_guard rg = Resource_guard();
  int pipe_fd1[2];
  if (pipe2(pipe_fd1, O_CLOEXEC) == -1) {
    perror("pipe2 fail");
    return;
  }
  rg.add_fd(pipe_fd1[READ_PIPE]);
  rg.add_fd(pipe_fd1[WRITE_PIPE]);
  __pid_t pid1 = fork();

  if (pid1 < 0) {
    perror("fork fail");
    return;
  } else if (pid1 == 0) {
    std::cout << "[CHILD1] pid = " << getpid() << std::endl;
    rg.remove_fd(pipe_fd1[READ_PIPE]);
    std::string message = "Hello from child1\n";
    int n = write(pipe_fd1[WRITE_PIPE], message.c_str(), message.size());
    if (n == -1) {
      perror("write fail");
    }
    return;
  }
  rg.remove_fd(pipe_fd1[WRITE_PIPE]);
  rg.add_pid(pid1);

  int pipe_fd2[2];
  if (pipe2(pipe_fd2, O_CLOEXEC) == -1) {
    perror("pipe2 fail");
    return;
  }
  rg.add_fd(pipe_fd2[READ_PIPE]);
  rg.add_fd(pipe_fd2[WRITE_PIPE]);

  __pid_t pid2 = fork();
  if (pid2 < 0) {
    perror("fork fail");
    return;
  } else if (pid2 == 0) {
    std::cout << "[CHILD2] pid = " << getpid() << std::endl;
    rg.remove_fd(pipe_fd1[READ_PIPE]);
    rg.remove_fd(pipe_fd2[READ_PIPE]);
    std::string message = "Hello from child2\n";
    int n = write(pipe_fd2[WRITE_PIPE], message.c_str(), message.size());
    if (n == -1) {
      perror("write fail");
    }
    return;
  }
  rg.remove_fd(pipe_fd2[WRITE_PIPE]);
  rg.add_pid(pid2);

  int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  rg.add_fd(epoll_fd);
  std::vector<int> fds = {pipe_fd1[READ_PIPE], pipe_fd2[READ_PIPE]};
  for (int &i : fds) {
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = i;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, i, &ev) == -1) {
      perror("epoll_ctl fail");
    }
  }

  int handle = 0;
  int num_handle = 2;
  epoll_event evs[10];
  while (handle < num_handle) {
    int n = epoll_wait(epoll_fd, evs, 10, 1000);
    for (int i = 0; i < n; i++) {
      int triggered_fd = evs[i].data.fd;

      if (evs[i].events & EPOLLIN) {
        char buf[100];
        int read_char = 0;
        while ((read_char = read(triggered_fd, buf, sizeof(buf))) > 0) {
          std::cout.write(buf, read_char);
        }
        handle++;
      }
    }
  }
  return;
}

int main(int argc, char *argv[]) {
  std::vector<std::string> v_argv;
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      v_argv.push_back(std::string(argv[i]));
    }
  }
#ifdef EC1
  ec1(v_argv[0]);
#endif

#ifdef EC2
  if (argc == 3) {
    ec2(v_argv[0], v_argv[1]);
  }
#endif

#ifdef EC4
  ec4();
#endif

#ifdef EC5
  ec5();
#endif

#ifdef EC6
  ec6();
#endif
  return 1;
}