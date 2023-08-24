//
// Created by mudong on 23-05-19.
//

#pragma once

#include <type_traits>
#include <cmath>
#include <string>

#include "Exception.hpp"
#include "Value.hpp"
#include "FileReadStream.hpp"
#include "StringReadStream.hpp"

namespace json {

class Reader: noncopyable {
public:
    template <typename ReadStream, typename Handler,
              typename = std::enable_if_t<(std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value)>>
    static ParseError parse(ReadStream& is, Handler& handler) {
        try {
            parseWhiteSpace(is);
            parseValue(is, handler);
            parseWhiteSpace(is);
            if (is.hasNext()) throw Exception(ParseError::PARSE_ROOT_NOT_SINGULAR);
            return ParseError::PARSE_OK;
        }
        catch (Exception& e) {
            return e.err();
        }
    }

private:
#define CALL(expr) if (!(expr)) throw Exception(ParseError::PARSE_USER_STOPPED)

    template <typename ReadStream, 
            typename = std::enable_if_t<std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value>>
    static unsigned parseHex4(ReadStream& is) {
        unsigned u = 0;
        for (int i = 0; i < 4; ++i) {
            u <<= 4;
            switch (char ch = is.next()) {
                case '0'...'9': u |= ch - '0'; break;
                case 'a'...'f': u |= ch - 'a' + 10; break;
                case 'A'...'F': u |= ch - 'A' + 10; break;
                default: throw Exception(ParseError::PARSE_BAD_UNICODE_HEX);
            }
        }
        return u;
    }

    template <typename ReadStream, 
            typename = std::enable_if_t<std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value>>
    static void parseWhiteSpace(ReadStream& is) {
        while (is.hasNext()) {
            char ch = is.peek();
            if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') is.next();
            else break;
        }
    }

    template <typename ReadStream, typename Handler,
            typename = std::enable_if_t<(std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value)>>
    static void parseLiteral(ReadStream& is, Handler& handler, const char* literal, ValueType type) {
        char ch = *literal;

        is.assertNext(*literal++);
        while (*literal != '\0' && *literal == is.peek()) {
            literal++;
            is.next();
        }
        if (*literal == '\0') {
            switch (type) {
            case ValueType::TYPE_NULL:
                CALL(handler.Null());
                return;
            case ValueType::TYPE_BOOL:
                CALL(handler.Bool(ch == 't'));
                return;
            case ValueType::TYPE_DOUBLE:
                CALL(handler.Double(ch == 'N' ? NAN : INFINITY));
                return;
            default:
                assert(false && "bad type");
            }
        }
        throw Exception(ParseError::PARSE_BAD_VALUE);
    }

