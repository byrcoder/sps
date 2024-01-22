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

//
// Created by byrcoder on 2021/8/31.
//

#include <sps_sys_st_io_ssl.hpp>
#include <sps_log.hpp>

#ifdef OPENSSL_ENABLED

namespace sps {

error_t st_tcp_ssl_connect(const std::string& server, int port,
                           utime_t tm, PSocket& io) {
    // TODO: IMPL
    return ERROR_SSL_READ;
}

error_t st_tcp_ssl_accept(PSocket& io, utime_t timeout) {
    // TODO: IMPL
    return ERROR_SSL_READ;
}

std::map<SSL_CTX*, StSSLSocket*> StSSLSocket::cb_ctx;

int StSSLSocket::ssl_servername_callback(SSL *ssl, int *ag, void *arg) {
    SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);

    if (cb_ctx.count(ctx) == 0) {
        sp_error("no found ctx");
        return SSL_TLSEXT_ERR_NOACK;
    }
    StSSLSocket* skt = cb_ctx[ctx];

    const char* server_name = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
    error_t ret = SUCCESS;

    SSLConfig config;
    if (!server_name || (ret = skt->cert_searcher->search(server_name, config)) != SUCCESS) {
        sp_error("ssl sni search cert ret %d", ret);
        return SSL_TLSEXT_ERR_NOACK;
    }

    if ((ret = skt->do_ssl_verify(config)) != SUCCESS) {
        sp_error("ssl sni verify cert ret %d", ret);
        return SSL_TLSEXT_ERR_NOACK;
    }

    return SSL_TLSEXT_ERR_OK;
}

StSSLSocket::StSSLSocket(PIReaderWriter io, SSLRole mode, SSLConfig config, PISSLCertSearch cs) {
    this->io   = std::move(io);
    this->role = mode;
    this->config = config;
    this->cert_searcher = std::move(cs);

    inited     = false;
    // ssl init
#if (OPENSSL_VERSION_NUMBER < 0x10002000L) // v1.0.2
    ctx = SSL_CTX_new(TLSv1_2_method());
#else
    ctx = SSL_CTX_new(TLSv1_2_method());  // TODO FIXME
#endif
    ssl = SSL_new(ctx);
    rbio = BIO_new(BIO_s_mem());
    wbio = BIO_new(BIO_s_mem());
    SSL_set_bio(ssl, rbio, wbio);

    cb_ctx[ctx] = this;
}

StSSLSocket::~StSSLSocket() {
    cb_ctx.erase(ctx);

    SSL_free(ssl);
    SSL_CTX_free(ctx);

    // coredump
    // BIO_free(rbio);
    // BIO_free(wbio);
}

error_t StSSLSocket::init() {
    if (inited) {
        return SUCCESS;
    }

    inited = true;
    switch (role) {
        case SSL_CLIENT:
            return handshake();
        case SSL_SERVER:
            return accept();
        default:
            sp_error("unknown ssl role");
            return ERROR_SSL_READ;
    }

    return SUCCESS;
}

/**
 * TODO: support 7.4.4.  Certificate Request
 * also see dtls https://web.archive.org/web/20150814081716/http://sctp.fh-muenster.de/dtls-samples.html
 * @return
 */
