#include <benchmark/benchmark.h>

#include <Document.hpp>
#include <FileReadStream.hpp>
#include <StringWriteStream.hpp>
#include <Writer.hpp>

template <class ...ExtraArgs>
void BM_read(benchmark::State &s, ExtraArgs &&... extra_args)
{
    for (auto _: s) {
        FILE *input = fopen(extra_args..., "r");
        if (input == nullptr)
            exit(1);
        json::FileReadStream is(input);
        benchmark::DoNotOptimize(is);
        fclose(input);
    }
}

template <class ...ExtraArgs>
void BM_read_parse(benchmark::State &s, ExtraArgs &&... extra_args)
{
    for (auto _: s) {
        FILE *input = fopen(extra_args..., "r");
        if (input == nullptr)
            exit(1);
        json::Document doc;
        json::FileReadStream is(input);
        fclose(input);
        if (doc.parseStream(is) != json::ParseError::PARSE_OK) {
            exit(1);
        }
    }
}

template <class ...ExtraArgs>
void BM_read_parse_write(benchmark::State &s, ExtraArgs&&... extra_args)
{
    for (auto _: s) {
        FILE *input = fopen(extra_args..., "r");
        if (input == nullptr)
            exit(1);
        json::Document doc;
        json::FileReadStream is(input);
        fclose(input);
        if (doc.parseStream(is) != json::ParseError::PARSE_OK) {
            exit(1);
        }
        json::StringWriteStream os;
        json::Writer writer(os);
        doc.writeTo(writer);
        std::string_view ret = os.getStringView();
        benchmark::DoNotOptimize(ret);
    }
}

BENCHMARK_CAPTURE(BM_read, taobao, "/workspace/mudong-json/bench/taobao/cart.json")->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_read_parse, taobao, "/workspace/mudong-json/bench/taobao/cart.json")->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_read_parse_write, taobao, "/workspace/mudong-json/bench/taobao/cart.json")->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();