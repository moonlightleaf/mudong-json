// 
// Created by mudong on 23-05-04.
//

// 设计思路类似于QT中的QVariant，c++17中提供的variant

#pragma once

#include <vector>
#include <atomic>
#include <cassert>
#include <string>
#include <type_traits>
#include <algorithm>

#include "noncopyable.hpp"

namespace mudong {

namespace json {

enum class ValueType {
    TYPE_NULL,
    TYPE_BOOL,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT
};

struct Member;
class Document;

class Value {
    friend Document;
public:
    using MemberIterator      = std::vector<Member>::iterator;
    using ConstMemberIterator = std::vector<Member>::const_iterator;

public:
    explicit Value(ValueType = ValueType::TYPE_NULL);
    explicit Value(bool b)                    : type_(ValueType::TYPE_BOOL)  , b_(b)     { }
    explicit Value(int32_t i32)               : type_(ValueType::TYPE_INT32) , i32_(i32) { }
    explicit Value(int64_t i64)               : type_(ValueType::TYPE_INT64) , i64_(i64) { }
    explicit Value(double d)                  : type_(ValueType::TYPE_DOUBLE), d_(d)     { }
    explicit Value(std::string_view s) : type_(ValueType::TYPE_STRING), s_(new StringWithRefCount(s.begin(), s.end())) { }
    Value(const char* s, size_t len)          : Value(std::string_view(s, len)) { }
    Value(const Value&);
    Value(Value&&);

    Value& operator=(const Value&);
    Value& operator=(Value&&);

    ~Value();

public:
    ValueType getType() const { return type_; }
    size_t    getSize() const;

    bool isNull  () const { return type_ == ValueType::TYPE_NULL; }
    bool isBool  () const { return type_ == ValueType::TYPE_BOOL; }
    bool isInt32 () const { return type_ == ValueType::TYPE_INT32; }
    bool isInt64 () const { return type_ == ValueType::TYPE_INT64 || type_ == ValueType::TYPE_INT32; }
    bool isDouble() const { return type_ == ValueType::TYPE_DOUBLE; }
    bool isString() const { return type_ == ValueType::TYPE_STRING; }
    bool isArray () const { return type_ == ValueType::TYPE_ARRAY; }
    bool isObject() const { return type_ == ValueType::TYPE_OBJECT; }

    bool        getBool  () const { assert(type_ == ValueType::TYPE_BOOL);   return b_; }
    int32_t     getInt32 () const { assert(type_ == ValueType::TYPE_INT32);  return i32_; }
    double      getDouble() const { assert(type_ == ValueType::TYPE_DOUBLE); return d_; }
    const auto& getArray () const { assert(type_ == ValueType::TYPE_ARRAY);  return a_->data; }
    const auto& getObject() const { assert(type_ == ValueType::TYPE_OBJECT); return o_->data; }
    std::string getString() const { return std::string(getStringView()); }

    int64_t getInt64() const {
        assert(type_ == ValueType::TYPE_INT64 || type_ == ValueType::TYPE_INT32);
        return type_ == ValueType::TYPE_INT64 ? i64_ : i32_;
    }
    std::string_view getStringView() const {
        assert(type_ == ValueType::TYPE_STRING);
        return std::string_view(&*s_->data.begin(), s_->data.size());
    }

    Value& setNull  ()                   { this->~Value(); return *new (this) Value(ValueType::TYPE_NULL); } // placement new
    Value& setBool  (bool b)             { this->~Value(); return *new (this) Value(b); }
    Value& setInt32 (int32_t i32)        { this->~Value(); return *new (this) Value(i32); }
    Value& setInt64 (int64_t i64)        { this->~Value(); return *new (this) Value(i64); }
    Value& setDouble(double d)           { this->~Value(); return *new (this) Value(d); }
    Value& setArray ()                   { this->~Value(); return *new (this) Value(ValueType::TYPE_ARRAY); }
    Value& setObject()                   { this->~Value(); return *new (this) Value(ValueType::TYPE_OBJECT); }
    Value& setString(std::string_view s) { this->~Value(); return *new (this) Value(s); }

    Value&       operator[](const std::string_view&);       // non-const obj invokes this.
    const Value& operator[](const std::string_view&) const; // const obj invokes this.

