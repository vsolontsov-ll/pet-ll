/*
 * LogPoint.cpp
 *
 *  Created on: Feb 1, 2020
 *      Author: Vladimir Solontsov
 */
#include <LogPoint.hpp>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <memory>
namespace ll_log::details {

Points& getPoints () {
    static Points p;
    return p;
}

#if __cplusplus > 202000
#   define AGG_INIT(member) .##member =
#else
#   define AGG_INIT(member)
#endif


[[gnu::used]] PointId addPoint(
        PointId& ptid
        , int level
        , const char* fmt
        , std::string&& binFmt
        , const char* file
        , int line
        , const char* func){
    ptid = static_cast<PointId>(getPoints().size());
    LogPoint p{
        AGG_INIT(id_)           ptid
        , AGG_INIT(logLevel_)   level
        , AGG_INIT(binId_)      &ptid
        , AGG_INIT(file_)       file
        , AGG_INIT(line_)       line
        , AGG_INIT(function_)   func
        , AGG_INIT(format_)     fmt
        , AGG_INIT(decodeFmt_)  binFmt
    };
    getPoints().emplace_back(p);
    return ptid;
}

struct BinTextDelim{
    bool bin_;

    friend std::ostream& operator<<(std::ostream& os, BinTextDelim d){
        if(!d.bin_)
            os << ",";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, BinTextDelim& d){
        if(!d.bin_){
            [[maybe_unused]] char c;
            is >> c;
            assert(c == ',' || !is.good());
        }
        return is;
    }
};

struct StringWriter{
    std::string_view s_;
    bool bin_;

    friend std::ostream& operator<<(std::ostream& os, const StringWriter& sw){
        os << sw.s_.size() << BinTextDelim{sw.bin_};
        os.write(sw.s_.data(), static_cast<ssize_t>(sw.s_.size()));
        return os;
    }
};

struct StringReader{
    std::string& s_;
    bool bin_;

    friend std::istream& operator>>(std::istream& is, StringReader sr){
        size_t size;
        BinTextDelim d{sr.bin_};
        is >> size >> d;
        if(!is.good())
            return is;
        sr.s_.resize(size);
        //using char_input = std::istream_iterator<char>;
        is.read(&sr.s_[0], static_cast<ssize_t>(size));
        //std::copy_n(char_input{is}, size, sr.s_.begin());

        return is;
    }
};

template<bool forDumping>
struct PointCodec{
    using PtRef = std::conditional_t<forDumping, const LogPoint&, LogPoint&>;
    PtRef ref_;
    bool bin_;

    friend std::ostream& operator<<(std::ostream& os, const PointCodec& pt){
        BinTextDelim d{pt.bin_};
        return os << pt.ref_.id_ << d
            << pt.ref_.logLevel_ << d
            << reinterpret_cast<uintptr_t>(pt.ref_.binId_) << d
            << StringWriter{pt.ref_.file_, pt.bin_} << d
            << pt.ref_.line_ << d
            << StringWriter{pt.ref_.function, pt.bin_} << d
            << StringWriter{pt.ref_.format_, pt.bin_} << d
            << StringWriter{pt.ref_.decodeFmt_, pt.bin_};
    }
};

inline std::istream& operator>>(std::istream& istrm, const PointCodec<false>& pt){
    BinTextDelim d{pt.bin_};
    uintptr_t binId{0};
    istrm >> pt.ref_.id_ >> d
        >> pt.ref_.logLevel_ >> d
        >> binId >> d
        >> StringReader{pt.ref_.file_, pt.bin_} >> d
        >> pt.ref_.line_ >> d
        >> StringReader{pt.ref_.function, pt.bin_} >> d
        >> StringReader{pt.ref_.format_, pt.bin_} >> d
        >> StringReader{pt.ref_.decodeFmt_, pt.bin_};

    pt.ref_.binId_ = reinterpret_cast<PointBinId>(binId);

    return istrm;
}

std::ostream& operator<<(std::ostream& os, PointsWriter pw){
    const auto& points = getPoints();
    auto first = points.begin() + static_cast<ssize_t>(pw.start_);
    const auto bin{pw.bin_};
    std::for_each(first, points.end(), [&os, bin](const auto& pt){
        os << PointCodec<true>{pt, bin} << BinTextDelim{bin};
    });
    return os;
}


Points readPoints(std::istream& strm, bool bin, size_t num){
    (void)num;
    Points res;
    while(strm.good()){
        LogPoint pt;
        BinTextDelim d{bin};
        strm >> PointCodec<false>{pt, bin} >> d;
        if(pt.binId_)
            res.emplace_back(std::move(pt));
    }
    return res;
}

//struct PointsDumper::PointsDumperImpl{
//    std::ofstream strm_;
//    size_t lastDumpedIdx_{0};
//
//    void dumpNew(){
//        strm_ << PointsWriter{lastDumpedIdx_, true};
//    }
//};
//
//PointsDumper::PointsDumper(std::string_view file)
//: impl_{std::make_unique<PointsDumperImpl>(
//    PointsDumperImpl{std::ofstream(file.data(), std::ios_base::binary | std::ios_base::out), 0u})}
//{
//    assert(!impl_->strm_.fail());
//    update();
//}
//
//void PointsDumper::update(){
//    impl_->dumpNew();
//}

// End of namespace ll_log::details
}
