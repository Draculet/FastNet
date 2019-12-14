#ifndef __WEB_SOCKET_CODEC_H__
#define __WEB_SOCKET_CODEC_H__

#include "sha1.h"
#include "base64.h"
#include <string>
#include <vector>
#include <arpa/inet.h>
#include "headerParse/requestParser.h"

using namespace std;
using namespace http;
using namespace net;

uint64_t hton64(uint64_t &input)
{
    char arr[8];
    memcpy(arr, &input, 8);
    char *p = arr;
    for (int i = 0; i < 4; i++)
    {
        char tmp = *(p + i);
        *(p + i) = *(p + 7 - i);
        *(p + 7 - i) = tmp;
    }
    uint64_t res;
    memcpy(&res, arr, 8);
    return res;
}


uint64_t ntoh64t(uint64_t &input)
{
    char arr[8];
    memcpy(arr, &input, 8);
    char *p = arr;
    for (int i = 0; i < 4; i++)
    {
        char tmp = *(p + i);
        *(p + i) = *(p + 7 - i);
        *(p + 7 - i) = tmp;
    }
    uint64_t res;
    memcpy(&res, arr, 8);
    return res;
}


struct frame
{
    public:
    frame()
        :fin_(-1),
        opcode_(-1),
        mask_(-1),
        payloadLen_(0),
        messege_()
    {
        memset(maskbuf_, 0, 4);
    }

    frame(char fin, char opcode, char mask, uint64_t payloadLen, char *maskbuf, string messege)
        :fin_(fin),
        opcode_(opcode),
        mask_(mask),
        payloadLen_(payloadLen),
        messege_(messege)
    {
        memcpy(maskbuf_, maskbuf, 4);
    }

    string &getMessege()
    {
        return messege_;
    }

    char getFin()
    {
        return fin_;
    }

    char getOpcode()
    {
        return opcode_;
    }

    char getMask()
    {
        return mask_;
    }

    uint64_t getPayloadLen()
    {
        return payloadLen_;
    }
    
    //包装为一个完整帧，可直接发送
    static string toFrame(string &messege, char opcode)
    {
        char buf[128] = {0};
        int buflen = 0;
        buf[0] = 0b10000000;
        buf[0] |= opcode;
        buflen++;
        size_t len = messege.size();
        /*
            len == 126时 已开始拓展出2个字节作为长度标识 buf[0] == 126
            len == 65535时 仍是2个字节 buf[0] == 126
            len == 65536时 拓展为8个字节 buf[1] == 127
         */
        if (len < 126)
        {
            buf[1] = len;
            buflen += 1;
        }
        else if (len >= 126 && len <= 65535)
        {
            buf[1] = 126;
            uint16_t l = len;
            l = htons(l);
            memcpy(buf + 2, &l, sizeof(uint16_t));
            buflen += 3;
        }
        else if (len > 65535)
        {
            buf[1] = 127;
            uint64_t l = len;
            l = hton64(l);
            memcpy(buf + 2, &l, sizeof(uint64_t));
            buflen += 9;
        }

        return string(buf, buflen) + messege;
    }

    //使用引用在做掩码时会修改原来内容,使用者易错用,需要将传入的引用复制
    static string toFrameWithMask(string &messege, char opcode, string maskstr = string() )
    {
        char buf[128] = {0};
        char maskbuf[4];
        int buflen = 0;
        buf[0] = 0b10000000;
        buf[0] |= opcode;
        buflen++;
        size_t len = messege.size();
        /*
            len == 126时 已开始拓展出2个字节作为长度标识 buf[0] == 126
            len == 65535时 仍是2个字节 buf[0] == 126
            len == 65536时 拓展为8个字节 buf[1] == 127
         */
        if (len < 126)
        {
            buf[1] = len;
            buflen += 1;
        }
        else if (len >= 126 && len <= 65535)
        {
            buf[1] = 126;
            uint16_t l = len;
            l = htons(l);
            memcpy(buf + 2, &l, sizeof(uint16_t));
            buflen += 3;
        }
        else if (len > 65535)
        {
            buf[1] = 127;
            uint64_t l = len;
            l = hton64(l);
            memcpy(buf + 2, &l, sizeof(uint64_t));
            buflen += 9;
        }
        buf[1] |= 0x80;//mask位设置
        if (maskstr.size() == 4)
        {
            buf[buflen] = maskstr[0];
            maskbuf[0] = maskstr[0];
            buflen += 1;
            buf[buflen] = maskstr[1];
            maskbuf[1] = maskstr[1];
            buflen += 1;
            buf[buflen] = maskstr[2];
            maskbuf[2] = maskstr[2];
            buflen += 1;
            buf[buflen] = maskstr[3];
            maskbuf[3] = maskstr[3];
            buflen += 1;
        }
        else //如是0等同于不加掩码
        {
            buf[buflen] = 0;
            maskbuf[0] = 0;
            buflen += 1;
            buf[buflen] = 0;
            maskbuf[1] = 0;
            buflen += 1;
            buf[buflen] = 0;
            maskbuf[2] = 0;
            buflen += 1;
            buf[buflen] = 0;
            maskbuf[3] = 0;
            buflen += 1;
        }
        string mess = string(len, 0);
        for (int i = 0; i < len; i++)
        {
            int k = i % 4;
            mess[i] = messege[i] ^ maskbuf[k];
            //cout << maskbuf[k] << endl;
        }
        
        return string(buf, buflen) + mess;
    }

