cmake_minimum_required(VERSION 3.10)
project(sps)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)

# srt option
option(SRT_ENABLED "srt enabled" OFF)

# openssl option
option(OPENSSL_ENABLED "openssl enabled" OFF)
option(OPENSSL_INCLUDE "" "")
option(OPENSSL_LIB "" "")

# ffmpeg option
option(FFMPEG_ENABLED "ffmpeg enabled" OFF)
option(FFMPEG_INCLUDE "ffmpeg include" "")
option(FFMPEG_LIB "ffmpeg lib" "")

# unit test option
option(ENABLE_UNITTESTS "Enable unit tests" ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

message("building sps... C. ${CMAKE_C_COMPILER} C++. ${CMAKE_CXX_COMPILER}")

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

message("git version ${GIT_VERSION} ${GIT_VERSION} ${GIT_BRANCH}
        srt_enabled :${SRT_ENABLED}
        openssl_enabled: ${OPENSSL_ENABLED}
        openssl_include: ${OPENSSL_INCLUDE}
        openssl_lib: ${OPENSSL_LIB}
        ffmpeg_enabled: ${FFMPEG_ENABLED}
        ffmpeg_include: ${FFMPEG_INCLUDE}
        ffmpeg_lib: ${FFMPEG_LIB}")

# lib option
function(lib_opt def incl lib)
    if (${${def}})
        file(APPEND ${AUTO_HEAD} "#define ${def} \r\n")
        if (incl)
            include_directories(${${incl}})
        endif()

        if (lib)
            link_directories(${${lib}})
        endif()
        message("${def} ${${def}} inc ${${incl}} ${${lib}}")
    endif()
endfunction(lib_opt)

# auto header
set(AUTO_HEAD ${PROJECT_SOURCE_DIR}/src/sps_auto_header.hpp)
file(REMOVE ${AUTO_HEAD})
file(APPEND ${AUTO_HEAD}

"#ifndef SPS_AUTO_HEADER
#define SPS_AUTO_HEADER

#define GIT_VERSION \"${GIT_VERSION}\"
#define GIT_BRANCH  \"${GIT_BRANCH}\"\r\n")

if (SRT_ENABLED)
    file(APPEND ${AUTO_HEAD} "#define SRT_ENABLED\r\n")
endif()

# if (OPENSSL_ENABLED)
#    file(APPEND ${AUTO_HEAD} "#define OPENSSL_ENABLED\r\n")
# endif()

#if (FFMPEG_ENABLED)
#    file(APPEND ${AUTO_HEAD} "#define FFMPEG_ENABLED\r\n")
#endif()

#if (OPENSSL_ENABLED)
#    if (OPENSSL_INCLUDE)
#        include_directories(${OPENSSL_INCLUDE})
#    endif()
#
#    if (OPENSSL_LIB)
#        link_directories(${OPENSSL_LIB})
#    endif()
#endif()

lib_opt(OPENSSL_ENABLED OPENSSL_INCLUDE OPENSSL_LIB)
lib_opt(FFMPEG_ENABLED  FFMPEG_INCLUDE FFMPEG_LIB)

file(APPEND ${AUTO_HEAD} "\r\n")
file(APPEND ${AUTO_HEAD} "#endif")
#end auto header

include_directories(
        ${PROJECT_SOURCE_DIR}/3rdparty/state-threads/
        ${PROJECT_SOURCE_DIR}/3rdparty/srt/
        ${PROJECT_SOURCE_DIR}/3rdparty/gtest/googletest/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/http-parser/
        ${PROJECT_SOURCE_DIR}/3rdparty/rtmpdump/
        ${PROJECT_SOURCE_DIR}/3rdparty/rtmpdump/librtmp
        )

link_directories(
        ${PROJECT_SOURCE_DIR}/3rdparty/state-threads/obj
        ${PROJECT_SOURCE_DIR}/3rdparty/srt
        ${PROJECT_SOURCE_DIR}/3rdparty/gtest/lib
        ${PROJECT_SOURCE_DIR}/3rdparty/http-parser
        ${PROJECT_SOURCE_DIR}/3rdparty/rtmpdump/librtmp
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
        ${PROJECT_SOURCE_DIR}/src/module/srt/
        ${PROJECT_SOURCE_DIR}/src/module/stream/
        ${PROJECT_SOURCE_DIR}/src/module/upstream/

        ${PROJECT_SOURCE_DIR}/src/st/co
        ${PROJECT_SOURCE_DIR}/src/st/io
        ${PROJECT_SOURCE_DIR}/src/st/sync

        ${PROJECT_SOURCE_DIR}/src/sync
        ${PROJECT_SOURCE_DIR}/src/url
        ${PROJECT_SOURCE_DIR}/src/util
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
        "${PROJECT_SOURCE_DIR}/src/module/srt/*.cpp"
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
        "${PROJECT_SOURCE_DIR}/src/util/*.cpp"
        )

file(GLOB SRC_TEST_LIST
        "${PROJECT_SOURCE_DIR}/test/*.cpp")

message("src: ${SRC_LIST}"  test:"${SRC_TEST_LIST}")

add_executable(sps
        ${SRC_LIST}
        ${HEAD_LIST}
        ${PROJECT_SOURCE_DIR}/src/main/sps_main_sps.cpp
        )

target_link_libraries(sps libst.a libhttp_parser.a librtmp.a)
if (SRT_ENABLED)
    target_link_libraries(sps ${PROJECT_SOURCE_DIR}/3rdparty/srt/libsrt.a)
endif()
if (OPENSSL_ENABLED)
    target_link_libraries(sps ${OPENSSL_LIB}/libssl.a ${OPENSSL_LIB}/libcrypto.a)
endif()
if (FFMPEG_ENABLED)
    target_link_libraries(sps avformat avcodec avdevice avfilter avutil swresample swscale)
endif()

if(ENABLE_UNITTESTS)
    add_executable(test_all
            ${SRC_LIST}
            ${HEAD_LIST}
            ${SRC_TEST_LIST}
            )

    target_link_libraries(test_all
            libgtest.a
            libst.a libhttp_parser.a libgtest.a librtmp.a pthread
            )
    if (SRT_ENABLED)
        target_link_libraries(test_all ${PROJECT_SOURCE_DIR}/3rdparty/srt/libsrt.a)
    endif()
    if (OPENSSL_ENABLED)
        target_link_libraries(test_all ${OPENSSL_LIB}/libssl.a ${OPENSSL_LIB}/libcrypto.a)
    endif()

    if (FFMPEG_ENABLED)
        target_link_libraries(test_all avformat avcodec avdevice avfilter avutil swresample swscale)
    endif()

    add_test(
            NAME
            test_all
            COMMAND
            ${CMAKE_BINARY_DIR}/test_all
    )

    enable_testing()
endif()

if (SRT_ENABLED)
    add_executable(srt_http_client
        ${SRC_LIST}
        ${HEAD_LIST}
        ${PROJECT_SOURCE_DIR}/tools/srt_http_client.cpp
        )
    target_link_libraries(srt_http_client libst.a libhttp_parser.a librtmp.a)
    target_link_libraries(srt_http_client ${PROJECT_SOURCE_DIR}/3rdparty/srt/libsrt.a)
    if (OPENSSL_ENABLED)
        target_link_libraries(srt_http_client ${OPENSSL_LIB}/libssl.a  ${OPENSSL_LIB}/libcrypto.a)
    endif()
    if (FFMPEG_ENABLED)
        target_link_libraries(srt_http_client avformat avcodec avdevice avfilter avutil swresample swscale)
    endif()
endif()

