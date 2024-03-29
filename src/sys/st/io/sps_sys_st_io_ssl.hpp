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

#ifndef SPS_SYS_ST_IO_SSL_HPP
#define SPS_SYS_ST_IO_SSL_HPP

#include <sps_io_socket.hpp>

#ifdef OPENSSL_ENABLED

#include <openssl/ssl.h>

#include <memory>

#include <sps_sys_co.hpp>
#include <sps_sys_sync.hpp>

namespace sps {

// ssl connect for client
error_t st_tcp_ssl_connect(const std::string& server, int port,
                           utime_t tm, PSocket& io);

// ssl server accept
error_t st_tcp_ssl_accept(PSocket& io, utime_t timeout);

enum SSLRole {
    SSL_CLIENT,
    SSL_SERVER
};

struct SSLConfig {
    std::string crt_file;
    std::string key_file;
    std::string server_host;
};

/**
 * find cert config for host
 */
class ISSLCertSearch {
 public:
    virtual error_t search(const std::string& server_name, SSLConfig& config) = 0;
};

typedef std::shared_ptr<ISSLCertSearch> PISSLCertSearch;

/**
 * ssl wrapped
 * https://datatracker.ietf.org/doc/html/rfc5246
 */
class StSSLSocket : public IReaderWriter {
 public:
    static std::map<SSL_CTX*, StSSLSocket*> cb_ctx;
    // support sni
    static int ssl_servername_callback(SSL* ssl, int *ag, void *arg);

 public:
    explicit StSSLSocket(PIReaderWriter io, SSLRole mode, SSLConfig config, PISSLCertSearch cs = nullptr);
    ~StSSLSocket();

 public:
    error_t init();

 private:
    error_t handshake();
    error_t accept();

    error_t handshake_util_write();
    error_t handshake_util_read();
    error_t do_handshake(bool reading);

 private:
    error_t do_ssl_verify(SSLConfig& config);

 public:
    // reader
    void    set_recv_timeout(utime_t tm) override;
    utime_t get_recv_timeout() override;

    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;
    error_t read(void* buf, size_t size, size_t& nread) override;

 public:
    // writer
    void set_send_timeout(utime_t tm) override;
    utime_t get_send_timeout() override;
    error_t write(void* buf, size_t size) override;

 private:
    PIReaderWriter io;

    bool inited;
    // ssl context
    BIO* rbio;
    BIO* wbio;
    SSL_CTX* ctx;
    SSL* ssl;
    SSLRole role;
    SSLConfig config;
    PISSLCertSearch                 cert_searcher;
};

class StSSLAcceptor;
/**
 * ssl server wrapped
 */
class StSSLServerSocket : public IServerSocket {
 public:
    friend class StSSLAcceptor;
 public:
    StSSLServerSocket(PIServerSocket ssock);

 public:
    void init_config(SSLConfig& config, PISSLCertSearch search = nullptr);

 public:
    error_t listen(std::string self_ip, int sport, bool reuse_sport, int back_log) override;
    PSocket accept() override;

 private:
    void push_socket(PSocket sock);

 private:
    PIServerSocket                  ssock;
    PCondition                      condition;
    std::list<PSocket>              ready_sockets;
    std::shared_ptr<StSSLAcceptor>  acceptor;
    SSLConfig                       config;
    PISSLCertSearch                 cert_searcher;
};

// do tls handshake with async no blocking
class StSSLAcceptor : public ICoHandler {
 public:
    StSSLAcceptor(StSSLServerSocket& ssl);

 public:
    error_t handler() override;

 private:
    StSSLServerSocket& ssl;
};

}  // namespace sps

#endif

#endif  // SPS_SYS_ST_IO_SSL_HPP
