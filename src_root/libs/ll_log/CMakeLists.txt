set(LIB_NAME ll_log)

add_library(${LIB_NAME} SHARED src/LogPoint.cpp)
target_include_directories(${LIB_NAME} PUBLIC include)

add_subdirectory(tests/basic)
