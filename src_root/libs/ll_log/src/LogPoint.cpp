/*
 * LogPoint.cpp
 *
 *  Created on: Feb 1, 2020
 *      Author: vsol
 */
#include <LogPoint.hpp>

namespace ll_log::internal {

    Points& getPoints () {
        static Points p;
        return p;
    }

    [[gnu::used]] PointId addPoint(PointId& ptid, int level, const char* fmt, const char* file, int line, const char* func){
        ptid = static_cast<PointId>(getPoints().size());
        LogPoint p{
            AGG_INIT(id_)           ptid
            , AGG_INIT(logLevel_)   level
            , AGG_INIT(binId_)      &ptid
            , AGG_INIT(file_)       file
            , AGG_INIT(line_)       line
            , AGG_INIT(function_)   func
            , AGG_INIT(format_)     fmt
            , AGG_INIT(decodeFmt_)  ""
        };
        getPoints().emplace_back(p);
        return ptid;
    }

// End of namespace ll_log::internal
}
