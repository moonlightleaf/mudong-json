#include <FileWriteStream.hpp>
#include <Writer.hpp>

#include <iostream>

int main() {
    json::FileWriteStream os(stdout);
    json::Writer writer(os);

    writer.StartObject();
    writer.Key("B");
    writer.StartArray();
    writer.String("ByteDance");
    writer.String("BaiDu");
    writer.EndArray();
    writer.Key("A");
    writer.String("Alibaba");
    writer.Key("T");
    writer.String("Tencent");
    writer.EndObject();
}