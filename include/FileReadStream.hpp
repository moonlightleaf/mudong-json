//
// Created by mudong on 23-05-03.
//

#pragma once

#include <vector>
#include <cstdio>
#include <cassert>

#include "noncopyable.hpp"

namespace mudong {

namespace json {

class FileReadStream: noncopyable {
public:
    using ConstIterator = std::vector<char>::const_iterator;

public:
    explicit FileReadStream(FILE* input) { 
        char buf[65536];
        while (true) {
            size_t n = fread(buf, 1, sizeof(buf), input);
            if (n == 0) break;
            buffer_.insert(buffer_.end(), buf, buf + n);
        }

        iter_ = buffer_.cbegin();
    }

    bool          hasNext     () const { return iter_ != buffer_.cend(); }
    char          peek        () const { return hasNext() ? *iter_ : '\0'; }
    ConstIterator getConstIter() const { return iter_; }
    char          next        ()       { return hasNext() ? *iter_++ : '\0'; }
    void          assertNext  (char c) { assert(peek() == c); next(); }

private:
    std::vector<char> buffer_;
    ConstIterator     iter_;
};

} // namespace json

} // namespace mudong