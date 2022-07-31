<p align="center">
  <a href="https://github.com/byrcoder/sps">
    <img alt="SPS" src="doc/sps.png" width="712" height="306"/>
  </a>
</p>

[![Build Status Linux and macOS]

# Introduction

  Streaming proxy server(SPS) is a proprietary server for multimedia content. 
  SPS supports multiple  transport protocols, such as [SRT](https://github.com/Haivision/srt) / 
  HTTPS / HTTP / RTMP. 

# What can SPS do?
  SPS can work as [nginx-rtmp](https://github.com/arut/nginx-rtmp-module) and [SRS](https://github.com/ossrs/srs) for push media over rtmp, and
  also if ffmpeg configure is enabled, SPS support flv/ts/mp4/acc/h264-streaming over HTTP/HTTPS/SRT push to the server.
  And the clients can play with different the no-seek media container, such as flv/ts/hls over HTTPS/SRT.  


# Requirements

* cmake (as build system)
* gtest
* http-parser
* state-thread
* srt option
* ffmpeg option
* openssl option

# Configure Options
* `srt` --with(out)-srt enabled(disable) srt
* `https` --with(out)-openssl enabled https. May use -openssl_include and -openssl_lib specify details for openssl
* `ffmpeg` --with(out)-ffmpeg enabled ffmpeg. It is better to make ffmpeg enabled and more Media containers will supports. May use 
-ffmpeg_include and -ffmpeg_lib specify details for ffmpeg

# Build example
```
git clone https://github.com/byrcoder/sps.git
cd sps
git submodule init
git pull --recurse-submodules
./configure --with-ffmpeg -ffmpeg_include=ffmpeg/include -ffmpeg_lib=ffmpeg/lib --without-srt --without-openssl
cd build && make sps
```

# Run

* Run as Live Server
```
cd build
./sps ../conf/sps.conf
```

* Push RTMP or FLV/TS/MP4 to the server
```
ext="ts"
output="output"
ffmpeg -i xx.flv -f flv "rtmp://127.0.0.1/live/test"
ffmpeg -i yy.flv -f flv "http(s)://127.0.0.1/live/${output}.${ext}"
```

* Pull over RTMP/FLV/TS  
```
ext="flv"
output="output"
ffplay "rtmp://127.0.0.1/live/${output}"
ffplay "http(s)://127.0.0.1/live/${output}.${ext}"
```

* SRT
```
ffmpeg -re -i xx.ts -c:v copy -c:a copy -f mpegts 'srt://127.0.0.1:6000?streamid=#!::h=127.0.0.1/live/test,&mode=caller'
ffplay 'srt://127.0.0.1:6000?streamid=#!::h=127.0.0.1/live/test,m=request,&mode=caller'
```