error_t StSSLSocket::handshake() {
    error_t ret = SUCCESS;

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_cipher_list(ctx, "ALL");

    // SSL setup active, as server role.
    SSL_set_connect_state(ssl);
    SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);

    // set extension server name
    if (!config.server_host.empty()) {
        SSL_set_tlsext_host_name(ssl, config.server_host.c_str());
    }

    unsigned char protocol[] = {
            2, 'h', '1'
    };
    // set extension alpn
    // SSL_set_alpn_protos(ssl, protocol, sizeof(protocol));

    // Send ClientHello
    if ((ret = handshake_util_read()) != ERROR_SSL_HANDSHAKE_CONTINUE && ret != SUCCESS) {
        sp_error("fail Send ClientHello, ret %d", ret);
        return ret;
    }
    sp_info("Send ClientHello");

    // Receive ServerHello, Certificate, Server Key Exchange, Server Hello Done
    if ((ret = handshake_util_write()) != ERROR_SSL_HANDSHAKE_CONTINUE && ret != SUCCESS) {
        sp_error("fail Receive ServerHello, Certificate, Server Key Exchange, "
                 "Server Hello Done, ret %d", ret);
        return ret;
    }
    sp_info("Receive ServerHello, Certificate, Server Key Exchange, Server Hello Done");

    // Send Client Key Exchange, Change Cipher Spec, Encrypted Handshake Message
    if ((ret = handshake_util_read()) != ERROR_SSL_HANDSHAKE_CONTINUE  && ret != SUCCESS) {
        sp_error("fail Send Client Key Exchange, Change Cipher Spec, Encrypted "
                 "Handshake Message, ret %d", ret);
        return ret;
    }
    sp_info("Send Client Key Exchange, Change Cipher Spec, Encrypted Handshake Message");

    // Change Cipher Spec, Encrypted Handshake Message
    if ((ret = handshake_util_write()) != SUCCESS) {
        sp_error("fail Receive Change Cipher Spec, Encrypted Handshake Message, "
                 "ret %d", ret);
        return ret;
    }
    sp_info("Change Cipher Spec, Encrypted Handshake Message");

    uint8_t* data = nullptr;
    int     n = 0;
    if ((n = BIO_get_mem_data(wbio, &data)) > 0) {
        // OK, reset it for the next read.
        sp_info("remain for wrote n %d", n);
        if ((n = BIO_reset(rbio)) != 1) {
            return ERROR_SSL_HANDSHAKE;
        }
    }

    return ret;

}

error_t StSSLSocket::accept() {
    error_t ret = SUCCESS;
    int     n   = 0;

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_cipher_list(ctx, "ALL");


    // SSL setup active, as server role.
    SSL_set_accept_state(ssl);
    SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);

    // TODO: imp sni
    if (cert_searcher) {
        SSL_CTX_set_tlsext_servername_callback(ctx, StSSLSocket::ssl_servername_callback);
    } else {
        if ((ret = do_ssl_verify(config)) != SUCCESS) {
            return ret;
        }
    }

    // Receive ClientHello
    if ((ret = handshake_util_write()) != ERROR_SSL_HANDSHAKE_CONTINUE && ret != SUCCESS) {

        sp_error("fail Receive ClientHello ret %d", ret);
        return ret;
    }

    // Send ServerHello, Certificate, Server Key Exchange, Server Hello Done
    if ((ret = handshake_util_read()) != ERROR_SSL_HANDSHAKE_CONTINUE && ret != SUCCESS) {
        sp_error("fail Send ServerHello, Certificate, Server Key Exchange, "
                 "Server Hello Done, ret %d", ret);
        return ret;
    }

    // Receive Client Key Exchange, Change Cipher Spec, Encrypted Handshake Message
    if ((ret = handshake_util_write()) != ERROR_SSL_HANDSHAKE_CONTINUE && ret != SUCCESS) {
        sp_error("fail Receive Client Key Exchange, Change Cipher Spec, "
                 "Encrypted Handshake Message ret %d", ret);
        return ret;
    }

    // Change Cipher Spec, Encrypted Handshake Message
    if ((ret = handshake_util_read()) != ERROR_SSL_HANDSHAKE_CONTINUE && ret != SUCCESS) {
        sp_error("fail Send Change Cipher Spec, Encrypted Handshake Message ret %d", ret);
        return ret;
    }

    sp_info("https: Server done");

    return ret;
}

error_t StSSLSocket::handshake_util_write() {
    error_t ret = SUCCESS;
    uint8_t buf[1024];
    size_t  nn = 0;
    int     n = 0;
    int     ssl_err = 0;
    char    *data = nullptr;

    ret = do_handshake(false);

    if (ret == SUCCESS) {
        sp_warn("success handshake");
        return ret;
    } else if (ret != ERROR_SSL_HANDSHAKE_CONTINUE){
        sp_error("fail read handshake ret %d", ret);
        return ret;
    }

    while (true) {
        if ((ret = io->read(buf, sizeof(buf), nn)) != SUCCESS) {
            sp_error("fail io read ret %d", ret);
            return ret;
        }

        sp_info("read ssl handshake(%lu) %.*s",  nn, (int) nn, buf);

        if ((n = BIO_write(rbio, buf, nn)) <= 0) {
            sp_error("fail ssl bio write fail n %d", n);
            return ERROR_SSL_HANDSHAKE;
        }

        if ((ret = do_handshake(true)) == SUCCESS) {
            return ret;
        }

        if (ret != ERROR_SSL_HANDSHAKE_CONTINUE) {
            sp_error("fail handshake ret %d", ret);
            return ret;
        }

        if (BIO_get_mem_data(wbio, &data) > 0) {
            // OK, reset it for the next read.
            if ((n = BIO_reset(rbio)) != 1) {
                return ERROR_SSL_HANDSHAKE;
            }
            break;
        }
    }
    return ret;
}

