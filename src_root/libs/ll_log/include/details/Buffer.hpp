/*
 * Buffer.hpp
 *
 *  Created on: Feb 29, 2020
 *      Author: vsol
 */

#ifndef SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_BUFFER_HPP_
#define SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_BUFFER_HPP_
#include <cstdint>
#include <type_traits>
#include <utility>
#include <limits>

#include "MetaProgHelpers.hpp"
#include "TypeMap.hpp"


namespace ll_log::details{
using std::size_t;

template<bool isConst = false>
struct BasicBuffer{
    using value_type = std::conditional_t<isConst, const char, char>;
    using pointer_type = value_type*;
    using const_pointer_type = std::add_const_t<value_type>*;

    pointer_type data_{nullptr};
    size_t capacity_{0};
    size_t offset_{0};

    constexpr pointer_type data() noexcept {    return data_;   }
    constexpr const_pointer_type data() const noexcept {    return data_;   }
    constexpr size_t shift(size_t incr) noexcept {   return (offset_ += incr);   }

    constexpr size_t capacity() const noexcept {  return capacity_;   }
    constexpr bool hasSpace(size_t l) const noexcept{   return offset_ + l <= capacity_;    }
    //constexpr size_t size() const noexcept {  return size_ - offset_; }
    constexpr size_t offset() const noexcept {    return offset_; }
};

template<class Buffer>
inline auto bytePtr(Buffer& b) noexcept -> typename Buffer::pointer_type{
    return b.data() + b.offset();

}

template<class Buffer>
inline size_t availableSize(Buffer& b) noexcept{
    return b.capacity() - b.offset();

}

template<class, class = void> struct IsBuffer : std::false_type{};
template<class T> struct IsBuffer<T
    , std::enable_if_t<
        std::is_same_v<char, std::remove_const_t<std::remove_reference_t<decltype(*std::declval<T&>().data())>>>
        && std::is_same_v<const char&, decltype(*std::declval<const T&>().data())>
        && std::is_same<size_t, decltype(std::declval<const T&>().capacity())>::value
        && std::is_same<size_t, decltype(std::declval<const T&>().offset())>::value
        && std::is_same<size_t, decltype(std::declval<T&>().shift(0))>::value
    >
> : std::true_type{};

template<class, class = void> struct IsWritableBuffer : std::false_type{};
template<class T> struct IsWritableBuffer<T
    , std::enable_if_t<IsBuffer<T>::value
        && !std::is_const_v<decltype(*std::declval<const T&>().data())>
    >
> : std::true_type{};

static_assert(IsBuffer<BasicBuffer<true>>::value);
static_assert(IsBuffer<BasicBuffer<false>>::value);
static_assert(!IsBuffer<int>::value);

struct BasicWriter{
    void writeBytes(void* dest, const void* src, size_t size) noexcept {
        memcpy(dest, src, size);
    }

    constexpr void done() noexcept {}
};


template<class WriterT = BasicWriter>
class PackedCodecT{
    using Writer = WriterT;
    Writer writer_;

    template<class T>
    static constexpr size_t sizeOf(const T&, size_t& offset) noexcept{
        offset += sizeof(T);
        return sizeof(T);
    }
    static size_t sizeOf(const char* s, size_t& offset) noexcept{
        const auto l = strlen(s) + sizeof(ushort);
        offset += l;
        return l;
    }

    template<class Buffer, class T>
    void storeOne(Buffer& buf, const T& v, size_t = 0) noexcept{
        constexpr auto s = sizeof(T);
        writer_.writeBytes(bytePtr(buf), &v, s);
        buf.shift(s);
    }

    template<class Buffer>
    void storeOne(Buffer& buf, const char* s, size_t len) noexcept{
        const auto strLen = static_cast<uint16_t>(len - sizeof(uint16_t));
        storeOne(buf, strLen);
        writer_.writeBytes(bytePtr(buf), s, strLen);
        buf.shift(strLen);
    }

    template<class Buffer, class TID>
    typename TID::ret_type readOne(Buffer& buf, TID) noexcept{
        using type = typename TID::type;
        constexpr auto s = sizeof(type);
        type r{};
        if(buf.hasSpace(s)){
            writer_.writeBytes(&r, bytePtr(buf), s);
            buf.shift(s);
        }
        return typename TID::ret_type{r};
    }

    template<class Buffer>
    std::string_view readOne(Buffer& buf, MapOf_cstring) noexcept{
        const auto l = readOne(buf, MapType<uint16_t>{});
        std::string_view ret{bytePtr(buf), l};
        buf.shift(l);
        return ret;
    }

public:
    template<class Buffer, class ...Args>
    void store(Buffer& buf, const Args& ...args){
        static_assert(IsWritableBuffer<Buffer>::value);

        size_t offset{0};
        const size_t lens[sizeof...(args)] = {
            sizeOf(args, offset)...
        };
        if(buf.hasSpace(offset)){
            size_t i{0};
            (storeOne(buf, args, lens[i++]), ...);
        }
        writer_.done();
    }

    template<class Type, class Buffer>
    auto read(Buffer& b) -> typename MapType<Type>::ret_type{
        static_assert(IsBuffer<Buffer>::value);
        return readOne(b, MapType<Type>{});
    }

};

using PackedCodec = PackedCodecT<>;

}   // End of namespace ll_log::details

#endif /* SRC_ROOT_LIBS_LL_LOG_INCLUDE_DETAILS_BUFFER_HPP_ */

