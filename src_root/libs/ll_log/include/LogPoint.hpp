/*
 * LogPoint.hpp
 *
 *  Created on: Feb 1, 2020
 *      Author: Vladimir Solontsov
 */

#ifndef SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGPOINT_HPP_
#define SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGPOINT_HPP_
#include <string>
#include <vector>

#include "details/LogInit.hpp"

namespace ll_log{

namespace details{

using PointBinId = const void*;
using PointId = unsigned;

struct LogPoint {
    PointId id_;                //!< Compile-time sequence number
    int logLevel_;              //!< User defined. Greater means more likely will be dumped
    PointBinId binId_;          //!< Pointer value used as ID in binary buffer
    const char* file_;          //!< Source file
    int line_;                  //!< Line number in source code
    const char* function;       //!< Function
    const char* format_;        //!< Format string
    std::string decodeFmt_;     //!< Would be nice to replace with some const-expr
};

using Points = std::vector<LogPoint>;
Points& getPoints();

PointId addPoint(PointId& ptid, int level, const char* fmt, std::string&& binFmt,
    const char* file, int line, const char* func);

}   // End of namespace details

}   // End of namespace ll_log


#endif /* SRC_ROOT_LIBS_LL_LOG_INCLUDE_LOGPOINT_HPP_ */
