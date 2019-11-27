#include "../Server.h"
#include "../Connection.h"
#include "../Buffer.h"
#include "headerParse/requestParser.h"

using namespace http;

void onRead(Buffer *buf, shared_ptr<Connection> conn);

int main(void)
{
    Server s(4, 8832);
    s.setReadCallback(bind(onRead, placeholders::_1, placeholders::_2 ));
    s.start();
}

void onRead(Buffer *buf, shared_ptr<Connection> conn)
{
    requestParser reqhead_(buf);
    while (reqhead_.parseBuffer())
    {
        
    }
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
}