set(LIB_NAME test_stat)

add_library(${LIB_NAME} OBJECT StatLib.cpp)
target_link_libraries(${LIB_NAME} PUBLIC ll_log)
