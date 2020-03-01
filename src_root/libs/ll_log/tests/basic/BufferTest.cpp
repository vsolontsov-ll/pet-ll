/*
 * BufferTest.cpp
 *
 *  Created on: Feb 9, 2020
 *      Author: vsol
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

#include <details/Buffer.hpp>

using Storage = std::array<char, 1024>;



TEST_CASE("Buffer Encode", "") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};
    //ll_log::details::PackedCodec codec;
    constexpr char c{'\1'};
    constexpr short i16{2};
    constexpr int i32{3};
    constexpr long i64{4};
    constexpr float f32{0.5f};
    constexpr double f64{0.6};

    ll_log::details::PackedCodec codec;
    codec.store(buf, c, i16, i32, i64, f32, f64);
    CHECK(buf.offset() == (1 + 2 + 4 + 8 + 4 + 8));

    ll_log::details::BasicBuffer<true> rdBuf{buf.data(), buf.offset(), 0u};
    CHECK(c == codec.read<char>(rdBuf));
    CHECK(i16 == codec.read<short>(rdBuf));
    CHECK(i32 == codec.read<int>(rdBuf));
    CHECK(i64 == codec.read<long>(rdBuf));
    CHECK(rdBuf.offset() == (1 + 2 + 4 + 8));
    CHECK(f32 == Approx{codec.read<float>(rdBuf)});
    CHECK(f64 == Approx{codec.read<double>(rdBuf)});
    CHECK(availableSize(rdBuf) == 0);
}

TEST_CASE("Buffer Encode Str", "") {
    Storage s;
    ll_log::details::BasicBuffer<> buf{&s.front(), s.size(), 0u};
    const char* strVal{"123456"};

    ll_log::details::PackedCodec codec;
    codec.store(buf, 'c', 1, strVal, 2l);
    CHECK(buf.offset() == (1 + 4 + (2 + 6) + 8));

    ll_log::details::BasicBuffer<true> rdBuf{buf.data(), buf.offset(), 0u};
    CHECK('c' == codec.read<char>(rdBuf));
    CHECK(1 == codec.read<int>(rdBuf));
    CHECK(strVal == (codec.read<const char*>(rdBuf)));
    CHECK(2l == codec.read<long>(rdBuf));
    CHECK(availableSize(rdBuf) == 0);
}
