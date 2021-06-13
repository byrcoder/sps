#ifndef SPS_NET_FILE_HPP
#define SPS_NET_FILE_HPP

#include <string>

#include <net/sps_net_io.hpp>
#include <fstream>

namespace sps {

class FileReader : public IReader {
 public:
    FileReader(const std::string& filename);
    ~FileReader();

 public:
    error_t initialize();
    void    close();

 public:
    void    set_recv_timeout(utime_t tm) override {  }
    utime_t get_recv_timeout()  override { return 0; }
    bool    seekable()  override { return true; }

    /**
    * 读满数据
    */
    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;

    /**
     * 尽量读数据
     */
    error_t read(void* buf, size_t size, size_t& nread) override;

    error_t read_line(std::string& line) override;

    int     cur_line_num() override;

 private:
    std::string filename;
    bool inited;
    std::ifstream fh;
    int line_num;

};

}

#endif //SPS_NET_FILE_HPP
