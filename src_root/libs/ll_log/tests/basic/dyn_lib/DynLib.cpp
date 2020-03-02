/*
 * DynLib.cpp
 *
 *  Created on: Feb 2, 2020
 *      Author: Vladimir Solontsov
 */
#include <LogPoint.hpp>

namespace test_log_init_dyn{
    [[gnu::used]] void func(){
        LOG_POINT_INIT(1, "dyn::func", "");

        const auto lambda = [](){
            asm("" ::: "memory");
            LOG_POINT_INIT(1, "dyn::lambda", "");
        };
        // Call lambda to prevent it from throwing away.
        // Anyway we don't call the func()
        lambda();
    }
} // End of namespace test_log_init



