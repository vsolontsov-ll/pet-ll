/*
 * StatLib.cpp
 *
 *  Created on: Feb 2, 2020
 *      Author: vsol
 */
#include <LogPoint.hpp>

namespace test_log_init_stat{
    [[gnu::used]] void func(){
        LOG_POINT_INIT(1, "stat::func", "");

        const auto lambda = [](){
            asm("" ::: "memory");
            LOG_POINT_INIT(1, "stat::lambda", "");
        };
        // Call lambda to prevent it from throwing away.
        // Anyway we don't call the func()
        lambda();
    }
} // End of namespace test_log_init



