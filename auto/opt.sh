#!/bin/bash

SRT_ENABLED=ON
OPENSSL_ENABLED=OFF
OPENSSL_INCLUDE=""
OPENSSL_LIB=""

FFMPEG_ENABLED=OFF
FFMPEG_INCLUDE=""
FFMPEG_LIB=""

function parse_key_value() {
    opt=$1
    value=$2

    case $opt in
      -openssl_include)
        OPENSSL_INCLUDE=$value
        echo "OPENSSL_INCLUDE $value"
        ;;
      -openssl_lib)
        OPENSSL_LIB=$value
        echo "OPENSSL_LIB $value"
        ;;
      -ffmpeg_include)
        FFMPEG_INCLUDE=$value
        echo "FFMPEG_INCLUDE $value"
        ;;
      -ffmpeg_lib)
        FFMPEG_LIB=$value
        echo "FFMPEG_LIB $value"
        ;;
      *)
        echo "unknown key value $opt $value"
        # unknown option
        ;;
    esac

}

function parse_opts() {
    opts="$@"
    echo "parse $opts"
    for i in $opts
    do
        case $i in
            --with-srt)
              SRT_ENABLED=ON
              shift # past argument=value
              ;;
            --without-srt)
              SRT_ENABLED=OFF
              shift
              ;;

             --without-openssl)
              OPENSSL_ENABLED=OFF
              shift
              ;;
            --with-openssl)
              OPENSSL_ENABLED=ON
              shift
              ;;
            --with-ffmpeg)
              FFMPEG_ENABLED=ON
              shift
              ;;

            -*=*)
              value=`echo "$i" | sed -e 's|[-_a-zA-Z0-9/]*=||'`
              opt=`echo "$i" | sed -e 's|=[-_a-zA-Z0-9/.]*||'`
              parse_key_value $opt $value
              shift
              ;;
            *)
              echo "unknown option $i"
              # unknown option
            ;;
        esac
    done
}

parse_opts $@

echo "srt enabled: ${SRT_ENABLED}"
