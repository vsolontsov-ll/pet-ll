/*
 * BufferTest.cpp
 *
 *  Created on: Feb 9, 2020
 *      Author: Vladimir Solontsov
 *
 *
 * For storing data we can use:
 *  * _mm_stream_si32()
 *  * _mm_stream_si64()
 *  * _mm_stream_si128()
 *
 * For simplicity let's choose _mm_stream_si64().
 * Short integral types (char, short, ...) can be combined into a bigger ones.
 *
 * Algorithm:
 * 1. Deduce message size
 * 2. Check whether there's enough space
 * 3. Try to obtain a new buffer and check again.
 * 4.
 */
#include <catch2/catch.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <array>
#include <string_view>

#include <details/Buffer.hpp>

using namespace std::literals::string_view_literals;
using Storage = std::array<char, 1024>;
using Codec = ll_log::details::PackedCodec;
using ll_log::details::read;


template<class Codec, class Buffer, class ...Args>
[[gnu::noinline]] void storeNoInline(Buffer& buf, Args... args){
    //Buffer b{bytePtr(buf), availableSize(buf), 0u};
    ll_log::details::store<Codec>(buf, args...);
    //buf.shift(b.offset());
}

template<class Buffer, class ...Args>
void testStore(Buffer& buf, const Args&... args){
    storeNoInline<Codec>(buf, ll_log::details::ArgType<Args>{args}...);
}

TEST_CASE("Buffer Encode", "") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};
    constexpr char c{'\1'};
    constexpr short i16{2};
    constexpr int i32{3};
    constexpr long i64{4};
    constexpr float f32{0.5f};
    constexpr double f64{0.6};

    testStore(buf, c, i16, i32, i64, f32, f64);
    CHECK(buf.offset() == (1 + 2 + 4 + 8 + 4 + 8));

    ll_log::details::BasicBuffer<true> rdBuf{buf.data(), buf.offset(), 0u};
    CHECK(c == read<Codec, char>(rdBuf));
    CHECK(i16 == read<Codec, short>(rdBuf));
    CHECK(i32 == read<Codec, int>(rdBuf));
    CHECK(i64 == read<Codec, long>(rdBuf));
    CHECK(rdBuf.offset() == (1 + 2 + 4 + 8));
    CHECK(f32 == Approx{read<Codec, float>(rdBuf)});
    CHECK(f64 == Approx{read<Codec, double>(rdBuf)});
    CHECK(availableSize(rdBuf) == 0);
}

TEST_CASE("Buffer Encode Str", "") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};
    const char* strVal{"123456"};

    //codec.store(buf, 'c', 1, strVal, 2l);
    testStore(buf, 'c', 1, strVal, 2l);
    CHECK(buf.offset() == (1 + 4 + (2 + 6) + 8));

    ll_log::details::BasicBuffer<true> rdBuf{buf.data(), buf.offset(), 0u};
    CHECK('c' == (read<Codec, char>(rdBuf)));
    CHECK(1 == (read<Codec, int>(rdBuf)));
    CHECK(strVal == (read<Codec, const char*>(rdBuf)));
    CHECK(2l == (read<Codec, long>(rdBuf)));
    CHECK(availableSize(rdBuf) == 0);
}

TEST_CASE("Buffer Encode One", "ToDisassembly") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};
    testStore(buf, 100);
}

int i1{100};
int i2{200};

TEST_CASE("Buffer Encode Two", "ToDisassembly") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};
    testStore(buf, i1, i2);
}

TEST_CASE("Buffer Encode only cstring", "ToDisassembly") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};

    const char* strVal{"123456"};
    testStore(buf, strVal);
}

TEST_CASE("Buffer Encode only string_view", "ToDisassembly") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};

    testStore(buf, "123456"sv);
}

TEST_CASE("Buffer Encode with cstr", "ToDisassembly") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};

    const char* strVal{"123456"};
    testStore(buf, 'c', 1, strVal, 2l);
}

TEST_CASE("Buffer Encode with string_view", "ToDisassembly") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};

    testStore(buf, 'c', 1, "123456"sv, 2l);
}