    void debugPrint()
    {
        string top = string(messege_, 0, 100);
        printf("*debug* Frame Info:\nFin: %d\nOpcode: %d\nMask: %d\nPayloadLen: %d\nMessage:\n%s\n\n", fin_, opcode_, mask_, payloadLen_, top.c_str());
    }

    private:
        char fin_;
        char opcode_;
        char mask_;
        uint64_t payloadLen_;
        char maskbuf_[4];
        string messege_;
};


class webSocketCodec
{
    public:
        webSocketCodec(Buffer *buf)
            :frames(),
            buf_(buf),
            kfin(true),
            klenfin(true),
            kmaskfin(true),
            kpayloadfin(true),
            curFin(-1),
            curMask(-1),
            curOpcode(-1),
            curPayloadLen(0)
        {
            memset(curMaskBuf, 0, 4);
            //webSocketCodec::key = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        }
        ~webSocketCodec()
        {
            printf("*debug* ~webSocketCodec\n");
        } 

        static bool isWebSocket(requestHeader *header)
        {
            auto iter = header->getKV()->find("Sec-WebSocket-Key");
            return iter != header->getKV()->end();
        }

        //错误返回空串
        static string toHttpResponse(requestHeader *header)
        {
            string reqkey_;
            const string key = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
            string response_ = "HTTP/1.1 101 Switching Protocols\r\n";
            response_ += "Connection:Upgrade\r\n";
            response_ += "Server:websocket server\r\n";
            response_ += "Upgrade:WebSocket\r\n";
            response_ += "Access-Control-Allow-Credentials:true\r\n";
            response_ += "Access-Control-Allow-Headers:content-type\r\n";
            response_ += "Sec-WebSocket-Accept:";
            auto iter = header->getKV()->find("Sec-WebSocket-Key");
            if ( iter != header->getKV()->end() )
            {
                reqkey_ = iter->second;
            }
            else
            {
                return string();
            }
            reqkey_ += key;
            SHA1 sha;
            unsigned int message_digest[5];
            sha.Reset();
            sha << reqkey_.c_str();

            sha.Result(message_digest);
            for (int i = 0; i < 5; i++)
            {
                message_digest[i] = htonl(message_digest[i]);
            }
            reqkey_ = base64_encode(reinterpret_cast<const unsigned char*>(message_digest),20);
            reqkey_ += "\r\n\r\n";
            response_ += reqkey_;

            return response_;
        }
        
        int parse()
        {
            while (parseFrame())
            {

            }

            return frames.size();
        }

        bool parseFrame()
        {
            //处理请求body不完整的情况
            if (kfin == false)
            {
                if (klenfin == false)
                {
                    if (curPayloadLen == 127)
                    {
                        if (buf_->readable() < 8)
                        {
                            kfin = false;//解析不完整
                            klenfin = false;//标识获得长度缺失
                            kmaskfin = false;
                            kpayloadfin = false;
                            printf("*debug* klenfin==false frame仍解析不完整\n");

                            return false; //Exit
                        }
                        uint64_t length = 0;
                        memcpy(&length, buf_->current(), sizeof(uint64_t));
                        buf_->retrieve(8);
                        curPayloadLen = ntoh64t(length);
                    }
                    else
                    {
                        printf("*debug* curPayloadLen != 127 Error\n");
                        return false;
                    }
                    klenfin = true;
                }
                if (kmaskfin == false)
                {
                    if (curMask == 1)
                    {
                        if (buf_->readable() < 4)
                        {
                            kfin = false;
                            kmaskfin = false;
                            kpayloadfin = false;
                            printf("*debug* kmaskfin==false frame仍解析不完整\n");

                            return false; //Exit
                        }

                        for(int i = 0; i < 4; i++)
                        {
                            curMaskBuf[i] = *(buf_->current());
                            buf_->retrieve(1);
                        }
                    }
                    kmaskfin = true;
                }
                if (kpayloadfin == false)
                {
                    if (buf_->readable() >= curPayloadLen)
                    {
                        printf("*debug* 开始解析不完整Payload\n");
                        char payload[curPayloadLen + 8] = {0};
                        memset(payload, 0, sizeof(payload));
                        if(curMask != 1)
                        {
                            memcpy(payload, buf_->current(), curPayloadLen);
                        }
                        else
                        {
                            for(size_t i = 0; i < curPayloadLen; i++)
                            {
                                int j = i % 4;
                                payload[i] = *(buf_->current() + i) ^ curMaskBuf[j];
                            }
                        }
                        buf_->retrieve(curPayloadLen);
                        frames.push_back( frame(curFin, curOpcode, curMask, curPayloadLen, curMaskBuf, string(payload, curPayloadLen) ) );
                        klenfin = true;
                        kmaskfin = true;
                        kpayloadfin = true;
                        kfin = true;
                        curFin = -1;
                        curMask = -1;
                        memset(curMaskBuf, 0, 4);
                        curOpcode = -1;
                        curPayloadLen = 0;
                        printf("*debug* 解析完成\n");
                    }
                    else
                    {
                        kfin = false;
                        kpayloadfin = false;
                        printf("*debug* kpayloadfin==false frame仍然不完整, 该frame长度%d\n", curPayloadLen);

                        return false; //Exit
                    }
                }

                return true;
            }

            if (buf_->readable() >= 6)//最短frame 6 bytes,浏览器发送的包默认带掩码
            {
                printf("*debug* 解析开始\n");
                curFin = (unsigned char)*(buf_->current()) >> 7;
                curOpcode = *(buf_->current()) & 0x0f;
                buf_->retrieve(1);
                curMask = (unsigned char)*(buf_->current()) >> 7;
                curPayloadLen = *(buf_->current()) & 0x7f;
                buf_->retrieve(1);
                if(curPayloadLen == 126)
                {
                    uint16_t length = 0;
                    memcpy(&length, buf_->current(), 2);
                    buf_->retrieve(2);
                    curPayloadLen = ntohs(length);
                    klenfin = true;
                    //6字节能保证解析到此能完整
                }
                else if (curPayloadLen == 127)
                {
                    if (buf_->readable() < 8)
                    {
                        kfin = false;//解析不完整
                        klenfin = false;//标识获得长度缺失
                        kmaskfin = false;
                        kpayloadfin = false;
                        printf("*debug* klenfin==false frame解析不完整\n");

                        return false; //Exit
                    }
                    uint64_t length = 0;
                    memcpy(&length, buf_->current(), sizeof(uint64_t));
                    buf_->retrieve(8);
                    curPayloadLen = ntoh64t(length);
                    klenfin = true;
                }
                if (curMask == 1)
                {
                    if (buf_->readable() < 4)
                    {
                        kfin = false;
                        kmaskfin = false;
                        kpayloadfin = false;
                        printf("*debug* kmaskfin==false frame解析不完整\n");

                        return false; //Exit
                    }

                    for(int i = 0; i < 4; i++)
                    {
                        curMaskBuf[i] = *(buf_->current());
                        buf_->retrieve(1);
                    }
                }
                kmaskfin = true;
                if (buf_->readable() >= curPayloadLen)
                {
                    printf("*debug* 开始解析Payload\n");
                    char payload[curPayloadLen + 8] = {0};
                    memset(payload, 0, sizeof(payload));
                    if(curMask != 1)
                    {
                        memcpy(payload, buf_->current(), curPayloadLen);
                    }
                    else
                    {
                        for(size_t i = 0; i < curPayloadLen; i++)
                        {
                            int j = i % 4;
                            payload[i] = *(buf_->current() + i) ^ curMaskBuf[j];
                        }
                    }
                    buf_->retrieve(curPayloadLen);
                    frames.push_back( frame(curFin, curOpcode, curMask, curPayloadLen, curMaskBuf, string(payload, curPayloadLen) ) );
                    klenfin = true;
                    kmaskfin = true;
                    kpayloadfin = true;
                    kfin = true;
                    curFin = -1;
                    curMask = -1;
                    memset(curMaskBuf, 0, 4);
                    curOpcode = -1;
                    curPayloadLen = 0;
                }
                else
                {
                    kfin = false;
                    kpayloadfin = false;
                    printf("*debug* kpayloadfin==false frame解析不完整, 该frame长度%d\n", curPayloadLen);

                    return false; //Exit
                }
                printf("*debug* 解析完成\n");
                return true; //Exit
            }
            else
            {
                printf("*debug* 长度过短,未开始解析, 目前缓冲区中长度%d\n", buf_->readable());
                //不做标记
                return false;
            }
        }

        //TODO 考虑其他缓存的实现(StringPiece)
        vector<frame> frames;

    private:
        //static string key;
        Buffer *buf_;
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