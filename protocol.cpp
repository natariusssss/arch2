#include "protocol.h"
int pack_msg(const Message& msg, char* buf, int buf_size) {
    if (!buf || buf_size<1) return -1;
    buf[0]=(char)msg.type;
    return 1;
}
int unpack_msg(const char* buf, int len, Message& out) {
    if (len<1 || !buf) return -1;

    char type=buf[0];
    if (type!=MSG_RING && type!=MSG_PONG) return -1;
    out.type=static_cast<MsgType>(type);
    return 0;
}