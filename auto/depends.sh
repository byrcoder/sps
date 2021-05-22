#!/bin/bash



function cmake_project() {
    TARGET_DIR=$1
    TMP_BUILD_DIR=$2
    cd ${TARGET_DIR} && cmake ${TARGET_DIR} -DENABLE_ENCRYPTION=OFF -DENABLE_UNITTESTS=OFF && make && cd -
    ret=$?;  if [[ $ret -ne 0 ]]; then echo "build srt failed, ret=$ret"; exit $ret; fi

    ln -s ${TARGET_DIR} ${TMP_BUILD_DIR}

    echo "PWD:".`pwd`
}
echo "======building srt====="
SRT_TARGET_DIR=${WORK_DIR}/3rdparty/srt
cmake_project ${SRT_TARGET_DIR} ${SRT_BUILD}
echo "======build srt success ${SRT_BUILD} ${SRT_TARGET_DIR}====="

echo "======building gtest====="
GTEST_TARGET_DIR=${WORK_DIR}/3rdparty/gtest
cmake_project ${GTEST_TARGET_DIR} ${GTEST_BUILD}
echo "======build gtest success ${GTEST_BUILD} ${GTEST_TARGET_DIR}====="

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
cd ${ST_TARGET_DIR} &&  make ${_ST_MAKE} EXTRA_CFLAGS="${_ST_EXTRA_CFLAGS}" && cd -
ret=$?;  if [[ $ret -ne 0 ]]; then echo "build st failed, ret=$ret, ${_ST_MAKE} ${_ST_EXTRA_CFLAGS} ${OSX} "; exit $ret; fi

ln -s ${ST_TARGET_DIR} ${ST_BUILD}
echo "======build state-threads, ${_ST_MAKE} ${_ST_EXTRA_CFLAGS} ${OSX} success====="