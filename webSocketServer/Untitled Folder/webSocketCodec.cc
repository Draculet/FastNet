#include "sha1.h"
#include "base64.h"
#include <string>
#include <vector>
#include "headerParse/requestParser.h"

using namespace std;
using namespace net;

class webSocketCodec
{
    public:
        webSocketCodec()
            :response_(),
            reqkey_(),

        {
            response_ += "HTTP/1.1 101 Switching Protocols\r\n";
            response_ += "Connection:Upgrade\r\n";
            response_ += "Server:websocket server\r\n";
            response_ += "Upgrade:WebSocket\r\n";
            response_ += "Access-Control-Allow-Credentials:true\r\n";
            response_ += "Access-Control-Allow-Headers:content-type\r\n";
            response_ += "Sec-WebSocket-Accept:";
        }
            
        bool isWebSocket(requestHeader *header)
        {
            auto iter = header->getKV()->find("Sec-WebSocket-Key");
            return iter != header->getKV()->end()
        }

        //错误返回空串
        string toResponse(requestHeader *header)
        {
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
        
        size_t parseFrame()
        {
            
        }

        //TODO 考虑其他缓存的实现(StringPiece)
        vector<string> messeges;
    private:
        static const string key = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        string response_;
        string reqkey_;
        Buffer *buf_;
}