    template <typename ReadStream, typename Handler,
            typename = std::enable_if_t<(std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value)>>
    static void parseNumber(ReadStream& is, Handler& handler) {
        if (is.peek() == 'N') {
            parseLiteral(is, handler, "NaN", ValueType::TYPE_DOUBLE);
            return;
        }
        else if (is.peek() == 'I') {
            parseLiteral(is, handler, "Infinity", ValueType::TYPE_DOUBLE);
            return;
        }

        auto start = is.getConstIter();

        if (is.peek() == '-') is.next();

        if (is.peek() == '0') {
            is.next();
            if (isDigit(is.peek())) throw Exception(ParseError::PARSE_BAD_VALUE);
        }
        else if (isDigit19(is.peek())) {
            is.next();
            while (isDigit(is.peek())) is.next();
        }
        else throw Exception(ParseError::PARSE_BAD_VALUE);

        auto expectType = ValueType::TYPE_NULL;

        if (is.peek() == '.') {
            expectType = ValueType::TYPE_DOUBLE;
            is.next();
            if (!isDigit(is.peek())) throw Exception(ParseError::PARSE_BAD_VALUE);
            while (isDigit(is.peek())) is.next();
        }

        if (is.peek() == 'e' || is.peek() == 'E') {
            expectType = ValueType::TYPE_DOUBLE;
            is.next();
            if (is.peek() == '+' || is.peek() == '-') is.next();
            if (!isDigit(is.peek())) throw Exception(ParseError::PARSE_BAD_VALUE);
            is.next();
            while (isDigit(is.peek())) is.next();
        }

        //int32 or int64
        if (is.peek() == 'i') {
            is.next();
            if (expectType == ValueType::TYPE_DOUBLE)
                throw Exception(ParseError::PARSE_BAD_VALUE);
            switch (is.next()) {
                case '3':
                    if (is.next() != '2')
                        throw Exception(ParseError::PARSE_BAD_VALUE);
                    expectType = ValueType::TYPE_INT32;
                    break;
                case '6':
                    if (is.next() != '4')
                        throw Exception(ParseError::PARSE_BAD_VALUE);
                    expectType = ValueType::TYPE_INT64;
                    break;
                default:
                    throw Exception(ParseError::PARSE_BAD_VALUE);
            }
        }

        auto end = is.getConstIter();
        if (start == end) throw Exception(ParseError::PARSE_BAD_VALUE);

        try {
            //
            // std::stod() && std::stoi() are bad ideas,
            // because new string buffer is needed
            //
            // std::stoi(const string& __str, size_t* __idx = 0, int __base = 10) { 
            //     return __gnu_cxx::__stoa<long, int>(&std::strtol, "stoi", __str.c_str(),
			// 		__idx, __base); 
            // }
            std::size_t idx;
            if (expectType == ValueType::TYPE_DOUBLE) {
                double d = __gnu_cxx::__stoa(&std::strtod, "stod", &*start, &idx);
                assert(start + idx == end);
                CALL(handler.Double(d));
            }

            else {
                int64_t i64 = __gnu_cxx::__stoa(&std::strtol, "stol", &*start, &idx, 10);
                if (expectType == ValueType::TYPE_INT64)
                {
                    CALL(handler.Int64(i64));
                }
                else if (expectType == ValueType::TYPE_INT32)
                {
                    if (i64 > std::numeric_limits<int32_t>::max() ||
                        i64 < std::numeric_limits<int32_t>::min()) {
                        throw std::out_of_range("int32_t overflow");
                    }
                    CALL(handler.Int32(static_cast<int32_t>(i64)));
                }
                else if (i64 <= std::numeric_limits<int32_t>::max() &&
                         i64 >= std::numeric_limits<int32_t>::min()) {
                    CALL(handler.Int32(static_cast<int32_t>(i64)));
                }
                else
                {
                    CALL(handler.Int64(i64));
                }
            }
        }
        catch (std::out_of_range &e) {
            throw Exception(ParseError::PARSE_NUMBER_TOO_BIG);
        }
    }

    template <typename ReadStream, typename Handler,
            typename = std::enable_if_t<(std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value)>>
    static void parseString(ReadStream& is, Handler& handler, bool isKey) {
        is.assertNext('"');
        std::string buffer;
        while (is.hasNext()) {
            char ch = is.next();
            switch (ch) {
                case '"':
                    if (isKey) {CALL(handler.Key(std::move(buffer)));}
                    else {CALL(handler.String(std::move(buffer)));}
                    return;
                case '\x01'...'\x1f':
                    throw Exception(ParseError::PARSE_BAD_STRING_CHAR);
                case '\\':
                    switch (is.next()) {
                        case '"':  buffer.push_back('"');  break;
                        case '\\': buffer.push_back('\\'); break;
                        case '/':  buffer.push_back('/');  break;
                        case 'b':  buffer.push_back('\b'); break;
                        case 'f':  buffer.push_back('\f'); break;
                        case 'n':  buffer.push_back('\n'); break;
                        case 'r':  buffer.push_back('\r'); break;
                        case 't':  buffer.push_back('\t'); break;
                        case 'u': {
                            // unicode
                            unsigned u = parseHex4(is);
                            if (u >= 0xD800 && u <= 0xDBFF) {
                                if (is.next() != '\\')
                                    throw Exception(ParseError::PARSE_BAD_UNICODE_SURROGATE);
                                if (is.next() != 'u')
                                    throw Exception(ParseError::PARSE_BAD_UNICODE_SURROGATE);
                                unsigned u2 = parseHex4(is);
                                if (u2 >= 0xDC00 && u2 <= 0xDFFF)
                                    u = 0x10000 + (u - 0xD800) * 0x400 + (u2 - 0xDC00);
                                else
                                    throw Exception(ParseError::PARSE_BAD_UNICODE_SURROGATE);
                            }
                            encodeUtf8(buffer, u);
                            break;
                        }
                        default: throw Exception(ParseError::PARSE_BAD_STRING_ESCAPE);
                    }
                    break;
                default: buffer.push_back(ch);
            }
        }
        throw Exception(ParseError::PARSE_MISS_QUOTATION_MARK);
    }