error_t StSSLSocket::handshake_util_read() {
    error_t ret = SUCCESS;
    error_t tmp_ret = SUCCESS;
    size_t  nn = 0;
    int     n = 0;
    int     ssl_err = 0;
    char    *data = nullptr;

    ret = do_handshake(false);

    if (ret == SUCCESS) {
        // sp_warn("success handshake");
        // return ret;
    } else if (ret != ERROR_SSL_HANDSHAKE_CONTINUE){
        sp_error("fail writing handshake ret %d", ret);
        return ret;
    }

    if ((n = BIO_get_mem_data(wbio, &data)) <= 0) {
        // OK, reset it for the next read.
        sp_error("fail get mem for write n %d", n);
        return ERROR_SSL_HANDSHAKE;
    }

    sp_info("write ssl handshake(%d) %.*s", n, n, data);

    // for next read
    if ((ssl_err = BIO_reset(rbio)) != 1) {
        sp_error("fail reset rio mem for %d", ssl_err);
        return ERROR_SSL_HANDSHAKE;
    }

    if ((tmp_ret = io->write(data, n)) != SUCCESS) {
        sp_error("fail io read ret %d", tmp_ret);
        return tmp_ret;
    }

    // clear write value
    if ((ssl_err = BIO_reset(wbio)) != 1) {
        sp_error("fail reset wio mem for %d", ssl_err);
        return ERROR_SSL_HANDSHAKE;
    }

    return ret;
}

error_t StSSLSocket::do_handshake(bool reading) {
    int n = SSL_do_handshake(ssl);

    if (n == 1) {
        return SUCCESS;
    }

    int ssl_err = SSL_get_error(ssl, n);
    if (n < 0 && ((ssl_err == SSL_ERROR_WANT_READ) ||
            (ssl_err == SSL_ERROR_WANT_WRITE))) {
        return ERROR_SSL_HANDSHAKE_CONTINUE;
    }

    sp_error("do handshake n %d, ssl_err %d", n, ssl_err);

    return ERROR_SSL_HANDSHAKE;
}

error_t StSSLSocket::do_ssl_verify(SSLConfig& config) {
    int n;
    // Setup the key and cert file for server.
    if (!config.crt_file.empty() &&
        (n = SSL_use_certificate_file(ssl, config.crt_file.c_str(), SSL_FILETYPE_PEM)) != 1) {
        sp_error("fail use ssl crt file %s, ret %d", config.crt_file.c_str(), n);
        return ERROR_SSL_HANDSHAKE;
    }

    if (!config.key_file.empty() &&
        (n = SSL_use_RSAPrivateKey_file(ssl, config.key_file.c_str(), SSL_FILETYPE_PEM)) != 1) {
        sp_error("fail use ssl key file %s, ret %d", config.key_file.c_str(), n);
        return ERROR_SSL_HANDSHAKE;
    }

    if ((n = SSL_check_private_key(ssl)) != 1) {
        sp_error("fail ssl check private key ret %d", n);
        return ERROR_SSL_HANDSHAKE;
    }
    sp_info("ssl: use key %s and cert %s.", config.key_file.c_str(), config.crt_file.c_str());

    return SUCCESS;
}

void StSSLSocket::set_recv_timeout(utime_t tm) {
    io->set_recv_timeout(tm);
}

utime_t StSSLSocket::get_recv_timeout() {
    return io->get_recv_timeout();
}

error_t StSSLSocket::read_fully(void* buf, size_t size, ssize_t* nread) {
    size_t n = 0;
    error_t ret = SUCCESS;

    while (n < size) {
        size_t len = 0;
        if ((ret = read(buf, size-n, len)) != SUCCESS) {
            break;
        }

        n += len;
    }

    if (nread) {
        *nread = n;
    }

    return ret;
}

