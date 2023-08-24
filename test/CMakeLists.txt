set(GOOGLETEST_ROOT
        ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest
        CACHE STRING "googletest source root"
)

include_directories(SYSTEM
                ${GOOGLETEST_ROOT}/include
                ${GOOGLETEST_ROOT}
)

set(GOOGLETEST_SOURCES
        ${GOOGLETEST_ROOT}/src/gtest-all.cc
        ${GOOGLETEST_ROOT}/src/gtest_main.cc
)

foreach(_source ${GOOGLETEST_SOURCES})
    set_source_files_properties(${_source} PROPERTIES GENERATED 1)
endforeach()

add_library(googletest ${GOOGLETEST_SOURCES})
target_link_libraries(googletest pthread)

add_executable(test_value test_value.cc)
target_link_libraries(test_value mudong-json googletest)

add_executable(test_roundtrip test_roundtrip.cc)
target_link_libraries(test_roundtrip mudong-json googletest)

add_executable(test_fileread test_fileread.cc)
target_link_libraries(test_fileread mudong-json googletest)

set(TEST_DIR ${EXECUTABLE_OUTPUT_PATH})
add_test(test_value ${TEST_DIR}/test_value)
add_test(test_roundtrip ${TEST_DIR}/test_roundtrip)
add_test(test_fileread ${TEST_DIR}/test_fileread)