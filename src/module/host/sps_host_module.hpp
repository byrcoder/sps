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

#ifndef SPS_HOST_MODULE_HPP
#define SPS_HOST_MODULE_HPP

#include <map>
#include <memory>
#include <string>

#include <sps_module.hpp>
#include <sps_st_io_ssl.hpp>
#include <sps_stream_module.hpp>

#define HOST_OPTIONS \
            { "enabled",    "enabled",    OFFSET(enabled), CONF_OPT_TYPE_BOOL,   {.str = "on"} }, \
            { "pass_proxy", "pass_proxy",    OFFSET(pass_proxy), CONF_OPT_TYPE_STRING,   {.str = "- pass_proxy"} }, \
            { "pass_url",   "pass_url",      OFFSET(pass_url),   CONF_OPT_TYPE_STRING,   {.str = "- pass_url"} }, \
            { "role",       "proxy/source",  OFFSET(role),       CONF_OPT_TYPE_STRING, {.str = "proxy"} }, \
            { "streaming",  "streaming",     OFFSET(streaming),  CONF_OPT_TYPE_BOOL,   { .str = "on" }, }, \
            { "edge_avformat",  "stream_avformat",     OFFSET(edge_avformat),  CONF_OPT_TYPE_STRING,   { .str = "-" }, }

namespace sps {

extern const char* krole_proxy;
extern const char* krole_source;

struct HostConfCtx : public ConfCtx {
    int         enabled;  // off when disabled, on enabled
    std::string pass_proxy;   // address
    std::string pass_url;     // pass_url
    std::string role;         // edge or source

    int streaming;  // on, edge or source is rtmp/flv
    std::string edge_avformat;  // edge avformat

    std::string ssl_key_file;  // ssl key
    std::string ssl_crt_file;  // ssl crt
};

#define OFFSET(x) offsetof(HostConfCtx, x)
static const ConfigOption host_options[] = {
        HOST_OPTIONS,
        {"ssl_key_file",    "ssl key file",     OFFSET(ssl_key_file),    CONF_OPT_TYPE_STRING,   { .str = "" }, },
        {"ssl_crt_file",    "ssl crt file",     OFFSET(ssl_crt_file),    CONF_OPT_TYPE_STRING,   { .str = "" }, },
        { nullptr }
};
#undef OFFSET

class HostModule : public IModule {
 public:
    MODULE_CONSTRUCT(Host, host_options);

    MODULE_CREATE_CTX(Host);

 public:
    error_t post_sub_module(PIModule sub) override;

 public:
    bool        enabled();
    bool        publish();
    bool        proxy();
    bool        is_streaming();
    bool        support_publish(const std::string& format);
    std::string edge_format();
    std::string pass_proxy();
    std::string role();
    std::string ssl_key_file();
    std::string ssl_cert_file();

 public:
    /**
     * @deprecated stream module not used
     */
    PStreamModule stream_module;
};
typedef std::shared_ptr<HostModule> PHostModule;

MODULE_FACTORY(Host)
class HostModuleFactory;


class HostModulesRouter
#ifdef OPENSSL_ENABLED
        :public ISSLCertSearch
#endif
        {
 public:
    static std::string get_wildcard_host(std::string host);

 public:
    HostModulesRouter() = default;

 public:
    error_t register_host(PHostModule host);

    PHostModule find_host(const std::string& host);

    error_t merge(HostModulesRouter* router);

 public:
#ifdef OPENSSL_ENABLED
    error_t search(const std::string& server_name, SSLConfig& config) override;
#endif

 private:
    std::map<std::string, PHostModule> exact_hosts;   // exact match
    std::map<std::string, PHostModule> wildcard_hosts;    // *. path match
    PHostModule                        default_host;  // default module
};

typedef std::shared_ptr<HostModulesRouter> PHostModulesRouter;

}  // namespace sps

#endif  // SPS_HOST_MODULE_HPP
