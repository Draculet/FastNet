#include "../Server.h"
#include "../Connection.h"
#include "../Buffer.h"

void OnRead(Buffer *buf, shared_ptr<Connection> conn);


int main()
{
    Server s(4, 8832);
    s.setReadCallback(bind(OnRead, placeholders::_1, placeholders::_2 ));
    s.start();
}

void OnRead(Buffer *buf, shared_ptr<Connection> conn)
{
    string s = buf->retrieveAllAsString();
    conn->send(s);
}