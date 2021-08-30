<p align="center">
  <a href="https://github.com/byrcoder/sps">
    <img alt="SPS" src="doc/sps.png" width="712" height="306"/>
  </a>
</p>

[![Build Status Linux and macOS]

# Introduction

  Streaming proxy server(SPS) is a proprietary server for live or vod proxy server for http and
  rtmp/flv/ts. SPS support stream decode/encoder for rtmp/flv/ts. Plug is the characteristic of SPS, 
  which will make sps extensible. 
  
  |  module |   impl    |     directory  |
  |  -----------------|------------- |--------------------|
  | os dispatch       | coroutine    |      co  （plugin state-threads） |
  | transport         | tcp/srt      |      io   (plugin state-threads srt) |
  | app               | http/rtmp/srt    |      modules (plugin)                |
  | streaming format         | flv/rtmp/ts  |      avformat      |
  | streaming codec   | h264/aac  |      avcodec      |
  
# Requirements

* cmake (as build system)
* gtest
* http-parser
* state-thread
* srt option

## For Linux & Mac

Install cmake and install git submodules.

### Linux & Mac
```
git submodule init
git pull --recurse-submodules
./configure --without-srt
```

### Running
```
cd build
./sps ../conf/sps.conf
```

### push rtmp pull rtmp/flv example 
```
ffmpeg -i xx.flv -f flv "rtmp://127.0.0.1/live/test"
ffplay "rtmp://127.0.0.1/live/test"
ffplay "http://127.0.0.1/live/test.flv"
```

### push flv pull rtmp/flv example 
```
ffmpeg -i xx.flv -f flv "http://127.0.0.1/live/test.flv"
ffplay "rtmp://127.0.0.1/live/test"
ffplay "http://127.0.0.1/live/test.flv"
```

### push ts pull
```
ffmpeg -i xx.flv -f mpegts "http://127.0.0.1/live/test.ts"
```

###  push/pull ts over srt
```
ffmpeg -re -i xx.ts -c:v copy -c:a copy -f mpegts 'srt://127.0.0.1:6000?streamid=#!::h=127.0.0.1/live/test,&mode=caller'
ffplay 'srt://127.0.0.1:6000?streamid=#!::h=127.0.0.1/live/test,m=request,&mode=caller'
```
