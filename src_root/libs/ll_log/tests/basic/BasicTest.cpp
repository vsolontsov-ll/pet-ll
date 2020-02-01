/*
 * BasicTest.cpp
 *
 *  Created on: Jan 31, 2020
 *      Author: vsol
 */
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <iostream>

struct A
{
	[[gnu::noinline]] A()
	{
		std::cout << "test" << std::endl;
	}
};

static A a;
static A a1;

using LogInitFunc = void(*)();

TEST_CASE("Init array", "[initarray]") {
	static int tag;
	{
		struct A
		{
			static void init()
			{
				tag = 100;
			}
		};
		[[gnu::section(".init_array"), maybe_unused, gnu::used]] static LogInitFunc entry = A::init;
	}
	REQUIRE(tag == 100);
}
