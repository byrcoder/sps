<p align="center">
  <a href="https://github.com/byrcoder/sps">
    <img alt="SPS" src="https://github.com/byrcoder/sps/blob/dev/conf/doc/sps.jpeg" width="600"/>
  </a>
</p>

[![Build Status Linux and macOS][travis-badge]][travis]

# Introduction

  streaming proxy server(SPS) is a proprietary server for live or vod proxy server for http.
  
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

