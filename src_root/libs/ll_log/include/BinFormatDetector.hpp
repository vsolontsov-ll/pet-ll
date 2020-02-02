/*
 * BinFormatDetector.hpp
 *
 *  Created on: Feb 2, 2020
 *      Author: vsol
 */
#include <array>
#include <type_traits>
#include <string>
#include <string_view>

namespace ll_log::details{


enum FmtTypes {
    T_UINT              //! Unsigned integral (size is specified in size field, zero size means dummy (stub))
    , T_SINT            //! Signed integral
    , T_FLT             //! Floating point
    , T_STR             //! Sequence of chars. Either fixed size (up to 32) or variable size (prepended with size).
                        //! Size value in size bits is reduced by 1. If one wants to log char[1], he can use just char

    , T_MAX
};

template<size_t s>
constexpr size_t log2(){
    static_assert(s > 0);
    if constexpr(s == 1u)
        return 0;
    else if constexpr(s > 1u){
        static_assert((s & 1u) == 0);
        return 1 + log2<(s >> 1u)>();
    }
}

struct BinFmtItemFields{
    static constexpr size_t c_maxShortStrSize = (5u << 1u);

    char type:3;
    char size:5;
};

struct BinFmtItem{
    union {
        BinFmtItemFields flds;
        char val;
    };
};

//using BinFmtItem = unsigned char;
template<class T>
std::enable_if_t<std::is_integral_v<T>, char> getItemBinFmt(T)
{
    constexpr BinFmtItem res{{
       std::is_signed_v<T> ? T_SINT : T_UINT
       , log2<sizeof(T)>()
    }};
    return res.val;
}

template<class T>
std::enable_if_t<std::is_floating_point_v<T>, char> getItemBinFmt(T)
{
    constexpr BinFmtItem res{{
       T_FLT
       , log2<sizeof(T)>()
    }};
    return res.val;
}

template<size_t size>
std::enable_if_t<(size <= 32 && size > 1), char> getItemBinFmt(const char(&)[size]){
    constexpr BinFmtItem res{{
       T_STR
       , (size - 1)
    }};
    return res.val;
}

template<class T>
std::enable_if_t<std::is_convertible_v<T, const char*>, char> getItemFormat(T&&){
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    using BaseT = std::remove_cv_t<std::remove_extent_t<U>>;
    if constexpr(std::is_array_v<U> && std::extent_v<U> <= BinFmtItemFields::c_maxShortStrSize
         && (std::is_same_v<BaseT, char>
            || std::is_same_v<BaseT, unsigned char>
            || std::is_same_v<BaseT, signed char>)){
        constexpr BinFmtItem res{{
           T_STR
           , (std::extent_v<U> - 1)
        }};
    }
    constexpr BinFmtItem res{{
       T_STR
       , 0
    }};
    return res.val;
}

char getItemBinFmt(const std::string&){
    constexpr BinFmtItem res{{
       T_STR
       , 0
    }};
    return res.val;
}

char getItemBinFmt(const std::string_view&){
    constexpr BinFmtItem res{{
       T_STR
       , 0
    }};
    return res.val;
}

char getItemBinFmt(DummyArg){
    constexpr BinFmtItem res{{
       T_UINT
       , 0
    }};
    return res.val;
}

template<class ...T>
std::string getBinFormat(T&& ...p){
    //using namespace ll_log::details::getItemBinFmt;
    return (std::string{} + ... + getItemBinFmt(std::forward<T>(p)));
}


} // End of namespace ll_log::details


