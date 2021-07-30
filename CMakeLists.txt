cmake_minimum_required(VERSION 3.10)
project(sps)

set(CMAKE_CXX_STANDARD 14)

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

"#ifndef SPS_AUTO_HEADER
#define SPS_AUTO_HEADER

#define GIT_VERSION \"${GIT_VERSION}\"
#define GIT_BRANCH  \"${GIT_BRANCH}\"\r\n")

if (SRT_DISABLED)
    file(APPEND ${AUTO_HEAD} "#define SRT_DISABLED\r\n")
endif()

file(APPEND ${AUTO_HEAD} "\r\n")
file(APPEND ${AUTO_HEAD} "#endif")
#end auto header

include_directories(${PROJECT_SOURCE_DIR}/3rdparty/state-threads/
        ${PROJECT_SOURCE_DIR}/3rdparty/srt/
        ${PROJECT_SOURCE_DIR}/3rdparty/gtest/googletest/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/http-parser/
        ${PROJECT_SOURCE_DIR}/3rdparty/rtmpdump/
        ${PROJECT_SOURCE_DIR}/3rdparty/rtmpdump/librtmp
        )

link_directories(3rdparty/state-threads/obj
                 3rdparty/srt/
                 3rdparty/gtest/lib
                 3rdparty/http-parser
                 3rdparty/rtmpdump/librtmp
        )

include_directories(
        ${PROJECT_SOURCE_DIR}/src

        ${PROJECT_SOURCE_DIR}/src/avcodec

        ${PROJECT_SOURCE_DIR}/src/avformat
        ${PROJECT_SOURCE_DIR}/src/cache
        ${PROJECT_SOURCE_DIR}/src/co
        ${PROJECT_SOURCE_DIR}/src/install
        ${PROJECT_SOURCE_DIR}/src/io
        ${PROJECT_SOURCE_DIR}/src/librtmp
        ${PROJECT_SOURCE_DIR}/src/kernel
        ${PROJECT_SOURCE_DIR}/src/log

        ${PROJECT_SOURCE_DIR}/src/module/
        ${PROJECT_SOURCE_DIR}/src/module/root/
        ${PROJECT_SOURCE_DIR}/src/module/host/
        ${PROJECT_SOURCE_DIR}/src/module/http/
        ${PROJECT_SOURCE_DIR}/src/module/rtmp/
        ${PROJECT_SOURCE_DIR}/src/module/server/
        ${PROJECT_SOURCE_DIR}/src/module/stream/
        ${PROJECT_SOURCE_DIR}/src/module/upstream/

        ${PROJECT_SOURCE_DIR}/src/st/co
        ${PROJECT_SOURCE_DIR}/src/st/io
        ${PROJECT_SOURCE_DIR}/src/st/sync

        ${PROJECT_SOURCE_DIR}/src/sync
        ${PROJECT_SOURCE_DIR}/src/url
        )

file(GLOB SRC_LIST
        "${PROJECT_SOURCE_DIR}/src/*.cpp"

        ${PROJECT_SOURCE_DIR}/src/avcodec/*.cpp

        "${PROJECT_SOURCE_DIR}/src/module/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/module/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/root/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/host/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/http/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/rtmp/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/server/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/stream/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/tran/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/upstream/*.cpp"
        "src/url/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/avformat/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/cache/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/co/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/install/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/log/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/io/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/librtmp/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/st/co/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/io/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/sync/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/sync/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/url/*.cpp"
        )

file(GLOB SRC_TEST_LIST
        "${PROJECT_SOURCE_DIR}/test/*.cpp")

message("src: ${SRC_LIST}"  test:"${SRC_TEST_LIST}")



# Add static and shared library target
# add_library(spslib_static STATIC ${SRC_LIST})
# add_library(spslib_shared SHARED ${SRC_LIST})

if (SRT_DISABLED)
    set(libs "libst.a libhttp_parser.a libgtest.a librtmp.a")
else ()
    set(libs "srt libst.a libhttp_parser.a librtmp.a")
endif()

add_executable(sps
        ${SRC_LIST}
        ${HEAD_LIST}
        ${PROJECT_SOURCE_DIR}/src/main/sps_main_sps.cpp
        )

target_link_libraries(sps libst.a libhttp_parser.a librtmp.a)

if(ENABLE_UNITTESTS)

    add_executable(test_all
            ${SRC_LIST}
            ${HEAD_LIST}
            ${SRC_TEST_LIST}
            )

    add_executable(test_tmp
            ${SRC_LIST}
            ${HEAD_LIST}
            test/test_av_flv.cpp
            test/test_main.cpp
            )


    target_link_libraries(test_all
            libgtest.a
            libst.a libhttp_parser.a libgtest.a librtmp.a
            )

    target_link_libraries(test_tmp
            libgtest.a
            libst.a libhttp_parser.a libgtest.a  librtmp.a
            )

    add_test(
            NAME
            test_all
            COMMAND
            ${CMAKE_BINARY_DIR}/test_all
    )

    add_test(
            NAME
            test_tmp
            COMMAND
            ${CMAKE_BINARY_DIR}/test_tmp
    )

    enable_testing()

endif()