/*
 * BasicTest.cpp
 *
 *  Created on: Jan 31, 2020
 *      Author: vsol
 */
#include <catch2/catch.hpp>
#include <iostream>
#include <LogPoint.hpp>

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
        LOG_POINT_INIT(1, "func");

        const auto lambda = [](){
            asm("" ::: "memory");
            LOG_POINT_INIT(1, "lambda");
        };
        // Call lambda to prevent it from throwing away.
        // Anyway we don't call the func()
        lambda();
    }

    template<class T>
    void templateFunc(){
        LOG_POINT_INIT(1, "templateFunc");

        const auto lambda = [](){
            asm("" ::: "memory");
            LOG_POINT_INIT(1, "lambda in template");
        };
        lambda();
    }

    const ll_log::internal::LogPoint* findPointByFmt(const std::string& fmt) {
        for(const auto& p: ll_log::internal::getPoints()) {
            std::cout << p.id_ << ", " << p.file_ << ":" << p.line_ << std::endl;
            if(fmt == p.format_)
                return &p;
        }
        return nullptr;
    }

    template [[gnu::used]] void templateFunc<char>();

} // End of namespace test_log_init

TEST_CASE("Log init", ""){
    LOG_POINT_INIT(1, "1");
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