    template <typename ReadStream, typename Handler,
            typename = std::enable_if_t<(std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value)>>
    static void parseArray(ReadStream& is, Handler& handler) {
        CALL(handler.StartArray());

        is.assertNext('[');
        parseWhiteSpace(is);

        if (is.peek() == ']') {
            is.next();
            CALL(handler.EndArray());
            return;
        }

        while (true) {
            parseValue(is, handler);
            parseWhiteSpace(is);
            switch (is.next()) {
                case ',':
                    parseWhiteSpace(is);
                    break;
                case ']':
                    CALL(handler.EndArray());
                    return;
                default:
                    throw Exception(ParseError::PARSE_MISS_COMMA_OR_SQUARE_BRACKET);
            }
        }
    }

    template <typename ReadStream, typename Handler,
            typename = std::enable_if_t<(std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value)>>
    static void parseObject(ReadStream& is, Handler& handler) {
        CALL(handler.StartObject());

        is.assertNext('{');
        parseWhiteSpace(is);
        if (is.peek() == '}') {
            is.next();
            CALL(handler.EndObject());
            return;
        }

        while (true) {

            if (is.peek() != '"')
                throw Exception(ParseError::PARSE_MISS_KEY);

            parseString(is, handler, true);

            // parse ':'
            parseWhiteSpace(is);
            if (is.next() != ':')
                throw Exception(ParseError::PARSE_MISS_COLON);
            parseWhiteSpace(is);

            // go on
            parseValue(is, handler);
            parseWhiteSpace(is);
            switch (is.next()) {
                case ',':
                    parseWhiteSpace(is);
                    break;
                case '}':
                    CALL(handler.EndObject());
                    return;
                default:
                    throw Exception(ParseError::PARSE_MISS_COMMA_OR_CURLY_BRACKET);
            }
        }
    }
#undef CALL

    template <typename ReadStream, typename Handler,
            typename = std::enable_if_t<(std::is_same<ReadStream, FileReadStream>::value ||
                                        std::is_same<ReadStream, StringReadStream>::value)>>
    static void parseValue(ReadStream& is, Handler& handler) {
        if (!is.hasNext())
            throw Exception(ParseError::PARSE_EXPECT_VALUE);

        switch (is.peek()) {
            case 'n': return parseLiteral(is, handler, "null", ValueType::TYPE_NULL);
            case 't': return parseLiteral(is, handler, "true", ValueType::TYPE_BOOL);
            case 'f': return parseLiteral(is, handler, "false", ValueType::TYPE_BOOL);
            case '"': return parseString(is, handler, false);
            case '[': return parseArray(is, handler);
            case '{': return parseObject(is, handler);
            default:  return parseNumber(is, handler);
        }
    }

private:
    static bool isDigit(char ch)
    { return ch >= '0' && ch <= '9'; }
    static bool isDigit19(char ch)
    { return ch >= '1' && ch <= '9'; }
    static void encodeUtf8(std::string& buffer, unsigned u);
};

} // namespace json

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
//ignore the type conversion warning
void json::Reader::encodeUtf8(std::string& buffer, unsigned u)
{
    // unicode stuff from Milo's tutorial
    switch (u) {
        case 0x00 ... 0x7F:
            buffer.push_back(u & 0xFF);
            break;
        case 0x080 ... 0x7FF:
            buffer.push_back(0xC0 | ((u >> 6) & 0xFF));
            buffer.push_back(0x80 | (u & 0x3F));
            break;
        case 0x0800 ... 0xFFFF:
            buffer.push_back(0xE0 | ((u >> 12) & 0xFF));
            buffer.push_back(0x80 | ((u >> 6) & 0x3F));
            buffer.push_back(0x80 | (u & 0x3F));
            break;
        case 0x010000 ... 0x10FFFF:
            buffer.push_back(0xF0 | ((u >> 18) & 0xFF));
            buffer.push_back(0x80 | ((u >> 12) & 0x3F));
            buffer.push_back(0x80 | ((u >> 6) & 0x3F));
            buffer.push_back(0x80 | (u & 0x3F));
            break;
        default: assert(false && "out of range");
    }
}
#pragma GCC diagnostic pop
