cmake_minimum_required(VERSION 3.10)
project(sps)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)

option(SRT_DISABLED "disable srt" OFF)
option(ENABLE_UNITTESTS "Enable unit tests" ON)

# set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

message("building sps...")

find_package (Git)
# git 代码信息
execute_process(
        COMMAND	${GIT_EXECUTABLE} log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND	${GIT_EXECUTABLE}  rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

message("git version ${GIT_VERSION} ${GIT_VERSION} ${GIT_BRANCH} disable_srt:${SRT_DISABLED}")

# auto header
set(AUTO_HEAD ${PROJECT_SOURCE_DIR}/src/sps_auto_header.hpp)
file(REMOVE ${AUTO_HEAD})
file(APPEND ${AUTO_HEAD}

        "#ifndef AUTO_HEADER
#define AUTO_HEADER

#define GIT_VERSION \"${GIT_VERSION}\"
#define GIT_BRANCH  \"${GIT_BRANCH}\"
")

if (SRT_DISABLED)
    file(APPEND ${AUTO_HEAD} "#define SRT_DISABLED
")
endif()

file(APPEND ${AUTO_HEAD} "#endif")
#end auto header

include_directories(${PROJECT_SOURCE_DIR}/3rdparty/state-threads/
        ${PROJECT_SOURCE_DIR}/3rdparty/srt/
        ${PROJECT_SOURCE_DIR}/3rdparty/gtest/googletest/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/http-parser/
        )

link_directories(3rdparty/state-threads/obj
                 3rdparty/srt/
                 3rdparty/gtest/lib
                 3rdparty/http-parser
        )

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB SRC_LIST
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/app/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/co/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/co/st/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/http/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/net/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/protocol/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/server/*.cpp"
)

file(GLOB HEAD_LIST
        "${PROJECT_SOURCE_DIR}/src/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/app/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/co/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/co/st/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/http/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/net/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/protocol/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/server/*.hpp"
        )

message("src: ${SRC_LIST}  header:${HEAD_LIST}")

add_executable(sps
        ${SRC_LIST}
        ${HEAD_LIST}
        ${PROJECT_SOURCE_DIR}/src/main/sps.cpp
        )

TARGET_LINK_LIBRARIES(sps
        srt
        libst.a
        libhttp_parser.a
        )

if(ENABLE_UNITTESTS)

    add_executable(test_co
            ${SRC_LIST}
            ${HEAD_LIST}
            ${PROJECT_SOURCE_DIR}/src/test/test_co.cpp)

    add_executable(test_http
            ${SRC_LIST}
            ${HEAD_LIST}
            ${PROJECT_SOURCE_DIR}/src/test/test_http.cpp)

    TARGET_LINK_LIBRARIES(test_co
            libgtest.a
            srt
            libst.a
            libhttp_parser.a
            )

    TARGET_LINK_LIBRARIES(test_http
            libgtest.a
            srt
            libst.a
            libhttp_parser.a
            )

    add_test(
            NAME
            test_co
            COMMAND
            ${CMAKE_BINARY_DIR}/test_co
    )

    add_test(
            NAME
            test_http
            COMMAND
            ${CMAKE_BINARY_DIR}/test_http
    )

    enable_testing()

endif()