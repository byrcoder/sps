#ifndef SPS_PARSER_HPP
#define SPS_PARSER_HPP

#include <http_parser.h>

#include <memory>

#include "net/io.hpp"

namespace sps {

class HttpParser {
 public:
    HttpParser(PIReaderWriter io);

 private:
    PIReaderWriter io;
};

}

#endif //SPS_PARSER_HPP
