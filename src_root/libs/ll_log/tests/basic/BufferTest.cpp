/*
 * BufferTest.cpp
 *
 *  Created on: Feb 9, 2020
 *      Author: vsol
 *
 *
 * For storing data we can use:
 *  * _mm_stream_si32()
 *  * _mm_stream_si64()
 *  * _mm_stream_si128()
 *
 * For simplicity let's choose _mm_stream_si64().
 * Short integral types (char, short, ...) can be combined into a bigger ones.
 *
 * Algorithm:
 * 1. Deduce message size
 * 2. Check whether there's enough space
 * 3. Try to obtain a new buffer and check again.
 * 4.
 */
#include <catch2/catch.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>

//#include <utility>
//#include <string>


//namespace ll_log{
//namespace details{
//
//template<class T>
//constexpr std::enable_if_t<std::is_integral_v<T>, size_t> getSize(T&&){   return sizeof(T); }
//
//size_t getSize(const std::string& s){   return 8u + s.size();    }
//
//template<class T>
//constexpr std::enable_if_t<std::is_integral_v<T>, size_t> getAlignment(T&&){   return alignof(T); }
//
//constexpr size_t getAlignment(const std::string&){   return 8u;    }
//
//constexpr size_t sizeToStore(size_t offset){    return offset;  }
//
//template<class T0, class ...Args>
//constexpr size_t sizeToStore(size_t offset, T0&& arg, Args&& ...args){
//    constexpr auto align = getAlignment(std::forward<T0>(arg));
//
//    return sizeToStore(
//        ((offset + align - 1) & (~(align - 1))) + getSize(std::forward<T0>(arg))
//        , std::forward<Args>(args)...);
//}
//
//} // End of namespace details
//
//template<class Buffer>
//class Writer{
//    Buffer buf_;
//    uint64_t accumulator_{};
//public:
//    explicit Writer(Buffer buf) : buf_{buf}{}
//
//    template<class T>
//    void storeType(T&& t);
//
//    template<class ...Args>
//    void store(Args&& ...args){
//        const auto sizeToStore = details::sizeToStore(std::forward<Args>(args)...);
//        auto bufSize = buf_.size();
//        if(sizeToStore > bufSize)
//        {
//            // TODO: renew buffer
//            bufSize = buf_.size();
//            if(sizeToStore > bufSize)
//                return;
//        }
//        accumulator_ = 0;
//        (this->storeType(std::forward<Args>(args)), ...);
//    }
//};
//
//template<class T>
//constexpr size_t getItemBinSize(T&&){
//    return sizeof(T);
//}
//
//template<class Buffer> template<class ...Args>
//inline void LogWriter<Buffer>::store<Args...>(Args&& ...args){
//    if((getItemBinSize(std::forward<Args>(args) != 0) && ... ){
//
//    }
//}
//
//} // End of namespace ll_log

static constexpr size_t PAGE_SIZE{4096u};
static constexpr size_t CL_SIZE{64u};
static constexpr size_t PAGE_MASK{~(PAGE_SIZE - 1u)};

struct BufferInfo{
    // Assume page-aligned allocations and using lower bits to specify offset
    static constexpr size_t capacity{CL_SIZE * PAGE_SIZE};
    size_t compressedInfo_;

    void* pointer() const{
        return reinterpret_cast<void*>((compressedInfo_ & PAGE_MASK) + (compressedInfo_ & ~PAGE_MASK) * 64u);
    }

    void* base() const{
        return reinterpret_cast<void*>(compressedInfo_ & PAGE_MASK);
    }

    size_t sizeInCl() const{
        return PAGE_SIZE - (compressedInfo_ & ~PAGE_MASK);
    }
};


struct BasicBuffer {
    static constexpr size_t SIZE = 1024u;
    union Data{
        uint64_t qword_;
        char raw_[sizeof(uint64_t)];
    };
    Data data_[SIZE];
    constexpr size_t capacity() const{  return sizeof(data_);   }

    char* bytePtr(size_t p){    return &(data_[p / sizeof(uint64_t)].raw_[p % sizeof(uint64_t)]);    }
    const char* bytePtr(size_t p) const {    return &(data_[p / sizeof(uint64_t)].raw_[p % sizeof(uint64_t)]);    }
};

template<class Buffer>
class Writer{
    BasicBuffer& buf_;
    size_t size_{0};

    template<class T>
    void storeOne(const T& v, size_t){
        //if((size_ + sizeof(T)) < buf_.capacity()){
            memcpy(buf_.bytePtr(size_), &v, sizeof(T));
            size_ += sizeof(T);
        //}
    }
    void storeOne(const char* s, size_t len){
        const auto strLen = static_cast<uint16_t>(len - sizeof(ushort));
        store(strLen);
        memcpy(buf_.bytePtr(size_), s, strLen);
        size_ += strLen;
    }

