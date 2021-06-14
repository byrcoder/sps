<p align="center">
  <a href="https://github.com/byrcoder/sps">
    <img alt="SPS" src="https://github.com/byrcoder/sps/blob/dev/conf/doc/sps.jpeg" width="600"/>
  </a>
</p>

# 介绍

  sps 为了建立商业阶的音视频服务器，使用开发简单，维护成本低，支持高度可拓展性，开发自由度高。
  
# 简单性
    
    * sps使用c++11以上特性，支持协程，开发简单
    * sps支持流媒体专业库，不用开发者对ffmpeg精通（熟悉api即可）
  
# 扩展性

    * 协程独立，可替换协程模块
    * 传输扩展性，支持网络和应用层面的分离，如http可以在tcp/quic/srt等传输协议上实现。 
    * 模块扩展性，支持http/rtmp模块，也可以支持第三方模块，代码改动量比较小
    * 流媒体协议扩展性，支持ffmpeg专业级流媒体，也支持自定义协议

# 计划

    * http模块
    * stream模块
        * edge模式
        * 媒体流协议，flv
    * rtmp模块
        * 推流/拉流
        * 转换
    * stream remux支持
        * ffmpeg插件
        * 支持rtmp/hls/flv/dash/cmaf之间的转换   