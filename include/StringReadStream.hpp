//
// Created by mudong on 23-05-02.
//

#pragma once

#include <string>
#include <cassert>

#include "noncopyable.hpp"

namespace mudong {

namespace json {

class StringReadStream: noncopyable {
public:
    using ConstIterator = std::string_view::const_iterator;
    // std::string::const_iterator;

public:
    explicit StringReadStream(const std::string_view& json) : json_(json), iter_(json.begin()) { }
    
    bool          hasNext     () const { return iter_ != json_.end(); }
    char          peek        () const { return hasNext() ? *iter_ : '\0'; }
    ConstIterator getConstIter() const { return iter_; }
    char          next        ()       { return hasNext() ? *iter_++ : '\0'; }
    void          assertNext  (char c) { assert(peek() == c); next(); }

private:
    const std::string_view json_;
    ConstIterator     iter_; // Point to the next character to be processed
};

} // namespace json

} // namespace mudong

/*
json_采用string类型存储而非string_view，虽然增加了空间占用和复制成本，但规避了StringReadStream(string())
这样的风险，无需保证传递给构造函数的参数的生命周期的结束必须晚于StringReadStream对象，更安全。形参
const string_view&可以接住const/non-const string_view/string类型的参数。

以下即为一个存在use after free问题的demo：
string_view s;
void ss(string_view sv) {
    s = sv;
}
int main() {
    ss(string("abc"));
    cout << s << endl;
}

*/