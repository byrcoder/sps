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
// Created by byrcoder on 2021/6/22.
//

#include <sps_rtmp_handshake_handler.hpp>
#include <sps_rtmp_server.hpp>

namespace sps {

RtmpHandshakeHandler::RtmpHandshakeHandler() : IPhaseHandler("rtmp-handshake-handler") {

}

error_t RtmpHandshakeHandler::handler(HostPhaseCtx &ctx) {
    RtmpConnectionHandler* rt = dynamic_cast<RtmpConnectionHandler*>(ctx.conn);
    if (RTMP_Serve(rt->hk->get_rtmp()) == FALSE) {
        sp_error("rtmp shandshake failed "
                 "rt->hook: %p, "
                 "socket_buf->rtmp: %p", rt->hk->get_rtmp()->hook,
                rt->hk->get_rtmp()->m_sb.rtmp);
        return ERROR_RTMP_HANDSHAKE;
    }

    RTMPPacket packet = { 0 };
    while (RTMP_ReadPacket(rt->hk->get_rtmp(), &packet) == TRUE)
    {
        if (!RTMPPacket_IsReady(&packet))
            continue;
        sp_info("rtmp packet head_type: %u, m_packetType: %u, "
                "m_hasAbsTimestamp: %u, m_nChannel: %d, "
                "m_nTimeStamp: %u, m_nInfoField2: %d, "
                "m_nBodySize: %u, m_nBytesRead: %u",
                packet.m_headerType, packet.m_packetType,
                packet.m_hasAbsTimestamp, packet.m_nChannel,
                packet.m_nTimeStamp, packet.m_nInfoField2,
                packet.m_nBodySize, packet.m_nBytesRead);
        RTMPPacket_Free(&packet);
    }
    sp_error("rtmp final ret: %d", rt->hk->get_error());

    return rt->hk->get_error();
}

}