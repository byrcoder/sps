<p align="center">
  <a href="https://github.com/byrcoder/sps">
    <img alt="SPS" src="doc/sps.png" width="712" height="306"/>
  </a>
</p>

[![Build Status Linux and macOS]

# Introduction

  Streaming proxy server(SPS) is a proprietary server for live or vod proxy server for http. 
  Plug is the characteristic of SPS, which will make sps extensible. 
  
  |  sps structure layer |   impl    |     directory  |
  |  -----------------|------------- |--------------------|
  | os dispatch       | coroutine    |      co  （plugin state-threads） |
  | transport         | tcp/srt      |      io   (plugin state-threads srt) |
  | app               | http proxy   |      modules (plugin                 |
  | streaming         | flv          |      avformat(plugin flv)            |
  
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
git submodule update --recursive --remote
./configure --without-srt
```

