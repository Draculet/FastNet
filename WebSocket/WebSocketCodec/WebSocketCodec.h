#ifndef __WEB_SOCKET_CODEC_H__
#define __WEB_SOCKET_CODEC_H__

#include "sha1.h"
#include "base64.h"
#include <string>
#include <vector>
#include <arpa/inet.h>
#include "headerParse/requestParser.h"

uint64_t hton64(uint64_t &input);
uint64_t ntoh64t(uint64_t &input);


struct frame
{
    public:
    frame();
    frame(char fin, char opcode, char mask, uint64_t payloadLen, char *maskbuf, std::string messege);
    std::string &getMessege();
    char getFin();
    char getOpcode();
    char getMask();
    uint64_t getPayloadLen();
    //包装为一个完整帧，可直接发送
    static std::string toFrame(std::string &messege, char opcode);
    //使用引用在做掩码时会修改原来内容,使用者易错用,需要将传入的引用复制
    static std::string toFrameWithMask(std::string &messege, char opcode, std::string maskstr = std::string() );
    void debugPrint();

    private:
        char fin_;
        char opcode_;
        char mask_;
        uint64_t payloadLen_;
        char maskbuf_[4];
        std::string messege_;
};


class WebSocketCodec
{
    public:
    WebSocketCodec(net::Buffer *buf);
    static bool isWebSocket(http::requestHeader *header);
    //错误返回空串
    static std::string toHttpResponse(http::requestHeader *header);
    int parse();
    bool parseFrame();

    //TODO 考虑其他缓存的实现(StringPiece)
    std::vector<frame> frames;
    private:
    //static string key;
    net::Buffer *buf_;
    bool kfin;

    //以下三项用于标识不完整位置
    bool klenfin;
    bool kmaskfin;
    bool kpayloadfin;

    char curFin;
    char curMask;
    char curOpcode;
    uint64_t curPayloadLen;
    char curMaskBuf[4];
};

#endif