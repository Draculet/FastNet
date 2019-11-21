#include "../Server.h"
#include "../Connection.h"
#include "../Buffer.h"

void OnRead(Buffer *buf, shared_ptr<Connection> conn);
void handleRead(Server *serv);

int main()
{
    Server s(4, 8832);
    Eventloop *loop = s.getLoop();
    Channel ch(STDIN_FILENO, loop);
    ch.setReadCallback(bind(handleRead, &s));
    ch.enableRead();
    s.setReadCallback(bind(OnRead, placeholders::_1, placeholders::_2 ));
    s.start();
}

void OnRead(Buffer *buf, shared_ptr<Connection> conn)
{
    string s = buf->retrieveAllAsString();
    cout << "From Client: " << s << endl;
    cerr << s << endl;
    //char b[1024 * 1024] = {0};
    //read(STDIN_FILENO, b, 1024 * 1024);
    //conn->send(b, strlen(b));
}

void handleRead(Server *serv)
{
    char b[1024 * 1024] = {0};
    read(STDIN_FILENO, b, 1024 * 1024);

    cout << "handleRead()" << endl;
    auto map = serv->getConnMap();
    auto iter = map->begin();
    if (iter != map->end())
    {
        iter->second->send(b, strlen(b));
        cout << "send " << strlen(b) << " bytes\n" << endl;
    }
}