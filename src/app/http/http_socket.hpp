#ifndef SPS_PROTOCOL_HTTP_SOCKET_HPP
#define SPS_PROTOCOL_HTTP_SOCKET_HPP

#include <app/url/url.hpp>
#include <net/net_socket.hpp>

namespace sps {

class HttpResponseSocket : public Socket {
 public:
    HttpResponseSocket(PIReaderWriter rw, const std::string& ip, int port);

 public:
    error_t init(int s_code, std::list<RequestHeader>* hd, int content_len, bool chunked);
    error_t write_header();
    error_t write(void* buf, size_t size) override;

 private:
    int  status_code    = 200;
    bool sent_header    = false;
    int  content_length = 0;
    bool chunked        = false;
    std::list<RequestHeader> headers;
};
typedef std::shared_ptr<HttpResponseSocket> PHttpResponseSocket;

}

#endif  // SPS_PROTOCOL_HTTP_SOCKET_HPP
