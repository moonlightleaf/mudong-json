//
// Created by mudong on 23-05-02.
//

#pragma once

#include <string>
#include <vector>

#include "noncopyable.hpp"

namespace mudong {

namespace json {

class StringWriteStream: noncopyable {
public:
    void put(char c)                      { buffer_.push_back(c); }
    void put(const std::string_view& str) { buffer_.insert(buffer_.end(), str.begin(), str.end()); }

    std::string_view getStringView() const { return std::string_view(&*buffer_.begin(), buffer_.size()); }
    std::string      getString    () const { return std::string(buffer_.begin(), buffer_.end()); }

private:
    std::vector<char> buffer_;
};

} // namespace json

} // namespace mudong