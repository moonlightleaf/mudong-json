//
// Created by mudong on 23-05-20.
//

#pragma once

#include <string_view>
#include <type_traits>

#include "Value.hpp"
#include "Reader.hpp"
#include "FileReadStream.hpp"
#include "StringReadStream.hpp"

namespace mudong {

namespace json {

class Document: public Value {
public:
    ParseError parse(const std::string_view& json) {
        StringReadStream is(json);
        return parseStream(is);
    }

    ParseError parse(const char* json, size_t len) {
        return parse(std::string_view(json, len));
    }

    template <typename ReadStream, 
              typename = std::enable_if_t<std::is_same<ReadStream, StringReadStream>::value ||
                                          std::is_same<ReadStream, FileReadStream>::value>>
    ParseError parseStream(ReadStream& is) {
        return Reader::parse(is, *this);
    }

public:
    bool Null() {
        addValue(Value(ValueType::TYPE_NULL));
        return true;
    }
    bool Bool(bool b) {
        addValue(Value(b));
        return true;
    }
    bool Int32(int32_t i32) {
        addValue(Value(i32));
        return true;
    }
    bool Int64(int64_t i64) {
        addValue(Value(i64));
        return true;
    }
    bool Double(double d) {
        addValue(Value(d));
        return true;
    }
    bool String(const std::string_view& s) {
        addValue(Value(s));
        return true;
    }
    bool StartObject() {
        auto value = addValue(Value(ValueType::TYPE_OBJECT));
        stack_.emplace_back(value);
        return true;
    }
    bool Key(std::string_view s) {
        addValue(Value(s));
        return true;
    }
    bool EndObject() {
        assert(!stack_.empty());
        assert(stack_.back().type() == ValueType::TYPE_OBJECT);
        stack_.pop_back();
        return true;
    }
    bool StartArray() {
        auto value = addValue(Value(ValueType::TYPE_ARRAY));
        stack_.emplace_back(value);
        return true;
    }
    bool EndArray() {
        assert(!stack_.empty());
        assert(stack_.back().type() == ValueType::TYPE_ARRAY);
        stack_.pop_back();
        return true;
    }

private:
    Value* addValue(Value&& value) {
        ValueType type = value.getType();
        (void)type;
        if (seeValue_)
            assert(!stack_.empty() && "root not singular");
        else { 
            assert(type_ == ValueType::TYPE_NULL);
            seeValue_ = true;
            type_ = value.type_;
            a_ = value.a_;
            value.type_ = ValueType::TYPE_NULL;
            value.a_ = nullptr;
            return this;
        }


        auto& top = stack_.back();
        if (top.type() == ValueType::TYPE_ARRAY) {
            top.value->addValue(std::move(value));
            top.valueCount++;
            return const_cast<Value*>(top.lastValue());
        }
        else {
            assert(top.type() == ValueType::TYPE_OBJECT);

            if (top.valueCount % 2 == 0) {
                assert(type == ValueType::TYPE_STRING && "miss quotation mark");
                key_ = std::move(value);
                top.valueCount++;
                return &key_;
            }
            else {
                top.value->addMember(std::move(key_), std::move(value));
                top.valueCount++;
                return const_cast<Value*>(top.lastValue());
            }
        }
    }

private:
    struct Level {
        explicit Level(Value* value_):
                value(value_), valueCount(0)
        {}

        ValueType type() const {
            return value->getType();
        }

        const Value* lastValue() {
            if (type() == ValueType::TYPE_ARRAY) {
                return &value->getArray().back();
            } else {
                return &value->getObject().back().value;
            }
        }

        Value* value;
        int valueCount;
    };

private:
    std::vector<Level> stack_;
    Value key_;
    bool seeValue_ = false;
};

} // namespace json

} // namespace mudong