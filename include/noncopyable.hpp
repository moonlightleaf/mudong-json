//
// Created by mudong on 23-05-01.
//

#pragma once

namespace mudong {

namespace json {

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

} // namespace json

} // namespace mudong
