/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#ifndef SPS_UPSTREAM_HPP
#define SPS_UPSTREAM_HPP

#include <memory>

#include <cache/sps_cache.hpp>
#include <co/sps_co.hpp>
#include <net/sps_net_socket.hpp>
#include <app/url/sps_url.hpp>
#include <app/stream/sps_stream_dec.hpp>

/**
 * nginx upstream:
 * 1. create_request
 * 2. reinit_request
 * 3. process_header
 * 4. abort_request
 * 5. finalize_request
 * 6.
 */
namespace sps {

class Upstream;
typedef std::shared_ptr<Upstream> PIUpstream;

class UpstreamContainer : public Registers<UpstreamContainer, PIUpstream> {

};

typedef std::shared_ptr<UpstreamContainer> PIUpstreamContainer;;

class Upstream : public ICoHandler, public std::enable_shared_from_this<Upstream> {
    friend class UpstreamContainer;

 public:
    explicit Upstream(PICacheStream cs, PIAvDemuxer dec, PRequestUrl url);
    ~Upstream() override;

 public:
    // virtual error_t process() = 0; // process work in handler
    virtual void abort_request();

 public:
    error_t handler() override;
    void on_stop() override;


public:
    PICacheStream get_cs();

 protected:
    PICacheStream       cs;
    PRequestUrl         url;
    PIAvDemuxer         decoder;
};

class UpstreamFactory : public Single<UpstreamFactory> {
 public:
    PIUpstream create_upstream(PICacheStream cs, PRequestUrl url); // // work as reinit_request
};

}

#endif  // SPS_UPSTREAM_HPP
