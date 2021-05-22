cmake_minimum_required(VERSION 3.10)
project(sps)

set(CMAKE_CXX_STANDARD 11)

option(ENABLE_UNITTESTS "Enable unit tests" OFF)
# 关闭 加密
option(ENABLE_ENCRYPTION "Enable srt encryption" OFF)

set(CMAKE_CXX_STANDARD 11)

# 关闭 加密
set(ENABLE_ENCRYPTION OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g")

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

message("git version ${GIT_VERSION} ${GIT_VERSION} ${GIT_BRANCH}")

# auto header
set(AUTO_HEAD ${PROJECT_SOURCE_DIR}/src/sps_auto_header.hpp)
file(REMOVE ${AUTO_HEAD})
file(TOUCH ${AUTO_HEAD})
file(APPEND ${AUTO_HEAD}

        "#ifndef AUTO_HEADER
#define AUTO_HEADER

#define GIT_VERSION \"${GIT_VERSION}\"
#define GIT_BRANCH  \"${GIT_BRANCH}\"
")
file(APPEND ${AUTO_HEAD} "#endif")
#end auto header

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g")



include_directories(${PROJECT_SOURCE_DIR}/src)

file (GLOB_RECURSE App_Headers "${PROJECT_SOURCE_DIR}/src/app/*.hpp")
file (GLOB_RECURSE App_Cpps    "${PROJECT_SOURCE_DIR}/src/app/*.cpp")

file (GLOB Config_Headers    "${PROJECT_SOURCE_DIR}/src/config/*.hpp")
file (GLOB Config_Cpps       "${PROJECT_SOURCE_DIR}/src/config/*.cpp")

file (GLOB Utils_Headers     "${PROJECT_SOURCE_DIR}/src/utils/*.hpp")
file (GLOB Utils_Cpps        "${PROJECT_SOURCE_DIR}/src/utils/*.cpp")

file (GLOB Log_Cpps          "${PROJECT_SOURCE_DIR}/src/log/*.cpp")
file (GLOB Log_Headers       "${PROJECT_SOURCE_DIR}/src/log/*.hpp")

file (GLOB Coroutine_Cpps    "${PROJECT_SOURCE_DIR}/src/co/*.cpp")
file (GLOB Coroutine_Headers "${PROJECT_SOURCE_DIR}/src/co/*.h*")

file (GLOB StNet_Cpps        "${PROJECT_SOURCE_DIR}/src/co/st/*.cpp")
file (GLOB StNet_Headers     "${PROJECT_SOURCE_DIR}/src/co/st/*.h*")

file (GLOB Kernel_Cpps       "${PROJECT_SOURCE_DIR}/src/kernel/*.cpp")
file (GLOB Kernel_Headers    "${PROJECT_SOURCE_DIR}/src/kernel/*.h*")


file(GLOB Utils_Cpps         "${PROJECT_SOURCE_DIR}/src/utils/*.cpp")
file(GLOB Utils_Headers      "${PROJECT_SOURCE_DIR}/src/utils/*.hpp")

add_subdirectory(3rdparty/srt)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/srt/)

include_directories(${PROJECT_SOURCE_DIR}/3rdparty/state-threads/)
link_directories(3rdparty/state-threads/obj)

add_executable(sps
        ${App_Cpps}
        ${App_Headers}

        ${Config_Headers}
        ${Config_Cpps}

        ${Log_Headers}
        ${Log_Cpps}

        ${Utils_Headers}
        ${Utils_Cpps}

        ${Coroutine_Cpps}
        ${Coroutine_Headers}

        ${Kernel_Cpps}
        ${Kernel_Headers}

        ${StNet_Cpps}
        ${StNet_Headers}

        ${Utils_Cpps}
        ${Utils_Headers}

        ${PROJECT_SOURCE_DIR}/src/main/sps.cpp
        )

TARGET_LINK_LIBRARIES(sps
        srt_static
        st
        )

if(ENABLE_UNITTESTS)

    find_package(GTest)
    if (NOT GTEST_FOUND)
        warn("gtest not found skip it ...")
    else()
        message("building gtest ...")

        add_test(
                NAME
                test_tcp
                COMMAND
                ${CMAKE_BINARY_DIR}/test_tcp
        )

        enable_testing()
    endif()
endif()