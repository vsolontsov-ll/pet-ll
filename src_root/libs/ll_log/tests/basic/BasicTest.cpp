/*
 * BasicTest.cpp
 *
 *  Created on: Jan 31, 2020
 *      Author: vsol
 */
#include <catch2/catch.hpp>
#include <iostream>

#include <LogPoint.hpp>
#include <BinFormatDetector.hpp>

#include <string>

#include "../../include/Logger.hpp"
using namespace std::literals::string_literals;

static int counter;


#define POINT(var)	\
    static int var;	\
    {   struct Point{ [[gnu::used]] static void init(){ var = ++counter;    }};\
        INIT_ARRAY(Point::init);}

template<class T>
int foo()
{
	POINT(vvv);
	return vvv;
}


TEST_CASE("Init array", "[initarray]") {
	POINT(t1);
	CHECK(t1 > 0);

	const auto t2 = foo<char>();
	const auto t3 = foo<int>();
	CHECK(t2 > 0);
	CHECK(t3 > 0);
	CHECK(t2 != t3);
	CHECK(t1 + t2 + t3 == 6);
}

namespace test_log_init{
    [[gnu::used]] void func(){
        LOG_POINT_INIT(1, "func", "");

        const auto lambda = [](){
            asm("" ::: "memory");
            LOG_POINT_INIT(1, "lambda", "");
        };
        // Call lambda to prevent it from throwing away.
        // Anyway we don't call the func()
        lambda();
    }

    template<class T>
    void templateFunc(){
        LOG_POINT_INIT(1, "templateFunc", "");

        const auto lambda = [](){
            asm("" ::: "memory");
            LOG_POINT_INIT(1, "lambda in template", "");
        };
        lambda();
    }

    const ll_log::details::LogPoint* findPointByFmt(const std::string& fmt) {
        for(const auto& p: ll_log::details::getPoints()) {
            std::cout << p.id_ << ", " << p.file_ << ":" << p.line_ << std::endl;
            if(fmt == p.format_)
                return &p;
        }
        return nullptr;
    }

    template [[gnu::used]] void templateFunc<char>();

} // End of namespace test_log_init

TEST_CASE("Log init", ""){
    LOG_POINT_INIT(1, "1", "");
    CHECK(nullptr != test_log_init::findPointByFmt("1"));
    CHECK(nullptr != test_log_init::findPointByFmt("func"));
    CHECK(nullptr != test_log_init::findPointByFmt("lambda"));
    CHECK(nullptr != test_log_init::findPointByFmt("templateFunc"));
    CHECK(nullptr != test_log_init::findPointByFmt("lambda in template"));

    CHECK(nullptr != test_log_init::findPointByFmt("stat::func"));
    CHECK(nullptr != test_log_init::findPointByFmt("stat::lambda"));

    CHECK(nullptr != test_log_init::findPointByFmt("dyn::func"));
    CHECK(nullptr != test_log_init::findPointByFmt("dyn::lambda"));
    // Doesn't meter when calling it -- just prevent from removal
    //test_log_init::templateFunc<char>();
}

template<class T>
void checkFormat(char expectation){
    T t{};
    T& rt{t};
    const T& crt{t};
    const T ct{};

    const auto fmt = ll_log::details::getBinFormat(t, rt, crt, ct);
    for(const auto c: fmt){
        CHECK(int{c} == int{expectation});
    }
}

TEST_CASE("BinFmt", ""){
    checkFormat<int>('\x11');

    const int i{};
    const char c{};
    const float f{};
    const auto fmt = ll_log::details::getBinFormat(i, c, f, "12345");

    CHECK(int(fmt[3]) == int('\x2B'));
    CHECK(fmt == "\x11\x01\x12\x2B"s);
}


TEST_CASE("Log macro. Check points", ""){
    ll_log::DummyLogger logger;
    // LL_INFO("test");
    _mm_lfence();
    const auto tsStart{_rdtsc()};

    LL_INFO("Test with bin fmt", int{1}, char{2}, "12345", double{0.1}, float{0.2f});

    _mm_lfence();
    const auto tsEnd{_rdtsc()};

    auto point{test_log_init::findPointByFmt("Test with bin fmt")};
    const auto fmt = point->decodeFmt_;

    CHECK(point->decodeFmt_ == "\x11\x01\x2B\x1A\x12\0"s);

    using ll_log::details::read;
    using Codec = decltype(logger)::Codec;

    auto rdBuf = logger.getReadBuf();
    const auto ptid = read<Codec, const void*>(rdBuf);
    CHECK(ptid == point->binId_);

    auto ts = read<Codec, uint64_t>(rdBuf);
    CHECK(ts >= tsStart);
    CHECK(ts <= tsEnd);
    std::cout << "Latency: " << (tsEnd - tsStart) << std::endl;

    CHECK(1 == read<Codec, int>(rdBuf));
    CHECK('\2' == read<Codec, char>(rdBuf));
    CHECK("12345" == (read<Codec, const char*>(rdBuf)));
    CHECK(0.1 == Approx{read<Codec, double>(rdBuf)});
    CHECK(0.2f == Approx{read<Codec, float>(rdBuf)});
}
