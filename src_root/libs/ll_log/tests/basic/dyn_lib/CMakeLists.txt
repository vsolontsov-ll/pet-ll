set(LIB_NAME test_dyn)

add_library(${LIB_NAME} SHARED DynLib.cpp)
target_link_libraries(${LIB_NAME} PUBLIC ll_log)
