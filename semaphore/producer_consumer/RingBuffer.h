struct RingBuffer{
    static constexpr int BUF_SIZE = 10;
    int buffer[BUF_SIZE] = {};
    int write_index = 0;
    int read_index = 0;
    bool is_empty();
    bool is_full();
    void push(int value);
    int pop();
};