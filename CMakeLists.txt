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

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/avformat
        ${PROJECT_SOURCE_DIR}/src/cache
        ${PROJECT_SOURCE_DIR}/src/co
        ${PROJECT_SOURCE_DIR}/src/install
        ${PROJECT_SOURCE_DIR}/src/io
        ${PROJECT_SOURCE_DIR}/src/kernel
        ${PROJECT_SOURCE_DIR}/src/log

        ${PROJECT_SOURCE_DIR}/src/module/
        ${PROJECT_SOURCE_DIR}/src/module/core/
        ${PROJECT_SOURCE_DIR}/src/module/host/
        ${PROJECT_SOURCE_DIR}/src/module/http/
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

        "${PROJECT_SOURCE_DIR}/src/module/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/module/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/core/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/host/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/module/http/*.cpp"
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


        "${PROJECT_SOURCE_DIR}/src/st/co/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/io/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/sync/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/sync/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/url/*.cpp"
        )

message("src: ${SRC_LIST}")

add_executable(sps
        ${SRC_LIST}
        ${HEAD_LIST}
        ${PROJECT_SOURCE_DIR}/src/main/sps_main_sps.cpp
        )

TARGET_LINK_LIBRARIES(sps
        srt
        libst.a
        libhttp_parser.a
        )

if(ENABLE_UNITTESTS)

    add_executable(test_all
            ${SRC_LIST}
            ${HEAD_LIST}
            test/test_cache.cpp
            test/test_co.cpp
            test/test_http.cpp
            test/test_config.cpp
            test/test_url.cpp
            test/test_main.cpp
            )

    add_executable(test_tmp
            ${SRC_LIST}
            ${HEAD_LIST}
            test/test_config.cpp
            test/test_main.cpp
            )


    TARGET_LINK_LIBRARIES(test_all
            libgtest.a
            srt
            libst.a
            libhttp_parser.a
            )

    TARGET_LINK_LIBRARIES(test_tmp
            libgtest.a
            srt
            libst.a
            libhttp_parser.a
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