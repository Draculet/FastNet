#include "Connection.h"
#include "Acceptor.h"
#include "Eventloop.h"
#include "ThreadLoop.h"
#include "ThreadLoopPool.h"
#include "Buffer.h"
#include "NetAddr.h"
#include <map>

namespace net
{
    class Server : base::noncopyable
    {
        public:
        //使用主线程运行Server, Eventloop属于主线程
        Server(int threadnums, int port);
        ~Server();
        int getAcceptFd();
        void start();
        void handleNewConn(int sockfd,  NetAddr &addr);
        void doRemoveConn(std::shared_ptr< Connection> conn);
        void removeConn(std::shared_ptr< Connection> conn);
         Eventloop *getLoop();
        void setConnCallback(std::function<void (std::shared_ptr< Connection>)> connCallback);
        void setDisConnCallback(std::function<void (std::shared_ptr< Connection>)> disConnCallback);
        void setReadCallback(std::function<void ( Buffer *, std::shared_ptr< Connection>) > readCallback);
        void setWriteFinishCallBack(std::function<void ()> writeFinishCallBack);
        std::map<std::string, std::shared_ptr< Connection> > *getConnMap();
        void testPrint();
         Eventloop *getNextLoop();

        private:
        std::unique_ptr< Eventloop> loop_;
        std::unique_ptr< ThreadLoopPool> pool_;
        std::unique_ptr< Acceptor> acceptor_;
        //vector<shared_ptr<Connection> > connlist;//TODO 改成map<>
        std::map<std::string, std::shared_ptr< Connection> > connMap_;
        std::function<void (std::shared_ptr< Connection>) > connCallback_;
        std::function<void (std::shared_ptr< Connection>) > disConnCallback_;
        std::function<void ( Buffer *, std::shared_ptr< Connection>)> readCallback_;
        std::function<void ()> writeFinishCallBack_;
    };
}