#include "RingBuffer.h"
#include <iostream>

bool RingBuffer::is_empty(){
    return write_index == read_index;
}

bool RingBuffer::is_full(){
    return (write_index + 1) % BUF_SIZE == read_index;
}

void RingBuffer::push(int value){
    if(is_full()){
        std::cerr << "Buffer full" << std::endl;
        return;
    }
    buffer[write_index] = value;
    write_index = (write_index + 1) % BUF_SIZE;
    return;
}

int RingBuffer::pop() {
    int val = buffer[read_index];
    read_index = (read_index + 1) % BUF_SIZE;
    return val;
}