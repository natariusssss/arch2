#ifndef PROTOCOL_H
#define PROTOCOL_H
enum MsgType {
    MSG_RING=1,
    MSG_PONG=2
};
struct Message {
    MsgType type;
};
int pack_msg(const Message& msg, char* buf, int buf_size);
int unpack_msg(const char* buf, int len, Message& out);
#endif