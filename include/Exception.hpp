//
// Created by mudong on 23-05-11.
//

#pragma once

#include <exception>
#include <cassert>

namespace mudong {

namespace json {

#define ERROR_MAP(XX) \
    XX(OK, "ok") \
    XX(ROOT_NOT_SINGULAR, "root not singular") \
    XX(BAD_VALUE, "bad value") \
    XX(EXPECT_VALUE, "expect value") \
    XX(NUMBER_TOO_BIG, "number too big") \
    XX(BAD_STRING_CHAR, "bad character") \
    XX(BAD_STRING_ESCAPE, "bad escape") \
    XX(BAD_UNICODE_HEX, "bad unicode hex") \
    XX(BAD_UNICODE_SURROGATE, "bad unicode surrogate") \
    XX(MISS_QUOTATION_MARK, "miss quotation mark") \
    XX(MISS_COMMA_OR_SQUARE_BRACKET, "miss comma or square bracket") \
    XX(MISS_KEY, "miss key") \
    XX(MISS_COLON, "miss colon") \
    XX(MISS_COMMA_OR_CURLY_BRACKET, "miss comma or curly bracket") \
    XX(USER_STOPPED, "user stopped parse")

enum class ParseError: unsigned {
#define GEN_ERRNO(e, s) PARSE_##e,
    ERROR_MAP(GEN_ERRNO)
#undef GEN_ERRNO
};

inline const char* parseErrorStr(ParseError err) {
    static const char* tab[] = {
#define GEN_STRERR(e, n) n,
            ERROR_MAP(GEN_STRERR)
#undef GEN_STRERR
    };

    assert(unsigned(err) >= 0 && unsigned(err) < sizeof(tab) / sizeof(tab[0]));
    return tab[unsigned(err)];
}

class Exception: public std::exception {
public:
    explicit Exception(ParseError err): err_(err) { }
    const char* errStr() const { return parseErrorStr(err_); }
    ParseError err() const { return err_; }

private:
    ParseError err_;
};

#undef ERROR_MAP

} // namespace json

} // namespace mudong