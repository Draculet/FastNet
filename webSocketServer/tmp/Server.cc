#include "../Server.h"
#include "../Connection.h"
#include "../Buffer.h"
#include "headerParse/requestParser.h"
#include "base64.h"
#include "sha1.h"
#include "webSocketCodec.h"

using namespace http;

void onRead(map<shared_ptr<Connection>, webSocketCodec *> *codecMap, Buffer *buf, shared_ptr<Connection> conn);

int main(void)
{
    Server s(4, 8832);
    map<shared_ptr<Connection>, webSocketCodec *> codecMap;
    s.setReadCallback(bind(onRead, &codecMap,  placeholders::_1, placeholders::_2 ));
    s.start();
}

//FIXME 函数只考虑x86平台
uint64_t ntoh64(uint64_t &input)
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

void onRead(map<shared_ptr<Connection>, webSocketCodec *> *codecMap, Buffer *buf, shared_ptr<Connection> conn)
{
    webSocketCodec *codec;
    if (codecMap->find(conn) == codecMap->end())
    {
        (*codecMap)[conn] = new webSocketCodec(buf);
        codec = (*codecMap)[conn];
    }
    else
    {
        codec = (*codecMap)[conn];
    }
    if (buf->preViewAsString(3) == "GET")
    {
        requestParser reqhead(buf);//用法有误
        
        reqhead.parse();
        for (int i = 0; i < reqhead.size(); i++)
        {
            if ( webSocketCodec::isWebSocket( &reqhead.reqs[i]) )
                conn->send( webSocketCodec::toHttpResponse( &reqhead.reqs[i]) );
        }
    }
    else
    {
        int num = codec->parse();
        string mark = "abcd";
        for (int i = 0; i < num; i++)
        {
            codec->frames[i].debugPrint();
            conn->send(frame::toFrameWithMask(codec->frames[i].getMessege(), 1, mark));
        }
        codec->frames.clear();
        //传入引用出现问题,toFrame函数修改了引用,已解决
        //frame::toFrameWithMask(, 1);
            //cout << "data:\n" << frame::toFrameWithMask(str, 1, mark) << endl;
        //conn->send(frame::toFrameWithMask(str, 1, mark));
    }
}
        /*  
            masking_key[0] = mark[0];
            masking_key[1] = mark[1];
            masking_key[2] = mark[2];
            masking_key[3] = mark[3];
            int len = str.size();
            //uint8_t len8 = len;
            char buf[1024] = {0};
            buf[0] = 0b10000001;
            //超过126需要处理
            buf[1] = len;
            buf[1] |= 0x80;
            buf[2] = masking_key[0];
            buf[3] = masking_key[1];
            buf[4] = masking_key[2];
            buf[5] = masking_key[3];
            int j = 0;
            int k = 0;
            for (int i = 6; i < len + 6; i++)
            {
                k = j % 4;
                buf[i] = str[j] ^ masking_key[k];
                j++;
            }
            for (int i = 0; i < len + 6; i++)
            {
                //printf("%d ", buf[i]);
            }
            cout << endl;
            
            string data = string(buf, len + 6);
            cout << "data:\n" << data << endl;
            for (int i = 0; i < data.size(); i++)
            {
                printf("%d ", data[i]);
            }
            printf("\n");
            conn->send(data);
            //TODO flag = 1;
        */

            /* 
            if (payload_[0] == '\r' && payload_[1] == '\n' && payload_[2] == '\r' && payload_[3] == '\n')
            {
                memset(fileToWrite, 0 ,100);
                memcpy(fileToWrite, "./TMP/", 6);
                memcpy(fileToWrite + 6, payload_ + 4, strlen(payload_) - 4);
                printf("Dir To Write: %s\n", fileToWrite);
            }
            else
            {
                FILE *fp = fopen(fileToWrite, "wb");
                fwrite(payload_, payload_length_, 1, fp);
                fclose(fp);
            }
            */

//}
    /*
    if (reqhead_.size() > 0)
    {
        //http1.1复用连接但仍是串行处理,所以不会有buffer里请求数大于1的情况
        //printf("In Conn Fd %d : request size %d\n", conn->chann_.getFd(), conn->reqhead_.size());
        for (int i = 0; i < reqhead_.size(); i++)
        {
            resp r = reqhead_.reqs[i].toResp("webapp");
            string data = string(r.resptr_, r.len_);
            conn->send(data);
            //printf("send %d bytes\n", r.len_);
        }
    }
    */