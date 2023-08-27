//
// Created by mudong on 23-05-03
//

#pragma once

#include <cstdio>
#include <string_view>

#include "noncopyable.hpp"

namespace mudong {

namespace json {

class FileWriteStream: noncopyable {
public:
    explicit FileWriteStream(FILE* output) : output_(output) { }
    ~FileWriteStream() { fflush(output_); }

    void put(char c)                      { fputc(c, output_); }
    void put(const std::string_view& str) { fprintf(output_, "%.*s", static_cast<int>(str.size()), str.data()); }

private:
    FILE* output_;
};

} // namespace json

} // namespace mudong

/*
使用带缓冲的写，FileWriteStream对象析构时，调用fflush清空缓冲区，防止缓冲区中暂存的数据丢失
*/