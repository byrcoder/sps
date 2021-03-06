#!/bin/bash

function cmake_project() {
    TMP_TARGET_DIR=$1
    TMP_BUILD_DIR=$2
    TMP_BUILD_FLAG=""
    if [ $# -gt 3 ] ;
    then
      TMP_BUILD_FLAG=$3
    fi

    cd ${TMP_TARGET_DIR} && cmake . -DENABLE_ENCRYPTION=OFF -DENABLE_UNITTESTS=OFF && make ${TMP_BUILD_FLAG} && cd -
    ret=$?;
    if [[ $ret -ne 0 ]];
      then echo "build srt failed, cmake ${TMP_TARGET_DIR} -DENABLE_ENCRYPTION=OFF -DENABLE_UNITTESTS=OFF, ret=$ret";
      exit $ret;
     fi

    ln -s ${TMP_TARGET_DIR} ${TMP_BUILD_DIR}

    echo "PWD:".`pwd`
}

function makefile_project() {
    TMP_TARGET_DIR=$1
    TMP_BUILD_DIR=$2
    TMP_ARGS=$3

    echo "${TMP_TARGET_DIR} -> ${TMP_ARGS}  -> ${TMP_BUILD_DIR}"

    cd ${TMP_TARGET_DIR} && make ${TMP_ARGS} && cd -
    ret=$?;  if [[ $ret -ne 0 ]]; then echo "build ${TMP_TARGET_DIR} ${TMP_ARGS} ${TMP_BUILD_DIR} failed, ret=$ret "; exit $ret; fi

    ln -s ${TMP_TARGET_DIR} ${TMP_BUILD_DIR}
}

if [ ${SRT_ENABLED} = "ON" ]
then
  SRT_TARGET_DIR=${WORK_DIR}/3rdparty/srt
  echo "======building srt ${BUILD_SRT} ${SRT_TARGET_DIR}====="
  cmake_project ${SRT_TARGET_DIR} ${BUILD_SRT}
  echo "======build srt success ${BUILD_SRT} ${SRT_TARGET_DIR}====="
fi

echo "======building gtest====="
GTEST_TARGET_DIR=${WORK_DIR}/3rdparty/gtest
cmake_project ${GTEST_TARGET_DIR} ${BUILD_GTEST}
echo "======build gtest success ${BUILD_GTEST} ${GTEST_TARGET_DIR}====="

ln -s

echo "======building state-threads====="
OSX=NO
uname -s|grep Darwin >/dev/null 2>&1

ret=$?
if [[ 0 -eq $ret ]]; then
    OSX=YES
fi

_ST_MAKE=linux-debug && _ST_EXTRA_CFLAGS="-DMD_HAVE_EPOLL"
# for osx, use darwin for st, donot use epoll.
if [ $OSX = YES ]; then
    _ST_MAKE=darwin-debug && _ST_EXTRA_CFLAGS="-DMD_HAVE_KQUEUE"
fi

ST_TARGET_DIR=${WORK_DIR}/3rdparty/state-threads
makefile_project ${ST_TARGET_DIR} ${BUILD_ST} "${_ST_MAKE} EXTRA_CFLAGS=${_ST_EXTRA_CFLAGS}"
echo "======build state-threads, ${_ST_MAKE} ${_ST_EXTRA_CFLAGS} ${OSX} success====="

echo "======building http-parser========"
HTTP_TARGET_DIR=${WORK_DIR}/3rdparty/http-parser
makefile_project ${HTTP_TARGET_DIR} ${BUILD_HTTP} "package"
echo "======build http-parser success==="

echo "=====building rtmpdump============"
RTMPDUMP_DIR=${WORK_DIR}/3rdparty/rtmpdump

RTMPDUMP_CRYPTO="CRYPTO=  SHARED="
RTMPDUMP_SYS=" "

if [ $OSX = YES ]; then
    RTMPDUMP_SYS="SYS=darwin"
fi

RTMPDUMP_FLAGS=" ${RTMPDUMP_CRYPTO} ${RTMPDUMP_SYS}"
echo "rtmpdump flag ${RTMPDUMP_FLAGS}"
makefile_project ${RTMPDUMP_DIR} ${BUILD_RTMPDUMP} "${RTMPDUMP_FLAGS}"
echo "=====build rtmpdump success============"
