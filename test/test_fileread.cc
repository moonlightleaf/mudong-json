#include <gtest/gtest.h>
#include <iostream>

#include <Writer.hpp>
#include <StringWriteStream.hpp>
#include <Document.hpp>

using namespace json;

inline void TEST_FILEREAD(const std::string& json) {
    Document doc;
    ParseError err = doc.parse(json);
    EXPECT_EQ(err, json::ParseError::PARSE_OK);
    StringWriteStream os;
    Writer writer(os);
    doc.writeTo(writer);
    EXPECT_EQ(json, os.getStringView());
}

TEST(FileRelative, read) {
    FILE *input = fopen("/workspace/mudong-json/bench/taobao/cart.json", "r");
    if (input == nullptr)
        exit(1);
    json::FileReadStream is(input);
    assert(is.peek() == '{' && "wrong peek");
    fclose(input);
    EXPECT_TRUE(true);
}

TEST(FileRelative, read_parse) {
    FILE *input = fopen("/workspace/mudong-json/bench/taobao/cart.json", "r");
    if (input == nullptr)
        exit(1);
    json::FileReadStream is(input);
    fclose(input);
    json::Document doc;
    EXPECT_EQ(doc.parseStream(is), json::ParseError::PARSE_OK);
}

TEST(FileRelative, read_parse_write) {
    FILE *input = fopen("/workspace/mudong-json/bench/taobao/cart.json", "r");
    if (input == nullptr)
        exit(1);
    json::FileReadStream is(input);
    fclose(input);
    json::Document doc;
    doc.parseStream(is);

    json::StringWriteStream os;
    json::Writer writer(os);
    doc.writeTo(writer);
    bool b = os.getStringView().size() > 0 && os.getStringView()[0] == '{';
    EXPECT_TRUE(b);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}