    MemberIterator      beginMember ()       { assert(type_ == ValueType::TYPE_OBJECT); return o_->data.begin(); }
    ConstMemberIterator cbeginMember() const { assert(type_ == ValueType::TYPE_OBJECT); return o_->data.cbegin(); }
    MemberIterator      endMember   ()       { assert(type_ == ValueType::TYPE_OBJECT); return o_->data.end(); }
    ConstMemberIterator cendMember  () const { assert(type_ == ValueType::TYPE_OBJECT); return o_->data.cend(); }
    ConstMemberIterator beginMember () const { return cbeginMember(); } // const obj invokes this.
    ConstMemberIterator endMember   () const { return cendMember(); }   // const obj invokes this.
    MemberIterator      findMember  (const std::string_view&);
    ConstMemberIterator findMember  (const std::string_view&) const; // const obj invokes this.

    template <typename V>
    Value& addMember(const char* k, V&& v) { return addMember(Value(k), Value(std::forward<V>(v))); }

    Value& addMember(Value&&, Value&&);

    template <typename T>
    Value& addValue(T&& value) {
        assert(type_ == ValueType::TYPE_ARRAY);
        a_->data.emplace_back(std::forward<T>(value));
        return a_->data.back();
    }

    Value&       operator[](size_t i)       { assert(type_ == ValueType::TYPE_ARRAY); return a_->data[i]; }
    const Value& operator[](size_t i) const { assert(type_ == ValueType::TYPE_ARRAY); return a_->data[i]; }

    template <typename Handler>
    bool writeTo(Handler&) const;

private:
    template <typename T, typename = std::enable_if_t<std::is_same_v<T, std::vector<char>>  || 
                                                      std::is_same_v<T, std::vector<Value>> ||
                                                      std::is_same_v<T, std::vector<Member>>>>
    struct AddRefCount {
        template <typename... Args>
        AddRefCount(Args&&... args) : refCount(1), data(std::forward<Args>(args)...) { }
        ~AddRefCount() { assert(refCount == 0); }

        int incrAndGet() { assert(refCount > 0); return ++refCount; }
        int decrAndGet() { assert(refCount > 0); return --refCount; }

        std::atomic_int refCount;
        T data;
    };

    using StringWithRefCount = AddRefCount<std::vector<char>>;
    using ArrayWithRefCount  = AddRefCount<std::vector<Value>>;
    using ObjectWithRefCount = AddRefCount<std::vector<Member>>;

    ValueType type_;
    
    union {
        bool                b_;
        int32_t             i32_;
        int64_t             i64_;
        double              d_;
        StringWithRefCount* s_;
        ArrayWithRefCount*  a_;
        ObjectWithRefCount* o_;
    };

}; // End of class Value definition

struct Member {
    Member(Value&& k, Value&& v)                 : key(std::move(k)), value(std::move(v)) { }
    Member(const std::string_view& k, Value&& v) : key(k)           , value(std::move(v)) { }

