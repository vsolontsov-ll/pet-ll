/*
 * \file Logger.hpp
 *
 *  Created on: Mar 1, 2020
 *      Author: Vladimir Solontsov
 */

#ifndef SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGGER_HPP_
#define SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGGER_HPP_

#include <array>
#include <x86intrin.h>

#include <details/Buffer.hpp>

namespace ll_log {

struct DummyLogger{
    using Codec = details::PackedCodec;
    std::array<char, 4096u> storage_;
    details::BasicBuffer<false> buf_{storage_.data(), storage_.size(), 0u};

    template<size_t fmtSize, class... Args>
    void store(const void* tpid, const char(&)[fmtSize], const Args&... args){
        // Ignore format string
        details::store<Codec>(buf_, tpid, _rdtsc(), args...);
    }

    details::BasicBuffer<true> getReadBuf() const{
        return details::BasicBuffer<true>{buf_.data(), buf_.offset(), 0u};
    }
};

}  // namespace ll_log


#define LLI_ARG_FIRST(fmt, ...) (fmt)
#define LLI_GET_BIN_FMT(fmt, ...) ll_log::details::getBinFormat(__VA_ARGS__)

// TODO: For now assume there's a 'logger' object in scope
#define LL_INFO(...)                                                            \
    {                                                                           \
        LOG_POINT_INIT(                                                         \
            1 /* TODO replace with log level) */                                \
            , LLI_ARG_FIRST(__VA_ARGS__, Dummy)                                 \
            , LLI_GET_BIN_FMT(__VA_ARGS__, ll_log::details::DummyArg{})         \
        );                                                                      \
        logger.store(&ptid, __VA_ARGS__);                                       \
    } do{}while(false)


#endif /* SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGGER_HPP_ */
