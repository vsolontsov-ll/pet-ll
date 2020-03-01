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



#endif /* SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGGER_HPP_ */
