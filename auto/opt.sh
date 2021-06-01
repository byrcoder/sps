#!/bin/bash

SRT_DISABLED=ON

function parse_opts() {
    opts="$@"
    echo "parse $opts"
    for i in $opts
    do
        case $i in
            --with-srt)
              SRT_DISABLED=OFF
              shift # past argument=value
              ;;
            --without-srt)
              SRT_DISABLED=ON
              shift
              ;;
            *)
              # unknown option
            ;;
        esac
    done
}

parse_opts $@

echo "srt dis enabled: ${SRT_DISABLED}"
