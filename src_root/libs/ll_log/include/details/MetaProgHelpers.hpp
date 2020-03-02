/*
 * MetaProgHelpers.hpp
 *
 *  Created on: Mar 1, 2020
 *      Author: Vladimir Solontsov
 */

#ifndef SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_METAPROGHELPERS_HPP_
#define SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_METAPROGHELPERS_HPP_

#include <type_traits>

namespace ll_log::details{

//! Basic helper to debug static_asserts
template<class A, class B>
struct AssertSame{
    static_assert(std::is_same_v<A, B>);
    static constexpr auto value = std::is_same_v<A, B>;
};

template<class T>
struct ArgTypeT{
    using type = typename std::conditional_t<std::is_trivially_copyable_v<T> && (sizeof(T) <= sizeof(void*))
        , T
        , const T&
    >;
};

template<class T>
using ArgType = typename ArgTypeT<T>::type;

}



#endif /* SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_METAPROGHELPERS_HPP_ */
