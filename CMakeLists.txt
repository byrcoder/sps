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

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB SRC_LIST
        "${PROJECT_SOURCE_DIR}/src/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/app/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/app/config/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/app/http/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/app/stream/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/app/tran/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/app/upstream/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/app/url/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/cache/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/co/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/log/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/net/*.cpp"

        "src/app/server/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/st/co/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/net/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/sync/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/st/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/sync/*.cpp"
        )

file(GLOB HEAD_LIST
        "${PROJECT_SOURCE_DIR}/src/*.hpp"

        "${PROJECT_SOURCE_DIR}/src/app/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/app/config/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/app/http/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/app/stream/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/app/tran/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/app/upstream/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/app/url/*.hpp"

        "${PROJECT_SOURCE_DIR}/src/cache/*.hpp"


        "${PROJECT_SOURCE_DIR}/src/co/*.hpp"

        "${PROJECT_SOURCE_DIR}/src/log/*.hpp"

        "${PROJECT_SOURCE_DIR}/src/net/*.hpp"

        "src/app/server/*.hpp"

        "${PROJECT_SOURCE_DIR}/src/st/co/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/st/net/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/st/sync/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/st/*.hpp"

        "${PROJECT_SOURCE_DIR}/src/sync/*.cpp"

        "src/app/upstream/*.cpp"
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

    add_executable(test_all
            ${SRC_LIST}
            ${HEAD_LIST}
            ${PROJECT_SOURCE_DIR}/src/test/test_cache.cpp
            ${PROJECT_SOURCE_DIR}/src/test/test_co.cpp
            ${PROJECT_SOURCE_DIR}/src/test/test_http.cpp
            ${PROJECT_SOURCE_DIR}/src/test/test_config.cpp
            ${PROJECT_SOURCE_DIR}/src/test/test_url.cpp
            )


    TARGET_LINK_LIBRARIES(test_all
            libgtest.a
            srt
            libst.a
            libhttp_parser.a
            )


    add_test(
            NAME
            test_cache
            COMMAND
            ${CMAKE_BINARY_DIR}/test_all
    )

    enable_testing()

endif()