    template<class T>
    static constexpr size_t sizeOf(const T&, size_t& offset){
        offset += sizeof(T);
        return sizeof(T);
    }
    static size_t sizeOf(const char* s, size_t& offset){
        const auto l = strlen(s) + sizeof(ushort);
        offset += l;
        return l;
    }
public:
    explicit Writer(Buffer& b) : buf_{b}{}

    template<class ...Args>
    void store(const Args& ...args){
        size_t offset{0};
        const size_t lens[sizeof...(args)] = {
            sizeOf(args, offset)...
        };
        if(size_ + offset <= buf_.capacity()){
            size_t i{0};
            (storeOne(args, lens[i++]), ...);
        }
    }

    size_t size() const{    return size_;   }
};

template<class Type, class RetType = Type>
struct TypeId{
    using type = Type;
    using ret_type = RetType;
};

//! Default template for mapping type to type ID. Specialize if needed.
template<class Type>
struct MapType{
    using type_id = TypeId<Type>;
    using type = typename type_id::type;
    using ret_type = typename type_id::ret_type;
};


#define DECL_TYPE_ID(Type)\
    struct IdOf_##Type : TypeId<Type>{}

using uchar = unsigned char;
using schar = signed char;
using ushort = unsigned short;
using ulong = unsigned long;
using llong = long long;
using ullong = unsigned long long;
using ldouble = long double;
using cstring = const char*;

DECL_TYPE_ID(char);
DECL_TYPE_ID(uchar);
DECL_TYPE_ID(schar);
DECL_TYPE_ID(short);
DECL_TYPE_ID(ushort);
DECL_TYPE_ID(int);
DECL_TYPE_ID(unsigned);
DECL_TYPE_ID(long);
DECL_TYPE_ID(ulong);
DECL_TYPE_ID(llong);
DECL_TYPE_ID(ullong);

DECL_TYPE_ID(float);
DECL_TYPE_ID(double);
DECL_TYPE_ID(ldouble);

#undef DECL_TYPE_ID

//template<> struct MapType<cstring>{
//    using type_id = TypeId<cstring, std::string_view>;
//    using type = typename type_id::type;
//    using ret_type = typename type_id::ret_type;
//};

#define DECL_TYPE_ID_EX(Type, RetType)              \
struct IdOf_##Type : TypeId<Type, RetType>{};       \
template<> struct MapType<Type>{                    \
    using type_id = IdOf_##Type;                    \
    using type = typename type_id::type;            \
    using ret_type = typename type_id::ret_type;    \
}

DECL_TYPE_ID_EX(cstring, std::string_view);

#undef DECL_TYPE_ID_EX

class Reader{
    const BasicBuffer& buf_;
    size_t size_;
    size_t rp_;
public:
    Reader(const BasicBuffer& b, size_t s)
    : buf_{b}, size_{s}, rp_{0}
    {}

    template<class TID>
    typename TID::ret_type read(TID){
        using type = typename TID::type;
        type r{};
        if(size_ >=  rp_ + sizeof(type)){
            memcpy(&r, buf_.bytePtr(rp_), sizeof(type));
            rp_ += sizeof(type);
        }
        return typename TID::ret_type{r};
    }

    std::string_view read(IdOf_cstring){
        const auto l = read(IdOf_ushort{});
        std::string_view ret{buf_.bytePtr(rp_), l};
        rp_ += l;
        return ret;
    }

    template<class Type>
    auto read() -> typename MapType<Type>::ret_type{    return read(typename MapType<Type>::type_id{}); }

    void endRecord(){}
};


TEST_CASE("Buffer Encode", "") {
    BasicBuffer buff;
    Writer<BasicBuffer> wr{buff};

    constexpr char c{'\1'};
    constexpr short i16{2};
    constexpr int i32{3};
    constexpr long i64{4};
    constexpr float f32{0.5f};
    constexpr double f64{0.6};

    wr.store(c, i16, i32, i64, f32, f64);

    CHECK(wr.size() == (1 + 2 + 4 + 8 + 4 + 8));
    Reader rd{buff, wr.size()};

    CHECK(c == rd.read<char>());
    CHECK(i16 == rd.read<short>());
    CHECK(i32 == rd.read<int>());
    CHECK(i64 == rd.read<long>());
    CHECK(f32 == Approx{rd.read<float>()});
    CHECK(f64 == Approx{rd.read<double>()});
}

TEST_CASE("Buffer Encode Str", "") {
    BasicBuffer buf;
    Writer<BasicBuffer> wr{buf};

    const char* strVal{"123456"};

    wr.store('c', 1, strVal, 2l);
    CHECK(wr.size() == (1 + 4 + (2 + 6) + 8));

    Reader rd{buf, wr.size()};

    CHECK('c' == rd.read<char>());
    CHECK(1 == rd.read<int>());
    CHECK(strVal == rd.read<const char*>());
    CHECK(2l == rd.read<long>());
}