error_t StSSLSocket::read(void* buf, size_t size, size_t& nread) {
    error_t ret = SUCCESS;

    do {
        // read decode buffer
        int n  = SSL_read(ssl, buf, size);
        int r2 = BIO_ctrl_pending(rbio);
        int r3 = SSL_is_init_finished(ssl);

        if (n > 0) {
            nread = n;
            break;
        }
        sp_debug("SSL READ n %d", n);

        if (n <= 0 && SSL_ERROR_WANT_READ == SSL_get_error(ssl, n)) {
            // read raw buffer
            if ((ret = io->read(buf, size, nread)) != SUCCESS) {
                sp_error("fail ssl raw read ret %d", ret);
                return ret;
            }

            sp_debug("reading raw buf %lu", nread);

            n = BIO_write(rbio, buf, nread);
            if (n <= 0) {
                sp_error("bio write failed n %d, num %ld", n, nread);
                return ERROR_SSL_READ;
            }
        } else {
            sp_error("ssl read failed n %d, %d, r2 %d, r3 %d",
                      n, SSL_get_error(ssl, n), r2, r3);
            return ERROR_SSL_READ;
        }
    } while (true);

    return ret;
}

void StSSLSocket::set_send_timeout(utime_t tm) {
    io->set_send_timeout(tm);
}

utime_t StSSLSocket::get_send_timeout() {
    return io->get_send_timeout();
}

error_t StSSLSocket::write(void* buf, size_t size) {
    // TODO: IMPL SSL
    error_t ret     = SUCCESS;
    size_t  nwrite  = 0;

    if (size == 0) {
        sp_error("fail for write empty");
        return ERROR_SSL_WRITE;
    }

    do {
        // write for encode to wbio
        int n = SSL_write(ssl, (char*)buf + nwrite, size - nwrite);

        if (n <= 0) {
            sp_error("fail write n %d", n);
            return ERROR_SSL_WRITE;
        }

        nwrite           += n;
        char* encode_data = nullptr;
        // get encode data
        auto nsize        = BIO_get_mem_data(wbio, &encode_data);

        // write encode data
        if ((ret = io->write(encode_data, nsize)) != SUCCESS) {
            sp_error("fail ssl encode write ret %d", ret);
            return ret;
        }
        sp_debug("encode_data %lu, %.*s", nsize, (int) nsize, encode_data);

        n = BIO_reset(wbio);
        if (n < 0) {
            sp_error("fail bio write reset n %d,", n);
            return ERROR_SSL_WRITE;
        }
    } while (nwrite < size);

    return ret;
}

StSSLServerSocket::StSSLServerSocket(PIServerSocket ssock) {
    this->ssock = ssock;
    condition   = IConditionFactory::get_instance().create_condition();
    acceptor    = std::make_unique<StSSLAcceptor>(*this);
}

void StSSLServerSocket::init_config(SSLConfig& config, PISSLCertSearch search) {
    this->config         = config;
    this->cert_searcher  = std::move(search);
}

error_t StSSLServerSocket::listen(std::string self_ip, int sport, bool reuse_sport, int back_log) {
    auto ret = ssock->listen(self_ip, sport, reuse_sport, back_log);
    if (ret != SUCCESS) {
        return ret;
    }

    return ICoFactory::get_instance().start(acceptor);
}

PSocket StSSLServerSocket::accept() {
    // TODO: IMPL SSL
    while (ready_sockets.empty()) {
        condition->wait(SLEEP_FOREVER);
    }

    auto socket = ready_sockets.front();
    ready_sockets.erase(ready_sockets.begin());
    return socket;
}

void StSSLServerSocket::push_socket(PSocket sock) {
    ready_sockets.push_back(std::move(sock));
    condition->signal();
}

StSSLAcceptor::StSSLAcceptor(StSSLServerSocket &ssl) : ssl(ssl) {
}

error_t StSSLAcceptor::handler() {
    error_t ret = SUCCESS;

    do {
        auto sock = ssl.ssock->accept();
        auto ssl_sock = std::make_shared<StSSLSocket>(sock, SSL_SERVER, ssl.config, ssl.cert_searcher);

        // TODO: fixme with threads startup
        if ((ret = ssl_sock->init()) != SUCCESS) {
            sp_error("ssl accept ret %d", ret);
            continue;
        }

        ssl.push_socket(std::make_shared<Socket>(ssl_sock, sock->get_peer_ip(),
                sock->get_peer_port()));

    } while (true);

    return SUCCESS;
}

}

#endif
