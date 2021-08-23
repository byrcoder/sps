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
// Created by byrcoder on 2021/8/11.
//

#include <memory>

#include <sps_st_io_srt.hpp>
#include <sps_log.hpp>

#include <sps_http_parser.hpp>
#include <sps_url_http.hpp>
#include <sps_url_file.hpp>

namespace sps {

class SrtHttpClient {
 public:
    SrtHttpClient() {

    }

 public:
    int get(std::string& ip, int port, std::string& path_query, std::string out) {

        auto url = "http://" + ip + ":" + std::to_string(port) + path_query;
        PRequestUrl req;
        RequestUrl::from(url, req);
        req->tm = 10 * 1000 * 1000;

        auto http = std::make_shared<HttpUrlProtocol>();

        if (http->open(req, SRT) != SUCCESS) {
            sp_error("open %s failed", url.c_str());
            return -1;
        }

        http->set_recv_timeout(10 * 1000 * 1000);
        http->set_send_timeout(10 * 1000 * 1000);

        uint8_t buf[1024];
        int size = sizeof(buf);

        std::shared_ptr<FileURLProtocol> fp;
        if (!out.empty()) {
            fp = std::make_shared<FileURLProtocol>(true, false);

            if (fp->open(out) != SUCCESS) {
                sp_error("open file %s", out.c_str());
                fp.reset();
            }
        }

        do {
            size_t nread = 0;
            if (http->read(buf, size-1, nread) != SUCCESS) {
                sp_error("read full failed");
                return -2;
            }
            buf[nread] = '\0';

            if (fp) {
                fp->write(buf, nread);
            } else {
                // sp_info("read msg %s", (char*) buf);
            }
        } while(true);
    }

 private:
    std::shared_ptr<StSrtSocket> skt;
};

}

int help() {
    sp_info("./srt_http_client ip port /path/url?query");
    return -1;
}

using namespace sps;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        return help();
    }

    st_init();

    std::string ip = argv[1];
    int port = atoi(argv[2]);
    std::string path = argv[3];
    std::string out;

    if (argc > 4) {
        out = argv[4];
    }

    SrtHttpClient client;

    client.get(ip, port, path, out);

    return 0;
}