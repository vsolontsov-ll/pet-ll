/*
 * LogPoint.hpp
 *
 *  Created on: Feb 1, 2020
 *      Author: Vladimir Solontsov
 */

#ifndef SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGPOINT_HPP_
#define SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGPOINT_HPP_
#include <memory>
#include <string>
#include <vector>

#include "details/LogInit.hpp"

namespace ll_log{

namespace details{

using PointBinId = const void*;
using PointId = unsigned;

/**
 * TODO: Consider segregating to runtime and compile-time log point description.
 * For now as it's common for dumping and parsing use objects for all strings
 */
struct LogPoint {
    PointId id_;                //!< Compile-time sequence number
    int logLevel_;              //!< User defined. Greater means more likely will be dumped
    PointBinId binId_;          //!< Pointer value used as ID in binary buffer
    std::string file_;          //!< Source file
    int line_;                  //!< Line number in source code
    std::string function;       //!< Function
    std::string format_;        //!< Format string
    std::string decodeFmt_;     //!< Would be nice to replace with some const-expr
};

using Points = std::vector<LogPoint>;
Points& getPoints();

//class PointsDumper{
//    //using Points = std::vector<LogPoint>;
//    struct PointsDumperImpl;
//    std::unique_ptr<PointsDumperImpl> impl_;
//public:
//    explicit PointsDumper(std::string_view file);
//    void update();
//};

//! Dumps points from start.
/*
 * Can be used to append points dump after loading a shared object
 */
struct PointsWriter{
    size_t start_{0};
    bool bin_{true};

    friend std::ostream& operator<<(std::ostream&, PointsWriter);
};

PointId addPoint(PointId& ptid, int level, const char* fmt, std::string&& binFmt,
    const char* file, int line, const char* func);

//! Read log points descriptions until EOF or num points parsed.
Points readPoints(std::istream&, bool bin, size_t num = 0);


}   // End of namespace details

}   // End of namespace ll_log


#endif /* SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGPOINT_HPP_ */
