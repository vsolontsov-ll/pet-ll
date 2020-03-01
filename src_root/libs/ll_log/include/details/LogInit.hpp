/*
 * LogInit.hpp
 *
 *  Created on: Feb 1, 2020
 *      Author: vsol
 */

#ifndef SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_LOGINIT_HPP_
#define SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_LOGINIT_HPP_

namespace ll_log::details{

using LogInitFunc = void(*)();

#define INIT_ARRAY(func)\
    asm(                                        \
        ".pushsection .init_array, \"aw\"\n"    \
        ".align %c1\n"                          \
        ".quad %c0\n"                           \
        ".popsection"                           \
        :: "p"(func), "n"(sizeof(void*)) :      \
    )

#define LOG_POINT_INIT(level, fmt, binFmt)                              \
    static ll_log::details::PointId ptid;                               \
    struct PtInit{                                                      \
        [[gnu::used]] static void init(){                               \
            ll_log::details::addPoint(ptid, (level), (fmt), (binFmt),   \
                __FILE__, __LINE__, __PRETTY_FUNCTION__);               \
        }                                                               \
    };                                                                  \
    INIT_ARRAY(PtInit::init);

//! Marker the end of arguments list
struct DummyArg{};

}   // End of namespace ll_log::details

#endif /* SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_LOGINIT_HPP_ */
