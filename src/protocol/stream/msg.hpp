#ifndef SPS_MSG_HPP
#define SPS_MSG_HPP

namespace sps {

enum ProtocolType {
    FLV = 0,
};

class IProtocolMessage {
 public:
    char* message()  {  return nullptr; }
    int   length()          {  return 0;       }
};

}


#endif  // SPS_MSG_HPP
