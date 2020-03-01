/*
 * TypeMap.hpp
 *
 *  Created on: Mar 1, 2020
 *      Author: vsol
 */

#ifndef SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_TYPEMAP_HPP_
#define SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_TYPEMAP_HPP_
#include <string_view>

namespace ll_log::details{


template<class Type, class RetType = Type>
struct MapType{
    using type = Type;
    using ret_type = RetType;
};

using cstring = const char*;

#define DECL_TYPE_MAP(Type, RetType)    \
template<> struct MapType<Type>{        \
    using type = Type;                  \
    using ret_type = RetType;           \
};                                      \
using MapOf_##Type = MapType<Type>

DECL_TYPE_MAP(cstring, std::string_view);


}



#endif /* SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_TYPEMAP_HPP_ */
