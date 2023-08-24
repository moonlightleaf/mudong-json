if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release")
endif()

add_executable(bench_taobao bench_taobao.cc)

target_link_libraries(bench_taobao mudong-json benchmark pthread)