    Value key;
    Value value;
};

// definition of class Value's member func

Value::Value(ValueType type) :
    type_(type),
    s_(nullptr) {
    switch (type_) {
        case ValueType::TYPE_NULL:
        case ValueType::TYPE_BOOL:
        case ValueType::TYPE_INT32:
        case ValueType::TYPE_INT64:
        case ValueType::TYPE_DOUBLE:                                break;
        case ValueType::TYPE_STRING: s_ = new StringWithRefCount(); break;
        case ValueType::TYPE_ARRAY:  a_ = new ArrayWithRefCount();  break;
        case ValueType::TYPE_OBJECT: o_ = new ObjectWithRefCount(); break;
        default: assert(false && "bad type when Value constuct.");
    }
}

Value::Value(const Value& rhs) :
    type_(rhs.type_),
    s_(rhs.s_) {
    switch (type_) {
        case ValueType::TYPE_NULL:
        case ValueType::TYPE_BOOL:
        case ValueType::TYPE_INT32:
        case ValueType::TYPE_INT64:
        case ValueType::TYPE_DOUBLE:                   break;
        case ValueType::TYPE_STRING: s_->incrAndGet(); break;
        case ValueType::TYPE_ARRAY:  a_->incrAndGet(); break;
        case ValueType::TYPE_OBJECT: o_->incrAndGet(); break;
        default: assert(false && "bad type when Value copy-constuct.");
    }
}

Value::Value(Value&& rhs) :
    type_(rhs.type_),
    s_(rhs.s_) {
    rhs.type_ = ValueType::TYPE_NULL;
    rhs.a_ = nullptr; // 移动拷贝构造，使原右值失效，故当前对象无须考虑引用计数增加 
}

Value& Value::operator=(const Value& rhs) {
    if (this == &rhs) return *this; // copy itself

    this->~Value();
    type_ = rhs.type_;
    s_ = rhs.s_;
    switch (type_) {
        case ValueType::TYPE_NULL:
        case ValueType::TYPE_BOOL:
        case ValueType::TYPE_INT32:
        case ValueType::TYPE_INT64:
        case ValueType::TYPE_DOUBLE:                   break;
        case ValueType::TYPE_STRING: s_->incrAndGet(); break;
        case ValueType::TYPE_ARRAY:  a_->incrAndGet(); break;
        case ValueType::TYPE_OBJECT: o_->incrAndGet(); break;
        default: assert(false && "bad type when Value copy.");
    }
    return *this;
}

Value& Value::operator=(Value&& rhs) {
    if (this == &rhs) return *this;

    this->~Value();
    type_ = rhs.type_;
    s_ = rhs.s_;
    rhs.type_ = ValueType::TYPE_NULL;
    rhs.s_ = nullptr;
    return *this;
}

Value::~Value() {
    switch (type_) {
        case ValueType::TYPE_NULL:
        case ValueType::TYPE_BOOL:
        case ValueType::TYPE_INT32:
        case ValueType::TYPE_INT64:
        case ValueType::TYPE_DOUBLE: break;
        case ValueType::TYPE_STRING:
            if (s_->decrAndGet() == 0) delete s_;
            break;
        case ValueType::TYPE_ARRAY:
            if (a_->decrAndGet() == 0) delete a_;
            break;
        case ValueType::TYPE_OBJECT:
            if (o_->decrAndGet() == 0) delete o_;
            break;
        default: assert(false && "bad type when Value copy.");
    }
}

size_t Value::getSize() const {
    if (type_ == ValueType::TYPE_ARRAY) return a_->data.size();
    else if (type_ == ValueType::TYPE_OBJECT) return o_->data.size();
    return 1;
}

Value& Value::operator[](const std::string_view& key) {
    assert(type_ == ValueType::TYPE_OBJECT);

    auto iter = findMember(key);
    if (iter != o_->data.end()) return iter->value;

    assert(false);
    static Value fake(ValueType::TYPE_NULL);
    return fake;
}

const Value& Value::operator[](const std::string_view& key) const {
    return const_cast<Value&>(*this)[key];
}

Value::MemberIterator Value::findMember(const std::string_view& key) {
    assert(type_ == ValueType::TYPE_OBJECT);
    return std::find_if(o_->data.begin(), o_->data.end(), 
                        [key](const Member& m) { return m.key.getStringView() == key; });
}

Value::ConstMemberIterator Value::findMember(const std::string_view& key) const {
    return const_cast<Value&>(*this).findMember(key);
}

Value& Value::addMember(Value&& key, Value&& value) {
    assert(type_ == ValueType::TYPE_OBJECT);
    assert(key.type_ == ValueType::TYPE_STRING);
    assert(findMember(key.getStringView()) == endMember());
    o_->data.emplace_back(std::move(key), std::move(value));
    return o_->data.back().value;
}

#define CALL(expr) do { if (!(expr)) return false; } while(false)
// https://zhuanlan.zhihu.com/p/22460835

template <typename Handler>
inline bool Value::writeTo(Handler& handler) const {
    switch (type_) {
        case ValueType::TYPE_NULL:
            CALL(handler.Null());
            break;
        case ValueType::TYPE_BOOL:
            CALL(handler.Bool(b_));
            break;
        case ValueType::TYPE_INT32:
            CALL(handler.Int32(i32_));
            break;
        case ValueType::TYPE_INT64:
            CALL(handler.Int64(i64_));
            break;
        case ValueType::TYPE_DOUBLE:
            CALL(handler.Double(d_));
            break;
        case ValueType::TYPE_STRING:
            CALL(handler.String(getStringView()));
            break;
        case ValueType::TYPE_ARRAY:
            CALL(handler.StartArray());
            for (auto& val : getArray()) CALL(val.writeTo(handler));
            CALL(handler.EndArray());
            break;
        case ValueType::TYPE_OBJECT:
            CALL(handler.StartObject());
            for (auto& member : getObject()) {
                handler.Key(member.key.getStringView());
                CALL(member.value.writeTo(handler));
            }
            CALL(handler.EndObject());
            break;
        default:
            assert(false && "bad type when writeTo.");
    }
    return true;
}

#undef CALL


} // namespace json

} // namespace mudong