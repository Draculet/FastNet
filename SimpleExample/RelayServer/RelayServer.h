#include "../../FastNet/include/Server.h"
#include "../../FastNet/include/Connection.h"
#include "../../FastNet/include/Buffer.h"

int connectPeer(std::string ip, int port, net::NetAddr *addr);

class RelayServer
{
    public:
    RelayServer(int port);
    void onRead(net::Buffer *buf, std::shared_ptr<net::Connection> conn);
    void onDisConnection(std::shared_ptr<net::Connection> conn);
    //client loop runInloop
    void disableConn(std::shared_ptr<net::Connection> conn);
    void onConnection(std::shared_ptr<net::Connection> conn);
    void clientClose(std::shared_ptr<net::Connection> conn);//由其他线程调用
    void start();

    private:
    std::unique_ptr<net::Server> serv_;
    std::map<std::shared_ptr<net::Connection>, std::shared_ptr<net::Connection> > connfdMap_;
    base::Mutex mutex_;
};
