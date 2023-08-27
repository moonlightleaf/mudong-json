#include <FileReadStream.hpp>
#include <FileWriteStream.hpp>
#include <Writer.hpp>
#include <Reader.hpp>
#include <noncopyable.hpp>
#include <Exception.hpp>

#include <iostream>

using namespace mudong;

template<typename Handler>
class AddOne: json::noncopyable {
public:
    bool Null()                { return handler_.Null(); }
    bool Bool(bool b)          { return handler_.Bool(b); }
    bool Int32(int32_t i32)    { return handler_.Int32(i32 + 1); } // add one
    bool Int64(int64_t i64)    { return handler_.Int64(i64 + 1); } // add one
    bool Double(double d)      { return handler_.Double(d + 1); }  // add one
    bool String(std::string_view s) { return handler_.String(s); }
    bool StartObject()         { return handler_.StartObject(); }
    bool Key(std::string_view s)    { return handler_.Key(s); }
    bool EndObject()           { return handler_.EndObject(); }
    bool StartArray()          { return handler_.StartArray(); }
    bool EndArray()            { return handler_.EndArray(); }

    explicit AddOne(Handler& handler): handler_(handler) { }

private:
    Handler& handler_;
};

int main() {
    json::FileReadStream is(stdin);
    json::FileWriteStream os(stdout);
    json::Writer writer(os);
    AddOne addOne(writer);

    json::ParseError err = json::Reader::parse(is, addOne);
    if (err != json::ParseError::PARSE_OK) {
        std::cerr << json::parseErrorStr(err) << std::endl;
        exit(1);
    }
}

